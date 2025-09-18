#include <stdio.h>
#include "pico/stdlib.h"
int main() {
    const uint GPIO2 = 2;
    gpio_init(GPIO2);
    gpio_set_dir(GPIO2, GPIO_OUT);
    stdio_init_all();
    while (true) {
        gpio_put(GPIO2, 1);
        sleep_ms(250);
        printf("Blink\r\n");
        gpio_put(GPIO2, 0);
        sleep_ms(250);
        printf("OFF\r\n");
    }
}
// only works with C11 standard!
