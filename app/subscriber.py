import paho.mqtt.client as mqtt
import json

class MQTTSubscriber:
    def __init__(self, broker_address, port, topic):
        self.broker_address = broker_address
        self.port = port
        self.topic = topic

        self.client = mqtt.Client()
        self.client.on_connect = self.on_connect
        self.client.on_message = self.on_message

    def start(self):
        self.client.connect(self.broker_address, self.port)
        self.client.loop_forever()

    def on_connect(self, client, userdata, flags, rc):
        if rc == 0:
            print("Connected to MQTT broker successfully")
            # Subscribe to the specified topic upon connection
            client.subscribe(self.topic)
        else:
            print("Failed to connect to MQTT broker")

    def on_message(self, client, userdata, msg):
        print("Message received on topic: " + msg.topic)
        print("Message content: " + msg.payload.decode())

# Create an instance of MQTTSubscriber
mqtt_subscriber = MQTTSubscriber("34.175.107.202", 1883, "device/data")

# Start the MQTT subscriber
mqtt_subscriber.start()
