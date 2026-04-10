#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h> 

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

const int BTN_PIN_R = 28;
const int BTN_PIN_Y = 21;

const int LED_PIN_R = 5;
const int LED_PIN_Y = 10;

QueueHandle_t xQueueBtn;
SemaphoreHandle_t xSemaphoreLedR;
SemaphoreHandle_t xSemaphoreLedY;

void btn_callback(uint gpio, uint32_t events) {
    static uint32_t last_time_r = 0;
    static uint32_t last_time_y = 0;
    
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (events == GPIO_IRQ_EDGE_FALL) {
        int btn_pressed = -1; 

        if (gpio == BTN_PIN_R && (current_time - last_time_r > 200)) { 
            last_time_r = current_time;
            btn_pressed = BTN_PIN_R;
        } 
        else if (gpio == BTN_PIN_Y && (current_time - last_time_y > 200)) { 
            last_time_y = current_time;
            btn_pressed = BTN_PIN_Y;
        }

        if (btn_pressed != -1) {
            xQueueSendFromISR(xQueueBtn, &btn_pressed, &xHigherPriorityTaskWoken);
        }
    }
    
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void btn_task(void* p) {
    int btn_recebido = 0;

    while (true) {
        if (xQueueReceive(xQueueBtn, &btn_recebido, portMAX_DELAY) == pdTRUE) {
            
            if (btn_recebido == BTN_PIN_R) {
                xSemaphoreGive(xSemaphoreLedR);
            } 
            else if (btn_recebido == BTN_PIN_Y) {
                xSemaphoreGive(xSemaphoreLedY);
            }
        }
    }
}

void led_r_task(void* p) {
    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);
    gpio_put(LED_PIN_R, 0);

    bool is_blinking = false;

    while (true) {
        TickType_t tempo_espera = is_blinking ? 0 : portMAX_DELAY;

        if (xSemaphoreTake(xSemaphoreLedR, tempo_espera) == pdTRUE) {
            is_blinking = !is_blinking; 
            if (!is_blinking) {
                gpio_put(LED_PIN_R, 0); 
            }
        }

        if (is_blinking) {
            gpio_put(LED_PIN_R, 1);
            vTaskDelay(pdMS_TO_TICKS(100)); 
            gpio_put(LED_PIN_R, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

void led_y_task(void* p) {
    gpio_init(LED_PIN_Y);
    gpio_set_dir(LED_PIN_Y, GPIO_OUT);
    gpio_put(LED_PIN_Y, 0);

    bool is_blinking = false;

    while (true) {
        TickType_t tempo_espera = is_blinking ? 0 : portMAX_DELAY;

        if (xSemaphoreTake(xSemaphoreLedY, tempo_espera) == pdTRUE) {
            is_blinking = !is_blinking;
            if (!is_blinking) {
                gpio_put(LED_PIN_Y, 0);
            }
        }

        if (is_blinking) {
            gpio_put(LED_PIN_Y, 1);
            vTaskDelay(pdMS_TO_TICKS(100));
            gpio_put(LED_PIN_Y, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

int main() {
    stdio_init_all();
    
    xQueueBtn = xQueueCreate(10, sizeof(int));

    xSemaphoreLedR = xSemaphoreCreateBinary();
    xSemaphoreLedY = xSemaphoreCreateBinary();

    gpio_init(BTN_PIN_R);
    gpio_set_dir(BTN_PIN_R, GPIO_IN);
    gpio_pull_up(BTN_PIN_R);
    
    gpio_init(BTN_PIN_Y);
    gpio_set_dir(BTN_PIN_Y, GPIO_IN);
    gpio_pull_up(BTN_PIN_Y);

    gpio_set_irq_enabled_with_callback(BTN_PIN_R, GPIO_IRQ_EDGE_FALL, true, &btn_callback);
    gpio_set_irq_enabled(BTN_PIN_Y, GPIO_IRQ_EDGE_FALL, true);

    xTaskCreate(btn_task, "BTN_Task", 256, NULL, 1, NULL); 
    xTaskCreate(led_r_task, "LED_Task R", 256, NULL, 1, NULL);
    xTaskCreate(led_y_task, "LED_Task Y", 256, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true);
    return 0;
}