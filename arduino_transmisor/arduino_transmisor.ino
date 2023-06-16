#include <Arduino_FreeRTOS.h>
#include "DHT.h"
#include <RH_RF95.h>

//Defino el pin y modelo del dht11
#define DHTPIN 8
#define DHTTYPE DHT11 
//Defino los puertos del modulo lora
#define RFM95_CS 10
#define RFM95_RST 9
#define RFM95_INT 2

//instancio lora y dht11 (importante el orden sino da error)
RH_RF95 rf95(RFM95_CS, RFM95_INT);
DHT dht(DHTPIN, DHTTYPE);

//Defino la tarea
void Sensores(void *pvParameters);

void setup() {
  // Inicializo el Serie
  Serial.begin(9600);
  // Creación de tareas
  xTaskCreate(Sensores, "Sensores", 400, NULL, 1, NULL);
  // Inicializo el dht11
  dht.begin();
  // Inicializo el LoRa
  rf95.init();

  Serial.println("funciona");

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
    int luz = analogRead(A1);
    //Serial.println(luz);
    int tierra = analogRead(A0);
    float tierra_humedad= map(tierra, 0, 1023, 0, 100);

    Serial.println("Sending to rf95_server");
    // Creo el mensaje con los datos
    char data[70];
    char t_str[10];
    char h_str[10];
    char tierra_str[10];

    dtostrf(t, 5, 2, t_str);  // Convierte temperature en una cadena con 5 caracteres en total y 2 decimales
    dtostrf(h, 5, 2, h_str);  // Convierte temperature en una cadena con 5 caracteres en total y 2 decimales
    dtostrf(tierra_humedad, 5, 2, tierra_str);  // Convierte temperature en una cadena con 5 caracteres en total y 2 decimales


    sprintf(data, "t = %s°C, h es: %s%%, luz es: %d, tierra es: %s", t_str, h_str, luz, tierra_str);
    // Envio el mensaje
    Serial.println((char*)data);
    rf95.send((uint8_t*)data, strlen(data));
    rf95.waitPacketSent();

    uint8_t len = sizeof(data);
    // Compruebo si hay respuesta
    if (rf95.waitAvailableTimeout(3000))
    { 
      if (rf95.recv(data, &len))
    {
        Serial.print("got reply: ");
        Serial.println((char*)data);
      }
      else
      {
        Serial.println("recv failed");
      }
    }
    else
    {
      Serial.println("No reply, is rf95_server running?");
    }

    // Delay opcional para evitar bloquear completamente la CPU
    vTaskDelay(3000 / portTICK_PERIOD_MS); // Retraso de 3 segundos
  }
}