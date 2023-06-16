#include <Arduino_FreeRTOS.h>
#include <RH_RF95.h>

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
  xTaskCreate(Task1, "Task1", 400, NULL, 2, &task1Handle);

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
    

    if (rf95.available())
  {
    // Should be a message for us now   
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if (rf95.recv(buf, &len))
    {
      
//      RH_RF95::printBuffer("request: ", buf, len);
      Serial.print("got request: ");
      Serial.println((char*)buf);
//      Serial.print("RSSI: ");
//      Serial.println(rf95.lastRssi(), DEC);
      
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
    // Delay opcional para evitar bloquear completamente la CPU
    vTaskDelay(pdMS_TO_TICKS(2000)); // Retraso de 1000 ms
  }
}
