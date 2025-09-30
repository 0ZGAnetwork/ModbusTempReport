#ifndef SDC35_STATUS_H
#define SDC35_STATUS_H

typedef struct {
    float pv_lo_max;
    float pv_hi_max;
    float sv_lo_max;
    float sv_hi_max;

    int config;
    int alarm;

    uint16_t alarms_status;
    bool alarm_pv_over;
    bool alarm_pv_under;
    bool alarm_cj_burnout;
    bool alarm_rsp_over;
    bool alarm_mfb_burnout;
    bool alarm_motor_fail;
    bool alarm_ct_over;

    char timestamp[32];
} SDC35Status;

#endif