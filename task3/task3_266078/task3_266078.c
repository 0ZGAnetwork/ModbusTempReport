#include <stdio.h>                          // Standard input/output
#include "pico/stdlib.h"                     // Raspberry Pi Pico standard library
#include "FreeRTOS.h"                        // FreeRTOS core definitions
#include "queue.h"                           // FreeRTOS queue
#include "task.h"                            // FreeRTOS task management

#define LED0_GPIO 5                          // L0
#define LED1_GPIO 4                          // L1
#define LED2_GPIO 3                          // L2
#define BUTTON1_GPIO 2                       // B0

QueueHandle_t coreQueue;                    // Queue handle for passing button data

typedef struct {
    int buttonState;                        // 1 if pressed, 0 if released
    int coreid;                             // ID of the core that handled the button
} ButtonData;

void task1_blink_led2(void *pvParameters) {
    gpio_init(LED2_GPIO);                    // Initialize LED2 pin
    gpio_set_dir(LED2_GPIO, GPIO_OUT);       // Set as output

    while (1) {
        gpio_put(LED2_GPIO, 1);              // Turn on LED2
        printf("LED2 ON\n");
        vTaskDelay(pdMS_TO_TICKS(3));       // Wait 33ms
        //busy_wait_ms(200);
        gpio_put(LED2_GPIO, 0);              // Turn off LED2
        printf("LED2 OFF\n");
        vTaskDelay(pdMS_TO_TICKS(6));       // Wait 67ms
        //busy_wait_ms(200);
    }
}

void task2_button_press(void *pvParameters) {
    gpio_init(BUTTON1_GPIO);                 // Initialize button GPIO
    gpio_set_dir(BUTTON1_GPIO, GPIO_IN);     // Set button pin as input
    gpio_pull_up(BUTTON1_GPIO);              // Enable internal pull-up resistor

    ButtonData data;
    int lastButtonState = 1;                 // Store previous button state (1 = not pressed)

    while (1) {
        int currentButtonState = gpio_get(BUTTON1_GPIO); // Read button state

        if (currentButtonState == 0 && lastButtonState == 1) {
            data.buttonState = 1;              // Button was just pressed
            //data.coreid = 1;
            data.coreid = get_core_num();                   // We are on core 0 or 1
            printf("BUTTON PRESS on core %d!\n", data.coreid);
            xQueueSend(coreQueue, &data, portMAX_DELAY); // Send data to queue
        } else if (currentButtonState == 1 && lastButtonState == 0) {
            data.buttonState = 0;              // Button was just released
            xQueueSend(coreQueue, &data, portMAX_DELAY); //or define time:pdMS_TO_TICKS(10)
            printf("BUTTON RELEASED\n");
        }

        lastButtonState = currentButtonState;  // Update last state
        vTaskDelay(pdMS_TO_TICKS(50));         // Debounce delay (50ms)
    }
}

void task3_process_queue(void *pvParameters) {
    ButtonData receivedData;

    gpio_init(LED0_GPIO);                     // Initialize LED0 pin
    gpio_set_dir(LED0_GPIO, GPIO_OUT);        // Set as output

    gpio_init(LED1_GPIO);                     // Initialize LED1 pin
    gpio_set_dir(LED1_GPIO, GPIO_OUT);        // Set as output

    while (1) {
        if (xQueueReceive(coreQueue, &receivedData, portMAX_DELAY) == pdPASS) {
            if (receivedData.buttonState == 1) {
                printf("Button pressed on core %d\n", receivedData.coreid);

                if (receivedData.coreid == 0) {
                    gpio_put(LED0_GPIO, 1);             // Turn on LED0 for core 0
                    vTaskDelay(pdMS_TO_TICKS(1000));     // Wait 100ms
                    gpio_put(LED0_GPIO, 0);             // Turn off LED0
                } else if (receivedData.coreid == 1) {
                    gpio_put(LED1_GPIO, 1);             // Turn on LED1 for core 1
                    vTaskDelay(pdMS_TO_TICKS(1000));     // Wait 100ms
                    gpio_put(LED1_GPIO, 0);             // Turn off LED1
                }
            } else {
                printf("Button released\n");
            }
        }
    }
}

int main() {
    stdio_init_all();                          // Initialize USB serial communication

    coreQueue = xQueueCreate(16, sizeof(ButtonData));  // Create queue 
    if (coreQueue == NULL) {
        printf("Failed to create queue\n");
        return 1;
    }

    // Create tasks for core 0
    xTaskCreate(task1_blink_led2, "Task1_BlinkLED2", 256, NULL, 1, NULL);
    xTaskCreate(task2_button_press, "Task2_ButtonPress", 256, NULL, 2, NULL);
    xTaskCreate(task3_process_queue, "Task3_ProcessQueue", 256, NULL, 3, NULL);

    vTaskStartScheduler();  // Start FreeRTOS scheduler

    return 0;
}
