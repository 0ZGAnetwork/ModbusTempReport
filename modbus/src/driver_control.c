#include "driver_control.h"
#include "driver.h"
#include "stdio.h"

#define SLAVE_ADDR 1

void create_snapshot(SDC35Status *status) {
    int value;

    format_timestamp(status->timestamp, sizeof(status->timestamp));

    //read PV placeholder RAM Address)
    if (read_modbus_register(SLAVE_ADDR, 0x1455, &value) == 0) { //PV Input Range Low Limit
        status->pv = (float)value/10.0f; //scaling
    } else {
        status->pv = -1;
    }

    if (read_modbus_register(SLAVE_ADDR, 0x1457, &value) == 0) { //SV Input Range Low Limit
        status->sv = (float)value/10.0f; //scaling
    } else {
        status->sv = -1;
    }

    if (read_modbus_register(SLAVE_ADDR, 0x23F1, &value) == 0) { //Alarm status
        status->alarm = value; //scaling
    } else {
        status->alarm = -1;
    }
    
}
