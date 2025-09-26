#include "driver_control.h"
#include "driver.h"
#include <stdio.h>

#define SLAVE_ADDR 1

void create_snapshot(SDC35Status *status){
    unsigned short regs[3];
    int start_addr = 0x1455;
    int num_regs = 3;

    int result = read_modbus_registers(SLAVE_ADDR, start_addr, num_regs, regs);

    if (result == 0) {
        status->pv = (float)regs[0] / 10.0f;
        status->sv = (float)regs[1] / 10.0f;
        status->alarm = regs[2];
    } else {
        status->pv = -1;
        status->sv = -1;
        status->alarm = -1;
    }
    format_timestamp(status->timestamp, sizeof(status->timestamp));
}


void save_Snapshot_csv(const SDC35Status *status){
    FILE *f = fopen("snapshot.csv", "w");
    if (f) {
        fprintf(f, "Timestamp,PV,SV,Alarm\n");
        fprintf(f, "%s,%.1f,%.1f,%d\n",
                status->timestamp, status->pv, status->sv, status->alarm);
        fclose(f);
    }
}

void format_timestamp(char *buf, int buf_size){
    snprintf(buf, buf_size, "2025-09-26 15:00:00");
    }

