#include "driver_control.h"
#include "driver.h"
#include <stdio.h>
#include "sdc35_status.h"

#define SLAVE_ADDR 1

void create_snapshot(SDC35Status *status){
    int regs[4];
    int start_addr = 0x1455;
    int num_regs = 4;

    int result = read_modbus_registers(SLAVE_ADDR, start_addr, num_regs, regs);

    if (result == 0) {
        status->pv_lo_max = (float)regs[0] / 10.0f;
        status->pv_hi_max = (float)regs[1] / 10.0f;
        status->sv_lo_max = (float)regs[2] / 10.0f;
        status->sv_hi_max = (float)regs[3] / 10.0f;
    } else {
        status->pv_lo_max = -1;
        status->pv_hi_max = -1;
        status->sv_lo_max = -1;
        status->sv_hi_max = -1;
    }
    format_timestamp(status->timestamp, sizeof(status->timestamp));
}

//void save_Snapshot_csv(const SDC35Status *status){
//    FILE *f = fopen("snapshot.csv", "w");
//    if (f) {
//        fprintf(f, "Timestamp,PV,SV,Alarm\n");
//        fprintf(f, "%s,%.1f,%.1f,%d\n",
//                status->timestamp, status->pv, status->sv, status->alarm);
//        fclose(f);
//    }
//}

void save_Snapshot_uart(const SDC35Status *status){
    printf("Timestamp,PV,SV,Alarm\n");
    printf("%s,%.1f,%.1f,%d\n",
              status->timestamp, status->pv, status->sv, status->alarm);
}

void format_timestamp(char *buf, int buf_size){
    snprintf(buf, buf_size, "Status: OK ");
    }