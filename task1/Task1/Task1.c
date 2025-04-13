#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/uart.h"

#define UART_ID uart0
#define LED_PIN CYW43WL_GIPO_LED_PIN

int main()
{
    stdio_init_all();
    cyw43_arch_init();
    uart_init(UART_ID, 115200);

    int counter = 0;
    printf("Hello world");
    while (true)
    {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, true); // turn the led on
        sleep_ms(500);                                    // wait

        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, false); // turn the led off
        sleep_ms(500);                                     // wait

        uart_putc(UART_ID, 'H');
        uart_puts(UART_ID, "ELLO WORLD\r\n");

        printf("counter value:%d\r\n", counter++);
        sleep_ms(1000);
    }
}