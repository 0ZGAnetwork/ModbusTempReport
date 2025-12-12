#include "driver_control.h"
#include "driver.h"
#include <stdio.h>
#include "sdc35_status.h"

#define SLAVE_ADDR 1

void create_snapshot(SDC35Status *status) {
    
        int regs[1];
        int start_addr = 0x1455;//0x238D
        int num_regs = 1;

        int result = read_modbus_registers(SLAVE_ADDR, start_addr, num_regs, regs);

        if (result == 0){
            status->pv = (float)regs[0] / 10.0f;
        } else {
            status->pv = -1;
            printf("s");   
        }
    format_timestamp(status->timestamp, sizeof(status->timestamp));
}

// ignore:
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
    printf("pv\n");

    printf("%.1f\n",
           status->pv);
}

void format_timestamp(char *buf, int buf_size){
    snprintf(buf, buf_size, "Status: Test Snapshot ");
    }