#include "sdc35_status.h"
#include "driver_control.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "driver.h"

int main (void){
    stdio_init_all();
    uart_init_max485();
    modbus_flush_rx();

    SDC35Status status;

    const uint GPIO2 = 2;
    gpio_init(GPIO2);
    gpio_set_dir(GPIO2, GPIO_OUT);

    while(1) {
        //test_modbus_slave();
        create_snapshot(&status); // SCADA / DCS data capture
        show_Snapshot_uart(&status);

        gpio_put(GPIO2, 1);
        sleep_ms(2000);
        gpio_put(GPIO2, 0);
        sleep_ms(200);
    }

    return 0;
}