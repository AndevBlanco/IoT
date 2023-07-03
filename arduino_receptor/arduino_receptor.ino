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

int mqttprority=0;


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
      Serial.printf(" conectado al broker: %s\n",mqtt_server);
      mqtt_client.subscribe(topic_SUB);
      Serial.printf("Suscrito al tema: %s\n", topic_SUB); // Agrega esta línea para mostrar el resultado de la suscripción
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
  //Serial.printf("Mensaje recibido [%s] %s\n", topic, mensaje);
  char infoled[128];  
  // compruebo el topic
  if(strcmp(topic, topic_SUB)==0) 
  {

    //Serial.println("llega respuesta del mqtt");
    //Serial.println(mensaje);

    int valor = atoi(mensaje);

    uint8_t data[6] ;

    if (valor == 0) {
      sprintf(reinterpret_cast<char*>(data), "open");
      Serial.println("abre desde el mqtt");
    } else if (valor == 180){
      sprintf(reinterpret_cast<char*>(data), "close");
      Serial.println("cierra desde el mqtt");
    }

    rf95.send(data, sizeof(data));
    rf95.waitPacketSent();
    //Serial.println("Sent a reply");
  } // if topic
  else
  {
    Serial.println("Error: Topic desconocido");
  }  
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

  // crea topics 
  sprintf(topic_PUB, "device/data");
  sprintf(topic_SUB, "device/response");

  conecta_wifi();
  mqtt_client.setServer(mqtt_server, 1883);
  mqtt_client.setBufferSize(512); // para poder enviar mensajes de hasta X bytes
  mqtt_client.setCallback(callback);
  conecta_mqtt();

  Serial.println("RFM95 initialized");
}

void loop() {

  if (!mqtt_client.connected()) conecta_mqtt();
  mqtt_client.loop();
  if (rf95.available())
  {
    // Should be a message for us now   
    uint8_t buf[100];
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
    int servo;
    

    int matches = sscanf(receivedData, "t: %[^°]°C, h: %[^%%]%%, luz: %d, se: %d, ti: %s", t_str, h_str, &luz, &servo, tierra_str);


    if (matches == 5)
    {
        // Los valores se extrajeron correctamente
        /*Serial.print("t = ");
        Serial.println(t_str);
        Serial.print("h es: ");
        Serial.println(h_str);
        Serial.print("luz es: ");
        Serial.println(luz);
        Serial.print("tierra es: ");
        Serial.println(tierra_str);
        Serial.print("servo es: ");
        Serial.println(servo);*/
    }
    else
    {
        // Hubo un error al extraer los valores
        Serial.println("Error al extraer los valores");
    }

   /* char* servoPtr = strstr(receivedData, "se: ");
  if (servoPtr != nullptr)
  {
    // Avanzar hasta el comienzo del valor del servo
    servoPtr += 4;

    // Extraer el valor del servo
    int servo = atoi(servoPtr);

    // Imprimir el valor del servo
    Serial.print("servo es: ");
    Serial.println(servo);
  }
  else
  {
    // No se encontró el valor del servo en la cadena
    Serial.println("Error al extraer el valor del servo");
  }*/



      DynamicJsonDocument doc(256); // Tamaño adecuado para tu caso
      doc["ambient_temperature"] = t_str; // Implementa una función randomFloat() para generar valores aleatorios
      doc["ambient_humidity"] = h_str;
      doc["soil_moisture"] = tierra_str;
      doc["servo"] = servo;
      doc["ambient_light"] = luz;

      char jsonBuffer[256];
      serializeJson(doc, jsonBuffer);

      mqtt_client.publish(topic_PUB, jsonBuffer);


      delay(1000);
      // Send a reply

      uint8_t data[6] ;

      float tierra = std::stof(tierra_str);

      if (luz >= 341 || tierra <= 30) {
        sprintf(reinterpret_cast<char*>(data), "open");
      } else {
        sprintf(reinterpret_cast<char*>(data), "close");
      }

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
