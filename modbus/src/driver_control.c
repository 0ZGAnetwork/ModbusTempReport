#include "driver_control.h"
#include "driver.h"
#include <stdio.h>
#include "sdc35_status.h"

#define SLAVE_ADDR 1

void create_snapshot(SDC35Status *status) {
    {
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
    }

    {
        int regs[1];
        int start_addr = 0x23F1;
        int num_regs = 1;

        int result = read_modbus_registers(SLAVE_ADDR, start_addr, num_regs, regs);

        if (result == 0){
            status->alarms_status = (uint16_t)regs[0];

            status->alarm_pv_over     = (status->alarms_status & (1 << 0)) != 0;
            status->alarm_pv_under    = (status->alarms_status & (1 << 1)) != 0;
            status->alarm_cj_burnout  = (status->alarms_status & (1 << 2)) != 0;
            status->alarm_rsp_over    = (status->alarms_status & (1 << 4)) != 0;
            status->alarm_mfb_burnout = (status->alarms_status & (1 << 6)) != 0;
            status->alarm_motor_fail  = (status->alarms_status & (1 << 9)) != 0;
            status->alarm_ct_over     = (status->alarms_status & (1 << 10)) != 0;

        } else {
            status->alarms_status = 0xFFFF;
            status->alarm_pv_over = 0;
            status->alarm_pv_under = 0;
            status->alarm_cj_burnout = 0;
            status->alarm_rsp_over = 0;
            status->alarm_mfb_burnout = 0;
            status->alarm_motor_fail = 0;
            status->alarm_ct_over = 0;
        }
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
    printf("Timestamp,pv_lo_max,pv_hi_max,sv_lo_max,sv_hi_max\n");
    printf("%s,%.1f,%.1f,%.1f,%.1f\n",
              status->timestamp, status->pv_lo_max, status->pv_hi_max, status->sv_lo_max, status->sv_hi_max);
}

void format_timestamp(char *buf, int buf_size){
    snprintf(buf, buf_size, "Status: Test Snapshot ");
    }