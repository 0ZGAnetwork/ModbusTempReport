#include <stdio.h>
#include "pico/stdlib.h"
int main() {
    const uint GPIO2 = 4; //PICO_DEFAULT_LED_PIN;
    gpio_init(GPIO2);
    gpio_set_dir(GPIO2, GPIO_OUT);
    stdio_init_all();
    while (true) {
        gpio_put(GPIO2, 1);
        sleep_ms(250);
        printf("Blinking\r\n");
        gpio_put(GPIO2, 0);
        sleep_ms(250);
        printf("OFF\r\n");
    }
}