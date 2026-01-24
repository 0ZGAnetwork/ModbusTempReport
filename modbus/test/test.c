#include <stdio.h>
#include "pico/stdlib.h"

int main() {
    stdio_init_all();

    // Wbudowana dioda LED
    const uint GPIO2 = 2;
    gpio_init(GPIO2);
    gpio_set_dir(GPIO2, GPIO_OUT);

    while (true) {
        gpio_put(GPIO2, 1);
        printf("LED right now ON\n");
        sleep_ms(2000);

        gpio_put(GPIO2, 0);
        printf("LED right now OFF\n");
        sleep_ms(500);
    }
}
