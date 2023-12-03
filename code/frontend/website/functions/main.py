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


CORRECTION_FACTOR = 2

@https_fn.on_request()
def on_add_detected_object(request: https_fn.Request) -> https_fn.Response:
    if request.method != "POST":
        return https_fn.Response("Forbidden", status=403)
    
    timestamp = datetime.datetime.now(tz=pytz.UTC)
    start_angle = int(request.args.get('start'))
    end_angle = int(request.args.get('end'))
    distance = int(request.args.get('distance'))

    db = firestore.client()
    expected_objects_ref = db.collection('expected_objects')
    detected_objects_ref = db.collection('detected_objects')

    # Query for matching expected objects
    expected_objects = expected_objects_ref.where(filter=FieldFilter('end_time', '>=', timestamp)).stream()

    informed_object = None
    for expected_object in expected_objects:
        expected_object = expected_object.to_dict()
        if expected_object['start_time'] <= timestamp \
            and expected_object['start'] <= start_angle + CORRECTION_FACTOR \
            and expected_object['end'] >= end_angle - CORRECTION_FACTOR:
            informed_object = {
                'start': expected_object['start'],
                'end': expected_object['end'],
                'distance': expected_object['distance'],
                'expected': True,
                'timestamp': timestamp,
                'name': expected_object['name']
            }
            break

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

    return json.dumps(informed_object, default=str)
