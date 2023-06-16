import json, requests, random, datetime

FUNCTION_URL='https://europe-central2-stoked-dryad-385005.cloudfunctions.net/sensors'
# FUNCTION_URL='http://127.0.0.1:8080'

msg=dict({
    'temperature': '{:.2f}'.format(random.uniform(10, 40)),
    'water_level': '{:.2f}'.format(random.uniform(100, 150)),
    'humidity': '{:.2f}'.format(random.uniform(70, 100)),
    'date': datetime.datetime.now()
    })
json_data=json.dumps(msg,default=str)
newHeaders = {'Content-type': 'application/json', 'Accept': 'application/json'}

response = requests.post(FUNCTION_URL, data=json_data, headers=newHeaders)

print(response)
print(response.content)