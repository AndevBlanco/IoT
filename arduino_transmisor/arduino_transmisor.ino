#include <Arduino_FreeRTOS.h>
#include "DHT.h"
#include <RH_RF95.h>
#include <Servo.h>


//Defino el pin y modelo del dht11
#define DHTPIN 8
#define DHTTYPE DHT11 
//Defino los puertos del modulo lora
#define RFM95_CS 10
#define RFM95_RST 9
#define RFM95_INT 2

//Defino mi conexion servo
Servo myservo;

//instancio lora y dht11 (importante el orden sino da error)
RH_RF95 rf95(RFM95_CS, RFM95_INT);
DHT dht(DHTPIN, DHTTYPE);

//Defino la tarea
void Sensores(void *pvParameters);
void Respuesta(void *pvParameters);

void setup() {
  // Inicializo el Serie
  Serial.begin(9600);
  // Creación de tareas
  xTaskCreate(Sensores, "Sensores", 256, NULL, 1, NULL);
  xTaskCreate(Respuesta, "Respuesta", 128, NULL, 2, NULL);
  // Inicializo el dht11
  dht.begin();
  // Inicializo el LoRa
  rf95.init();


  //Inicializo el servo al pin 7
  myservo.attach(4);
  myservo.write(0);

  Serial.println(F("funciona"));

}

void loop() {
  // El código en loop() no se ejecutará ya que el planificador de tareas se ha iniciado
}

//Tarea de lectura de sensores y envio de estos
void Sensores(void *pvParameters) {
  while (1) {

    //Leo los sensores
    float t = dht.readTemperature();
    //Serial.println(t);
    float h = dht.readHumidity();
    //Serial.println(h);
    //valor entre 0 y 1023
    int luz = analogRead(A1);
    //Serial.println(luz);
    int tierra = analogRead(A0);
    //valor entre 0 - 200
    float tierra_humedad= map(tierra, 0, 200, 0, 100);

    int angle = myservo.read();
    //Serial.println(angle);

    Serial.print(F("Sending"));
    // Creo el mensaje con los datos
    char t_str[10];
    char h_str[10];
    char tierra_str[10];

    dtostrf(t, 5, 2, t_str);  // Convierte temperature en una cadena con 5 caracteres en total y 2 decimales
    dtostrf(h, 5, 2, h_str);  // Convierte temperature en una cadena con 5 caracteres en total y 2 decimales
    dtostrf(tierra_humedad, 5, 2, tierra_str);  // Convierte temperature en una cadena con 5 caracteres en total y 2 decimales

        // Calcular el tamaño necesario para el arreglo de caracteres
    int dataSize = snprintf(nullptr, 0, "t: %s°C, h: %s%%, luz: %d, se: %d, ti: %s", t_str, h_str, luz, angle, tierra_str);

    // Crear el arreglo de caracteres con el tamaño necesario
    char data[dataSize + 1]; // +1 para el carácter nulo al final

    sprintf(data, "t: %s°C, h: %s%%, luz: %d, se: %d, ti: %s", t_str, h_str, luz, angle, tierra_str);
    // Envio el mensaje
    //Serial.println(dataSize);
    Serial.println((char*)data);
    rf95.send((uint8_t*)data, strlen(data));
    rf95.waitPacketSent();


    vTaskDelay(10000 / portTICK_PERIOD_MS); // Retraso de 3 segundos

  }
}


void Respuesta(void *pvParameters) {
  while (1) {

    //Serial.println(F("respuesta ?"));


    /*myservo.write(180);

    int angle = myservo.read();
    Serial.println(angle);*/
    char data2[70];
    uint8_t len = sizeof(data2);
    // Compruebo si hay respuesta
    if (rf95.waitAvailableTimeout(3000))
    { 
      if (rf95.recv(data2, &len))
    {
        Serial.print(F("reply: "));
        Serial.println((char*)data2);
        if (strcmp((char*)data2, "close") == 0){
          //Serial.print(F("abre"));
          myservo.write(0);
        }
        else if (strcmp((char*)data2, "open") == 0){
          //Serial.print(F("cierra"));
          myservo.write(180);
        }
      }
      else
      {
        Serial.println(F("failed"));
      }
    }
    else
    {
      //Serial.println(F("running?"));
    }

    // Delay opcional para evitar bloquear completamente la CPU
    vTaskDelay(100 / portTICK_PERIOD_MS); // Retraso de 3 segundos
  }
}