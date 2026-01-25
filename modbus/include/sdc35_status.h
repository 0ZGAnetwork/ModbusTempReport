#ifndef SDC35_STATUS_H
#define SDC35_STATUS_H

#include <stdbool.h>
#include <stdint.h>


typedef struct {
    // some register are not displayed depending on the 
    // availibity of optional function, model number, display 
    // setup (c73 to c78) ad user level (c79)
    // --- Process values ---
    float pv;              // PV real-time value
    float sv;              // SV real-time value
    uint16_t lsp;          // LSP group selection
    uint16_t pid_group;    // PID group being selected

    // --- Operation Display limits (example: snapshot min/max) ---
    float pv_lo_max;
    float pv_hi_max;
    float sv_lo_max;
    float sv_hi_max;

    // --- Setup / Input configuration ---
    uint16_t pv_input_type;     // 1451
    float pv_lo_limit;          // 1455
    float pv_hi_limit;          // 1456
    float sv_lo_limit;          // 1457
    float sv_hi_limit;          // 1458
    uint16_t rsp_input_type;    // 145A
    float rsp_lo_limit;         // 145B
    float rsp_hi_limit;         // 145C
    uint16_t decimal_point;     // 1467

    // --- Control / Output configuration ---
    uint16_t control_action;                 // 145E
    uint16_t output_at_pv_alarm;            // 1460
    uint16_t output_operation_at_pv_alarm;  // 145F
    uint16_t heat_cool_control;             // 146A
    uint16_t lsp_system_group;              // 146E
    uint16_t preset_manual_value;           // manual power 1464
    uint16_t pid_output_mode;               // initial output type/mode 1465
    uint16_t zone_pid_operation;            // 1468

    // --- Auxiliary outputs ---
    uint16_t aux_output_range;              // 1484
    uint16_t aux_type;                      // 1485
    float aux_scale_low;                    // 1486
    float aux_scale_high;                   // 1487
    float aux_mv_scale;                     // 1488
    uint16_t position_proportional_type;    // 1489
    uint16_t motor_auto_adjust;             // 148C

    // --- Communication / Modbus ---
    uint16_t comm_type;      // 1490
    uint16_t station_addr;   // 1491
    uint16_t tx_speed;       // 1492
    uint16_t data_length;    // 1493
    uint16_t parity;         // 1494
    uint16_t stop_bits;      // 1495
    uint16_t user_level;     // 149F

    // --- Parameter bank ---
    uint16_t control_method; // 1771
    float differential;      // 1774
    uint16_t bank_type;      // 2135

    // --- Alarms ---
    uint16_t alarm_typical;  // 3800
    uint16_t alarm_D0;       // 3801
    uint16_t alarm_D1;       // 3802
    uint16_t alarms_status;  

    bool alarm_pv_over;      // bit 
    bool alarm_pv_under;     // bit 
    bool alarm_cj_burnout;   // bit 
    bool alarm_rsp_over;     
    bool alarm_mfb_burnout;
    bool alarm_motor_fail;
    bool alarm_ct_over;

    bool alarm_pv_failure;         // bit 0
                                   // undefined bits 1 to 11
    bool alarm_hardware_failure;   // bit 12
    bool alarm_parameter_failure;  // bit 13
    bool alarm_adjustment_failure; // bit 14
    bool alarm_rom_failure;        // bit 15

    // --- Timestamp ---
    char timestamp[32];
} SDC35Status;


#endif