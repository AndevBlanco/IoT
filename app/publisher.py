import random, json, time, datetime
import paho.mqtt.client as mqtt

# Configurar el cliente MQTT
client = mqtt.Client()
client.connect("34.175.107.202", 1883)

mqtt_topic='device/data'
while True:
    msg=dict({
        'ambient_temperature': '{:.2f}'.format(random.uniform(10, 40)),
        'ambient_humidity': '{:.2f}'.format(random.uniform(100, 150)),
        'soil_moisture': '{:.2f}'.format(random.uniform(70, 100)),
        'ambient_light': '{:.2f}'.format(random.uniform(70, 100)),
        'date': datetime.datetime.now()
    })
    json_data=json.dumps(msg, default=str)
    client.publish(mqtt_topic, json_data, qos=1)
    print("Mensaje publicado:", json_data)
    time.sleep(5)


