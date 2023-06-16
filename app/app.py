from flask import Flask, jsonify, render_template
from flask_socketio import SocketIO
from flask_debugtoolbar import DebugToolbarExtension
import json, os
from google.cloud import bigquery
from google.cloud.bigquery.table import Row
import paho.mqtt.client as mqtt

app = Flask(__name__)
app.config['DEBUG_TB_INTERCEPT_REDIRECTS'] = False
app.config['TEMPLATES_AUTO_RELOAD'] = True
socketio = SocketIO(app)
mqtt_subscriber = None

os.environ["GOOGLE_APPLICATION_CREDENTIALS"] = "./stoked-dryad-385005-5a579b96bb15.json"
bigquery_client = bigquery.Client()
table_ref = bigquery_client.dataset("IoT_project").table("sensors_data")
table = bigquery_client.get_table(table_ref)

# Flask route to get the data
@app.route('/')
def home():
    if mqtt_subscriber is not None:
        print("data")
    else:
        print("MQTT Subscriber not initialized")

    return render_template('index.html', data="data")


def on_message(client, userdata, msg):
    print("Message content: " + msg.payload.decode())
    data = msg.payload.decode("utf-8")
    print(type(data))
    response_dict = json.loads(msg.payload)
    print(response_dict)
    print(type(response_dict))
    rows_to_insert = [ response_dict ]
    errors = bigquery_client.insert_rows(table, rows_to_insert)
    query_job = bigquery_client.query(f"SELECT * FROM `{table_ref}`")
    rows = query_job.result()
    historical = []
    for i in rows:
        historical.append({
            'ambient_humidity': i[1],
            'ambient_temperature': i[2],
            'ambient_light': i[3],
            'soil_moisture': i[4]
        })


    socketio.emit('data', {'last': response_dict, 'historical': historical})

client = None

if __name__ == '__main__':
    client = mqtt.Client()
    client.on_message = on_message
    client.connect("34.175.107.202", 1883)
    client.subscribe("device/data")
    client.loop_start()

    socketio.run(app, host='localhost', port=5000)
    # app.run(debug=True)
