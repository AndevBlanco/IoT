#include <SPI.h>
#include <LoRa.h>

#define LORA_SS 10
#define LORA_RST 9
#define LORA_DIO0 2

void setup() {
  Serial.begin(9600);
  while (!Serial);

  //Configuro el modulo lora con sus pines correspondientes
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0); 

  //inicio la comunicacion  a la frecuencia de 433 MHZ
  if (!LoRa.begin(433E6)) { 
    Serial.println("Error al iniciar el módulo LoRa");
    while (1);
  }

  //Se ha iniciado correctamente
  Serial.println("Módulo LoRa iniciado correctamente.");
}

void loop() {
  // Verifica si hay paquetes entrantes
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String mensaje = "";

    // Lee el mensaje recibido
    while (LoRa.available()) {
      mensaje += (char)LoRa.read();
    }

    Serial.println("Mensaje recibido en el Arduino 2: " + mensaje);
  }
}
