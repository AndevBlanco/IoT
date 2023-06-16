#include <Arduino_FreeRTOS.h>
#include "DHT.h"
#include <RH_RF95.h>

#define DHTPIN 4
#define DHTTYPE DHT11 

DHT dht(DHTPIN, DHTTYPE);

int t;
int h;



#define RFM95_CS 10
#define RFM95_RST 9
#define RFM95_INT 2

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);


// Definición de las tareas
void Task1(void *pvParameters);

// Variables globales
TaskHandle_t task1Handle;

void setup() {
  // Inicialización de Arduino y otras configuraciones
  Serial.begin(9600);
  // Creación de tareas
  xTaskCreate(Task1, "Task1", 450, NULL, 2, &task1Handle);

  dht.begin();

  if (!rf95.init())
    Serial.println("init failed");
  Serial.println("funciona");

}

void loop() {
  // El código en loop() no se ejecutará ya que el planificador de tareas se ha iniciado
}

// Implementación de las tareas
void Task1(void *pvParameters) {
  while (1) {
    // Código de la tarea 1
    //Serial.println("tarea 1");
    t = int(dht.readTemperature() * 10);
    Serial.print(F("La temperatura es: "));
    Serial.println(t/10);
    h = int(dht.readHumidity() * 10);
    Serial.print(F("La temperatura es: "));
    Serial.println(h/10);

    int luz = analogRead(A1);
    Serial.print(F("Valor de luz: "));
    Serial.println(luz);
    int tierra = analogRead(A0);
    float tierra_humedad= map(tierra, 0, 1023, 0, 100);
    Serial.print(F("Valor de tierra: "));
    Serial.println(tierra);

    Serial.println("Sending to rf95_server");
  // Send a message to rf95_server
  uint8_t data[] = "Hello World!";
  rf95.send(data, sizeof(data));
  
  rf95.waitPacketSent();
  // Now wait for a reply
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);

  if (rf95.waitAvailableTimeout(3000))
  { 
    // Should be a reply message for us now   
    if (rf95.recv(buf, &len))
   {
      Serial.print("got reply: ");
      Serial.println((char*)buf);
//      Serial.print("RSSI: ");
//      Serial.println(rf95.lastRssi(), DEC);    
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
    vTaskDelay(3000 / portTICK_PERIOD_MS); // Retraso de 1000 ms
  }
}
