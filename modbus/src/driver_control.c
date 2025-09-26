#include "driver_control.h"
#include "driver.h"
#include "stdio.h"

#define SLAVE_ADDR 1

void create_snapshot(SDC35Status *status) {

unsigned short regs[3];
int start_addr = 0x1455;
int num_regs = 3;

int result = read_modbus_registers(SLAVE_ADDR, start_addr, num_regs, regs);

if (result == 0) {
    status->pv    = (float)regs[0] / 10.0f;
    status->sv    = (float)regs[1] / 10.0f;
    status->alarm = regs[2];
} else {
    status->pv = -1;
    status->sv = -1;
    status->alarm = -1;
}

