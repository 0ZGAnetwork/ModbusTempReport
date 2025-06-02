#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define LIS3DH_ADDR 0x19
#define I2C_PORT i2c0
#define SDA_PIN 8
#define SCL_PIN 9
#define LED_PIN 3
#define LED2_PIN 4

QueueHandle_t z_accel_queue;

void lis3dh_init() {

    uint8_t ctrl_reg1[] = {0x20, 0b01010111}; // 100Hz, all axis enabled
    i2c_write_blocking(I2C_PORT, LIS3DH_ADDR, ctrl_reg1, 2, false);
//     0b 0101 0111 = 0x57
//      Bits:
//      [7:4] ODR = 0101 → 100 Hz
//      [3]  LPen = 0 → Normal mode
//      [2]  Z axis enabled = 1
//      [1]  Y axis enabled = 1
//      [0]  X axis enabled = 1

    //uint8_t ctrl_reg4[] = {0x23, 0b00000000}; //for +/-2g     raw ~16384
    uint8_t ctrl_reg4[] = {0x23, 0b00100000};  // +/-4g        raw ~8192     
    //uint8_t ctrl_reg4[] = {0x23, 0b00010000}; // +/-8g        raw ~4096
    //uint8_t ctrl_reg4[] = {0x23, 0b00110000}; // +/-16g       raw ~2048
    i2c_write_blocking(I2C_PORT, LIS3DH_ADDR, ctrl_reg4, 2, false);
}

int16_t read_z_axis() {
    uint8_t reg = 0x2C | 0x80; // OUT_Z_L, auto-increment (upper bit set)
    uint8_t data[2];
    i2c_write_blocking(I2C_PORT, LIS3DH_ADDR, &reg, 1, true);   //send addres and wait for ACK
    i2c_read_blocking(I2C_PORT, LIS3DH_ADDR, data, 2, false);   //read address and get 2 bytes of data
    return (int16_t)(data[1] << 8 | data[0]);                   // combine low and high byte
}

void task_read_accel(void *params) {
    lis3dh_init();
    uint8_t who_am_i_reg = 0x0F; // adress 0x33 == good
    
    uint8_t who_am_i;                                           //check if device is connected & address is correct
    i2c_write_blocking(I2C_PORT, LIS3DH_ADDR, &who_am_i_reg, 1, true);
    i2c_read_blocking(I2C_PORT, LIS3DH_ADDR, &who_am_i, 1, false);
    printf("WHO_AM_I = 0x%02X\n", who_am_i);
    
    uint8_t ctrl_reg1 = 0x20;
    uint8_t ctrl_val;
    i2c_write_blocking(I2C_PORT, LIS3DH_ADDR, &ctrl_reg1, 1, true); // send information for reading CTRL_REG1
    i2c_read_blocking(I2C_PORT, LIS3DH_ADDR, &ctrl_val, 1, false);  // get 1 bait information from CTRL_REG1 & save it in ctrl_val
    printf("CTRL_REG1 = 0x%02X\n", ctrl_val);                       // Expected terminal:0x57

    while (1) {
        int16_t z = read_z_axis();
        //printf("Z-axis: %d, Z_g: %.2f\n", z, z_g);
        //printf("Z-axis: %d\n", z);
        xQueueSend(z_accel_queue, &z, 0);
        vTaskDelay(pdMS_TO_TICKS(5)); // ~200Hz
    }
}

void task_detect_tap(void *params) {
    int16_t z = 0;
    float last_z = 0;
    int tap_count = 0;
    uint32_t last_tap_time = 0;
    uint32_t last_detection_time = 0;

    while (1) {
        if (xQueueReceive(z_accel_queue, &z, portMAX_DELAY)) {
            float z_g = z * 0.000122f; // for ±4g
            float delta = z_g - last_z;
            last_z = z_g;
            float threshold = 0.6f;
            uint32_t now = xTaskGetTickCount();

            if (delta > threshold || delta < -threshold) {
                if ((now - last_detection_time) < pdMS_TO_TICKS(200)) { //to avoid multiple detections
                    continue;
                }
                last_detection_time = now;                              //initialization of last_detection_time

                if (tap_count == 0) {       //LED == one tap
                    printf("Single tap\n");
                    printf("delta: %.2f g\n", delta);
                    gpio_put(LED_PIN, 1);        
                    vTaskDelay(pdMS_TO_TICKS(100));
                    gpio_put(LED_PIN, 0);
                    tap_count = 1;
                    last_tap_time = now;
                } 
                else if ((now - last_tap_time) <= pdMS_TO_TICKS(500)) { //LED2 == double tap & time between taps is less than 500ms
                    printf("Double tap!\n");
                    printf("delta: %.2f g\n", delta);
                    for (int i = 0; i < 2; i++) {
                        gpio_put(LED2_PIN, 1);   
                        vTaskDelay(pdMS_TO_TICKS(100));
                        gpio_put(LED2_PIN, 0);
                        vTaskDelay(pdMS_TO_TICKS(100));
                    }
                    tap_count = 0;                                      // reset tap count after double tap
                }
            }
            if (tap_count == 1 && (now - last_tap_time) > pdMS_TO_TICKS(500)) { // reset tap count if no second tap within 500ms
                tap_count = 0;
            }
        } else {
            vTaskDelay(pdMS_TO_TICKS(10));  // wait for new data in the queue
        }
    }
}

void i2c_scan() {
    printf("Scanning I2C bus...\n");
    for (uint8_t addr = 0x08; addr <= 0x77; addr++) {       // Scan addresses from 0x08 to 0x77
        uint8_t dummy;                                      // Dummy variable to read data
        int result = i2c_read_blocking(I2C_PORT, addr, &dummy, 1, false);   // Try to read 1 byte from the address
        if (result >= 0) {                                  // If the read was successful, the device is present
            printf("Device found at 0x%02X\n", addr);
        }
    }
}

int main() {
    stdio_init_all();
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_init(LED2_PIN);                    // Init pin LED2
    gpio_set_dir(LED2_PIN, GPIO_OUT);       // set LED2 output
    

    i2c_init(I2C_PORT, 400 * 1000);                         // Initialize I2C at 400kHz
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    i2c_scan();                                             // Scan I2C bus for devices

    z_accel_queue = xQueueCreate(32, sizeof(int16_t));      // Create a queue to hold accelerometer data

    xTaskCreate(task_read_accel, "ReadAccel", 256, NULL, 1, NULL);
    xTaskCreate(task_detect_tap, "DetectTap", 256, NULL, 1, NULL);

    vTaskStartScheduler();

    //while (1);  
    return 0;
}
