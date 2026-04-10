#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>

#include "pico/stdlib.h"
#include <stdio.h>

const int BTN_PIN_R = 28;
const int BTN_PIN_G = 26;

const int LED_PIN_R = 4;
const int LED_PIN_G = 6;

QueueHandle_t xQueueDelayR;
QueueHandle_t xQueueDelayG;


void led_1_task(void *p) {
    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);

    int delay = 0;
    while (true) {
        if (xQueueReceive(xQueueDelayR, &delay, 0)) {
            printf("Novo delay R: %d\n", delay);
        }

        if (delay > 0) {
            gpio_put(LED_PIN_R, 1);
            vTaskDelay(pdMS_TO_TICKS(delay));
            gpio_put(LED_PIN_R, 0);
            vTaskDelay(pdMS_TO_TICKS(delay));
        } else {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}

void btn_1_task(void *p) {
    gpio_init(BTN_PIN_R);
    gpio_set_dir(BTN_PIN_R, GPIO_IN);
    gpio_pull_up(BTN_PIN_R);

    int delay = 0;
    while (true) {
        if (!gpio_get(BTN_PIN_R)) {
            while (!gpio_get(BTN_PIN_R)) {
                vTaskDelay(pdMS_TO_TICKS(10)); 
            }

            if (delay < 1000) {
                delay += 100;
            } else {
                delay = 100;
            }
            printf("Enviando delay R: %d \n", delay);
            
            xQueueSend(xQueueDelayR, &delay, 0);
        }
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}


void led_2_task(void *p) {
    gpio_init(LED_PIN_G);
    gpio_set_dir(LED_PIN_G, GPIO_OUT);

    int delay = 0;
    while (true) {
        if (xQueueReceive(xQueueDelayG, &delay, 0)) {
            printf("Novo delay G: %d\n", delay);
        }

        if (delay > 0) {
            gpio_put(LED_PIN_G, 1);
            vTaskDelay(pdMS_TO_TICKS(delay));
            gpio_put(LED_PIN_G, 0);
            vTaskDelay(pdMS_TO_TICKS(delay));
        } else {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}

void btn_2_task(void *p) {
    gpio_init(BTN_PIN_G);
    gpio_set_dir(BTN_PIN_G, GPIO_IN);
    gpio_pull_up(BTN_PIN_G);

    int delay = 0;
    while (true) {
        if (!gpio_get(BTN_PIN_G)) {
            
            while (!gpio_get(BTN_PIN_G)) {
                vTaskDelay(pdMS_TO_TICKS(10));
            }

            if (delay < 1000) {
                delay += 100;
            } else {
                delay = 100;
            }
            printf("Enviando delay G: %d \n", delay);
            
            xQueueSend(xQueueDelayG, &delay, 0);
        }
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

int main() {
    stdio_init_all();
    printf("Start RTOS \n");

    xQueueDelayR = xQueueCreate(32, sizeof(int));
    xQueueDelayG = xQueueCreate(32, sizeof(int));

    xTaskCreate(led_1_task, "LED_Task 1", 256, NULL, 1, NULL);
    xTaskCreate(btn_1_task, "BTN_Task 1", 256, NULL, 1, NULL);

    xTaskCreate(led_2_task, "LED_Task 2", 256, NULL, 1, NULL);
    xTaskCreate(btn_2_task, "BTN_Task 2", 256, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true)
        ;
}