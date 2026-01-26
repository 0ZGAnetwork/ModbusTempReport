#include "driver_control.h"
#include "driver.h"
#include "sdc35_csv.h"
#include "sdc35_status.h"

#include "pico/stdio.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

int main(void) {
    stdio_init_all();        // USB serial
    uart_init_max485();
    modbus_flush_rx();

    SDC35Status status;

    const uint GPIO2 = 2;    // dioda na Pico 2W
    gpio_init(GPIO2);
    gpio_set_dir(GPIO2, GPIO_OUT);

    // krótkie zapalenie diody przy starcie
    gpio_put(GPIO2, 1);
    sleep_ms(200);
    gpio_put(GPIO2, 0);

    char cmd[16];
    int i = 0;

    // poczekaj na połączenie USB
    while (!stdio_usb_connected()) {
        tight_loop_contents();
    }

    printf("Connected. Commands available: report1 / report2 / report3 / exit\n");
    printf("Waiting for command...\n");

    while (1) {
        int c = getchar_timeout_us(100);
        if (c == PICO_ERROR_TIMEOUT) continue;

        // ignoruj znaki kontrolne w środku komendy
        if (c == '\r') continue;

        if (c == '\n') {  // koniec komendy
            if (i == 0) continue;  // pusta linia
            cmd[i] = '\0';
            i = 0;

            if (strcmp(cmd, "report1") == 0) {
                create_snapshot(&status);
                show_Snapshot_uart(&status);
                csv_print_row(&status);

                gpio_put(GPIO2, 1);
                sleep_ms(200);
                gpio_put(GPIO2, 0);

                printf("report1 done\n");
            }
            else if (strcmp(cmd, "report2") == 0) {
                uint32_t duration_ms = 60000;
                uint32_t last_time = to_ms_since_boot(get_absolute_time());
                uint32_t start_time = last_time;

                while (to_ms_since_boot(get_absolute_time()) - start_time < duration_ms) {
                    uint32_t now = to_ms_since_boot(get_absolute_time());
                    if (now - last_time >= 5000) { 
                        last_time = now;

                        create_snapshot(&status);
                        show_Snapshot_uart(&status);
                        csv_print_row(&status);

                        gpio_put(GPIO2, 1);
                        sleep_ms(200);
                        gpio_put(GPIO2, 0);
                    }

                    // odczyt exit w trakcie report2
                    int c2 = getchar_timeout_us(100);
                    if (c2 != PICO_ERROR_TIMEOUT) {
                        if (c2 == '\n') {
                            cmd[i] = '\0';
                            i = 0;
                            if (strcmp(cmd, "exit") == 0) {
                                printf("Exiting program.\n");
                                return 0;
                            }
                        } else if (i < sizeof(cmd)-1 && c2 >= 32 && c2 <= 126) {  // tylko drukowalne znaki
                            cmd[i++] = c2;
                        }
                    }
                }

                printf("report2 done\n");
            }
            else if (strcmp(cmd, "report3") == 0) {
                printf("test mode 3\n");
                gpio_put(GPIO2, 1);
                sleep_ms(200);
                gpio_put(GPIO2, 0);
            }
            else if (strcmp(cmd, "exit") == 0) {
                printf("Exiting program.\n");
                sleep_ms(100);
                return 0;
            }
            else {
                printf("Unknown command: %s\n", cmd);
            }

            printf("Waiting for command...\n");
        }
        else if (i < sizeof(cmd)-1 && c >= 32 && c <= 126) {  // tylko drukowalne
            cmd[i++] = c;
        }
    }
}