from firebase_functions import firestore_fn, https_fn
from firebase_admin import initialize_app, firestore
from google.cloud.firestore_v1.base_query import FieldFilter
import pytz
import datetime
import json


initialize_app()

"""
expected object:
    start
    end
    distance
    name
    start_time
    end_time

detected object:
    start
    end
    distance
    expected : bool
    timestamp
    name

"""
# test

CORRECTION_FACTOR = 2

# add timestamp in order to filter frontend from last refresh!!!!    

def requested_refresh(db) -> bool:
    order = db.collection('orders').limit(1).get()

    refresh = False
    if order:
        order = order[0]
        order_dict = order.to_dict()
        refresh = order_dict['refresh']
        if refresh:
            order_ref = order.reference
            order_ref.update({'refresh': False})
            return True

@https_fn.on_request()
def on_add_detected_object(request: https_fn.Request) -> https_fn.Response:
    db = firestore.client()

    # if requested refresh, then clear the detected objects and reset refresh
    if requested_refresh(db):
        response_dict = {
            'refresh': True
        }
        return json.dumps(response_dict)

    """
    if request.method != "POST":
        return https_fn.Response("Forbidden", status=403)
    """
    timestamp = datetime.datetime.now(tz=pytz.UTC)
    start_angle = int(request.args.get('start'))
    end_angle = int(request.args.get('end'))
    distance = int(request.args.get('distance'))

    expected_objects_ref = db.collection('expected_objects')
    detected_objects_ref = db.collection('detected_objects')

    # Query for matching expected objects
    expected_objects = expected_objects_ref.where(filter=FieldFilter('end_time', '>=', timestamp)).stream()

    informed_object = None
    overlap = 0
    for expected_object in expected_objects:

        logger.log('This message will be logged')

        expected_object = expected_object.to_dict()
        if expected_object['start_time'] > timestamp:
            continue

        object_start = expected_object['start']
        object_end = expected_object['end']
        if object_start > object_end:
            temp = object_end
            object_end = object_start
            object_start = temp
        
        match_end = min(object_end, end_angle)
        match_start = max(object_start, object_end)

        if match_end < match_start:
            continue
        print(overlap)
        current_overlap = match_end - match_start
        if current_overlap > overlap:
            overlap = current_overlap
            print(overlap)
            
            if expected_object['start_time'] <= timestamp:
                informed_object = {
                    'start': expected_object['start'],
                    'end': expected_object['end'],
                    'distance': expected_object['distance'],
                    'expected': True,
                    'timestamp': timestamp,
                    'name': expected_object['name']
                }

    if informed_object is None:
        # No match, save the incoming object data to detected_objects
        informed_object = {
            'start': start_angle,
            'end': end_angle,
            'distance': distance,
            'expected': False,
            'timestamp': timestamp,
            'name':'unexpected'
        }
    
    detected_objects_ref.add(informed_object)
    
    response_dict = {
        'object': informed_object,
    }
    return  https_fn.Response(response=json.dumps(response_dict, default=str), content_type="json")


