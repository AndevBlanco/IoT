//bibliotecas a incluir (algunas son de base)
#include <SPI.h>
#include <LoRa.h>
#include <FreeRTOS.h>
#include <task.h>

#define LORA_SS 10
#define LORA_RST 9
#define LORA_DIO0 2


// Definir el tamaño de la pila para la tarea
#define STACK_SIZE 128

// Declaración de las funciones de tarea
void task1(void *pvParameters);
void task2(void *pvParameters);

// Declaración de los manejadores de las tareas
TaskHandle_t task1Handle;
TaskHandle_t task2Handle;


//configuracion inicial 
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

  //PARTE freeRTOS para la creacion de tareas / hilos

  //creo las tareas
  xTaskCreate(task1, "Task1", STACK_SIZE, NULL, 1, &task1Handle);
  xTaskCreate(task2, "Task2", STACK_SIZE, NULL, 2, &task2Handle);


}


//el loop para ejecucion en bucle
void loop() {


  // put your main code here, to run repeatedly:
  String mensaje = "¡Hola desde el Arduino 1!"; // El mensaje que deseas enviar

  // Envía el mensaje
  LoRa.beginPacket();
  LoRa.print(mensaje);
  LoRa.endPacket();

  Serial.println("Mensaje enviado desde el Arduino 1: " + mensaje);

  delay(5000); // Espera 5 segundos antes de enviar el siguiente mensaje

  / Tarea 1
void task1(void *pvParameters) {
  while (1) {
    // Código de la tarea 1 aquí
    
    vTaskDelay(pdMS_TO_TICKS(1000)); // Retardo de 1 segundo
  }
}

// Tarea 2
void task2(void *pvParameters) {
  while (1) {
    // Código de la tarea 2 aquí
    
    vTaskDelay(pdMS_TO_TICKS(500)); // Retardo de 500 milisegundos
  }
}


}


