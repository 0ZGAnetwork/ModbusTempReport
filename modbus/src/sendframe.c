#include <pico/stdlib.h>
#include <hardware/uart.h>
#include <stdio.h>

// test for modbus project..........
int main() {
    const uint GPIO2 = 2;
    gpio_init(GPIO2);
    gpio_set_dir(GPIO2, GPIO_OUT);
    stdio_init_all();
    while (true) {
        gpio_put(GPIO2, 1);
        sleep_ms(250);
        printf("Modbus file test: Blink\r\n");
        gpio_put(GPIO2, 0);
        sleep_ms(250);
        printf("Modbus file test: OFF\r\n");
    }
}
// only works with C11 standard!
