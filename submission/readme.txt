## Devices

### Sonar

The sonar arduino's responsabilities are:

- Scanning the area ahead.
- Report any detected objects to the cloud server.
- Forward the responses to the Notifier/Broadcaster arduino if it concerns them.

### Notifier

The Notifier arduino's responsabilities are:

- Listen for serial messages from the sonar.
- Parse the messages and extract the important components.
- Report this important information to registered devices over bluetooth

### Alarm

The Alarm arduino's responsabilities are:

- Listen for messages from the notifier.
- If they report on an expected object, light a green light for a predetermined amount of time.
- If they report on an unexpected object, sound the alarm for a predetermined amount of time.

### Video(s)

https://drive.google.com/drive/folders/1oc-qbnJBWB4S8JDzz71KJy5U568kDa8S?usp=sharing
