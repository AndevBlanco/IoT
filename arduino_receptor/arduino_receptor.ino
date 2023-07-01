#include <SPI.h>
#include <RH_RF95.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define RFM95_CS 15
#define RFM95_RST 5
#define RFM95_INT 4

RH_RF95 rf95(RFM95_CS, RFM95_INT);

WiFiClient wClient;
PubSubClient mqtt_client(wClient);


// Update these with values suitable for your network.
const char* ssid = "AndroidAP0F2C";
const char* password = "vstb6853";
const char* mqtt_server = "34.175.107.202";
//const char* mqtt_user = "infind";
//const char* mqtt_pass = "zancudo";

// cadenas para topics e ID
char ID_PLACA[16];
char topic_PUB[256];
char topic_SUB[256];
char topic_PUB_datos[256];
char topic_PUB_led[256];

//-----------------------------------------------------
void conecta_wifi() {
  Serial.printf("\nConnecting to %s:\n", ssid);
 
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
  }
  Serial.printf("\nWiFi connected, IP address: %s\n", WiFi.localIP().toString().c_str());
}

//-----------------------------------------------------
void conecta_mqtt() {
  // Loop until we're reconnected
  while (!mqtt_client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect con ultima voluntad
    if (mqtt_client.connect("ESP8266Client")) {
      Serial.printf(" conectado a broker: %s\n",mqtt_server);
      //mqtt_client.subscribe(topic_SUB);
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqtt_client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//-----------------------------------------------------
void callback(char* topic, byte* payload, unsigned int length) {
  char *mensaje = (char *)malloc(length+1); // reservo memoria para copia del mensaje
  strncpy(mensaje, (char*)payload, length); // copio el mensaje en cadena de caracteres
  mensaje[length]='\0'; // caracter cero marca el final de la cadena
  Serial.printf("Mensaje recibido [%s] %s\n", topic, mensaje);
  char infoled[128];  
  // compruebo el topic
  /*if(strcmp(topic, topic_SUB)==0) 
  {
    StaticJsonDocument<512> root;
    DeserializationError error = deserializeJson(root,mensaje,length);

    if (error){
      Serial.print("Error deserializeJson() failed: ");
      Serial.println(error.c_str());
    }else
    Serial.println(mensaje);
    if(root.containsKey("level"))  // comprobar si existe el campo/clave que estamos buscando
    { 
     int valor = root["level"];
     Serial.print("Mensaje OK, level = ");
     Serial.println(valor);
     analogWrite(LED1, valor);   // Turn the LED on (Note that LOW is the voltage level 
     led=valor;
     snprintf(infoled, 128, "{\"led\": %lu}", valor);
     
     mqtt_client.publish(topic_PUB_led, infoled);
    }else
    {
      Serial.print("Error : ");
      Serial.println("\"level\" key not found in JSON");
    }
  } // if topic
  else
  {
    Serial.println("Error: Topic desconocido");
  }  */
  free(mensaje);
}

void setup() {
  Serial.begin(9600);

  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
  delay(100);

  if (!rf95.init()) {
    Serial.println("Failed to initialize RFM95");
    while (1);
  }

  // crea topics usando id de la placa
  //sprintf(ID_PLACA, "ESP_%d", ESP.getChipId());
  sprintf(topic_PUB, "device/data");
  sprintf(topic_SUB, "device/data");
 // sprintf(topic_PUB_datos, "infind/GRUPO6/datos");
  //sprintf(topic_PUB_led, "infind/GRUPO6/led/status");  
  conecta_wifi();
  mqtt_client.setServer(mqtt_server, 1883);
  mqtt_client.setBufferSize(512); // para poder enviar mensajes de hasta X bytes
  mqtt_client.setCallback(callback);
  conecta_mqtt();
  //Serial.printf("Identificador placa: %s\n", ID_PLACA );
  //Serial.printf("Topic publicacion  : %s\n", topic_PUB);
  //Serial.printf("Topic subscripcion : %s\n", topic_SUB);
  //Serial.printf("Termina setup en %lu ms\n\n",millis());  
  
  
  //mensaje retenido de online
  //mqtt_client.publish("infind/GRUPO6/conexion","{\"online\":true}",true);


  Serial.println("RFM95 initialized");
}

void loop() {

  if (rf95.available())
  {
    // Should be a message for us now   
    uint8_t buf[56];
    memset(buf, 0, sizeof(buf));
    uint8_t len = sizeof(buf);
    if (rf95.recv(buf, &len))
    {
//      RH_RF95::printBuffer("request: ", buf, len);
      buf[len] = '\0'; // Agregar carácter nulo al final de la cadena
      Serial.print("got request: ");
      Serial.println((char*)buf);

      char receivedData[100];
      strcpy((char*)receivedData, (char*)buf); // Copiar los datos recibidos a un nuevo arreglo de caracteres
      
      char t_str[10];
      char h_str[10];
      int luz;
      char tierra_str[10];
      
      int matches = sscanf(receivedData, "t = %[^°]°C, h es: %[^%%]%%, luz es: %d, tierra es: %s", t_str, h_str, &luz, tierra_str);
      
      if (matches == 4) {
        // Los valores se extrajeron correctamente
        Serial.print("t = ");
        Serial.println(t_str);
        Serial.print("h es: ");
        Serial.println(h_str);
        Serial.print("luz es: ");
        Serial.println(luz);
        Serial.print("tierra es: ");
        Serial.println(tierra_str);
      }
      else {
        // Hubo un error al extraer los valores
        Serial.println("Error al extraer los valores");
      }

      DynamicJsonDocument doc(256); // Tamaño adecuado para tu caso
      doc["ambient_temperature"] = t_str; // Implementa una función randomFloat() para generar valores aleatorios
      doc["ambient_humidity"] = h_str;
      doc["soil_moisture"] = tierra_str;
      doc["ambient_light"] = luz;

      char jsonBuffer[256];
      serializeJson(doc, jsonBuffer);

      mqtt_client.publish(topic_PUB, jsonBuffer);

//      Serial.print("RSSI: ");
//      Serial.println(rf95.lastRssi(), DEC);
      delay(1000);
      // Send a reply
      uint8_t data[] = "And hello back to you";
      rf95.send(data, sizeof(data));
      rf95.waitPacketSent();
      Serial.println("Sent a reply");
    }
    else
    {
      Serial.println("recv failed");
    }
  }
}
