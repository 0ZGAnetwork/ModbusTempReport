#include "driver_control.h"
#include "driver.h"
#include <stdio.h>
#include "sdc35_status.h"
#include "pico/stdlib.h"
#include <string.h>
#include <math.h>

#define SLAVE_ADDR 2

// ---helper functions to print values with N/A handling---
static void print_u16(const char *label, uint16_t v)
{
    if (v == 0xFFFF)
        printf("%-30s : N/A\n", label);
    else
        printf("%-30s : %u\n", label, v);
}
static void print_u8(const char *label, uint8_t v)
{
    if (v == 0xFF)
        printf("%-30s : N/A\n", label);
    else
        printf("%-30s : %u\n", label, v);
}
static void print_float1(const char *label, float v)
{
    if (isnan(v))
        printf("%-30s : N/A\n", label);
    else
        printf("%-30s : %.1f\n", label, v);
}
// ---print frame in hex and decimal format for debugging---
void print_frame_hex(const uint8_t *frame, int len, const char *label) {
    printf("%s: ", label);
    for (int i = 0; i < len; i++) {
        printf("%02X ", frame[i]);
    }
    printf("\n");
}
void print_bits(uint16_t value, const char *label) {
    printf("%s: 0x%04X | bits: ", label, value);
    for (int i = 15; i >= 0; i--) {
        printf("%d", (value >> i) & 1);
        if (i % 4 == 0) printf(" ");
    }
    printf("\n");
}

uint32_t millis() {
    return to_ms_since_boot(get_absolute_time());
}

void modbus_flush_rx(void) {
    while (uart_is_readable(UART_PORT)) {
        uart_getc(UART_PORT);
    }
}
// ---read fuction code 0x03 registers---
void modbus_read(uint16_t start, uint16_t count) {
    uint8_t frame[8];

    frame[0] = SLAVE_ADDR;
    frame[1] = 0x03;
    frame[2] = start >> 8;
    frame[3] = start & 0xFF;
    frame[4] = count >> 8;
    frame[5] = count & 0xFF;

    uint16_t crc = crc16_modbus(frame, 6);
    frame[6] = crc & 0xFF;
    frame[7] = crc >> 8;

    modbus_flush_rx();
    //print_frame_hex(frame, 8, "TX");
    uart_send(frame, 8);

}

int modbus_receive(uint8_t *buf, int expected_len, int timeout_ms) {
    int pos = 0;
    uint32_t start = millis();
    modbus_flush_rx();
    while ((millis() - start) < timeout_ms) {
        if (uart_is_readable(UART_PORT)) {
            buf[pos++] = uart_getc(UART_PORT);
            if (pos >= expected_len)
                return pos;
        }
    }
    return -1; // timeout
}

int modbus_read_regs(uint16_t start,
                     uint16_t count,
                     uint16_t *out)
{
    uint8_t resp[256];
    int expected = 5 + 2 * count;
    modbus_read(start, count); // frame send
    int timeout_ms = 50 + count * 15; 
    int len = modbus_receive(resp, expected, timeout_ms);
    if (len != expected)
        return -1; // timeout / niepełna ramka
    //print_frame_hex(resp, len, "RX");
    if (!crc16_check(resp, len))
        return -2;
    if (resp[1] & 0x80)
        return -3; // exception
    for (int i = 0; i < count; i++) {
        out[i] = (resp[3 + i*2] << 8) | resp[4 + i*2];
    }
    return 0;
}

void test_modbus_slave() { // debugging for one register, required raw frame
    unsigned char frame[8] = {SLAVE_ADDR, 0x03, 0x14, 0x51, 0x00, 0x01};
    unsigned short crc = crc16_modbus(frame, 6);
    frame[6] = crc & 0xFF;
    frame[7] = (crc >> 8) & 0xFF;

    uart_send(frame, 8);

    int count = 0;
    printf("Response: ");
    while(uart_is_readable(UART_PORT) && count < 50) {
        unsigned char b = uart_getc(UART_PORT);
        printf("%02X ", b);
        count++;
    }
    printf(" (%d bytes received)\n", count);
}

void read_operation_display(SDC35Status *status) {
    uint16_t regs[4]; 

    if (modbus_read_regs(0x238D, 4, regs) != 0) {
        //printf("not available (model / user level) reading operation display registers!\n");
        status->pv = -1;
        status->sv = -1;
        status->lsp = -1;
        status->pid_group = -1;
        return;
    }

    status->pv = regs[0] / 10.0; // X REAL: X.X
    status->sv = regs[1] / 10.0;
    status->lsp = regs[2];
    status->pid_group = regs[3];
}
// --- Read functions for various register groups ---
// some register are not displayed depending on the user level or model
void read_pv_sv_limits(SDC35Status *status) {
    modbus_flush_rx();
    uint16_t regs_limits[4];
    if (modbus_read_regs(0x1455, 4, regs_limits) != 0) {//0x1455
        printf("not available (model / user level) reading PV/SV limits (1455-1458)!\n");
        status->pv_lo_limit = -1;
        status->pv_hi_limit = -1;
        status->sv_lo_limit = -1;
        status->sv_hi_limit = -1;
        return;
    }
     //else {
        status->pv_lo_limit = regs_limits[0];
        status->pv_hi_limit = regs_limits[1];
        status->sv_lo_limit = regs_limits[2];
        status->sv_hi_limit = regs_limits[3];
}

void read_setup(SDC35Status *status) {
    uint16_t reg;
    // --- PV input type (1451) ---
    if (modbus_read_regs(0x1451, 1, &reg) != 0) {
        //printf("not available (model / user level)reading PV input type (1451)!\n");
        status->pv_input_type = 0xFFFF;
    } else {
        status->pv_input_type = reg;
    }
    // --- PV and SV limits (1455-1458) ---
    // uint16_t regs_limits[4];
    // if (modbus_read_regs(0x1455, 4, regs_limits) != 0) {
    //     printf("not available (model / user level) reading PV/SV limits (1455-1458)!\n");
    //     status->pv_lo_limit = -1;
    //     status->pv_hi_limit = -1;
    //     status->sv_lo_limit = -1;
    //     status->sv_hi_limit = -1;
    //     return;
    // }
    //  //else {
    //     status->pv_lo_limit = regs_limits[0];
    //     status->pv_hi_limit = regs_limits[1];
    //     status->sv_lo_limit = regs_limits[2];
    //     status->sv_hi_limit = regs_limits[3];

    // --- RSP input type and limits (145A-145C) ---
    // uint16_t regs_rsp[3];
    // if (modbus_read_regs(0x145A, 3, regs_rsp) != 0) {
    //     printf("not available (model / user level) reading RSP input type and limits (145A-145C)!\n");
    //     status->rsp_input_type = 0xFFFF;
    //     status->rsp_lo_limit = -1;
    //     status->rsp_hi_limit = -1;
    // } else {
    //     status->rsp_input_type = regs_rsp[0];
    //     status->rsp_lo_limit   = regs_rsp[1];
    //     status->rsp_hi_limit   = regs_rsp[2];
    // }
    // // --- Decimal point (1467) ---
    // if (modbus_read_regs(0x1467, 1, &reg) != 0) {
    //     printf("not available (model / user level) reading decimal point (1467)!\n");
    //     status->decimal_point = -1;
    // } else {
    //     status->decimal_point = reg;
    // }
}

void read_setup_control(SDC35Status *status) {
    uint16_t regs[7]; // first block: 145E-1465
    if (modbus_read_regs(0x145E, 7, regs) != 0) {
        //printf("not available (model / user level) reading control setup registers block 1!\n");
        status->control_action = 0xFFFF;
        status->output_at_pv_alarm = 0xFFFF;
        status->output_operation_at_pv_alarm = 0xFFFF;
        status->heat_cool_control = 0xFFFF;
        status->lsp_system_group = 0xFFFF;
        status->preset_manual_value = 0xFFFF;
        status->pid_output_mode = 0xFFFF;
    } else {
        status->control_action = regs[0];
        status->output_at_pv_alarm = regs[1];
        status->output_operation_at_pv_alarm = regs[2];
        status->heat_cool_control = regs[3];
        status->lsp_system_group = regs[4];
        status->preset_manual_value = regs[5];
        status->pid_output_mode = regs[6];
    }
    // second block: 1468
    if (modbus_read_regs(0x1468, 1, regs) != 0) {
        //printf("not available (model / user level) reading zone PID operation!\n");
        status->zone_pid_operation = 0xFFFF;
    } else {
        status->zone_pid_operation = regs[0];
    }
}

void read_alarms(SDC35Status *status) {
    uint16_t regs[3]; // 3800, 3801, 3802
    if (modbus_read_regs(0x3800, 3, regs) != 0) {
        //printf("Reading alarm registers!\n");
        // --- debug ---
        status->alarm_typical = 0xFFFF;
        status->alarm_D0      = 0xFFFF;
        status->alarm_D1      = 0xFFFF;
        return;
    }
    // print_bits(regs[0], "alarm_typical");
    // print_bits(regs[1], "alarm_D0");
    // print_bits(regs[2], "alarm_D1");
    status->alarm_typical = regs[0];
    status->alarm_D0      = regs[1];
    status->alarm_D1      = regs[2];
    // --- Typical alarm (3800) ---
    status->alarm_pv_failure         = (regs[0] & (1 << 0))  ? 1 : 0;
    status->alarm_hardware_failure   = (regs[0] & (1 << 12)) ? 1 : 0;
    status->alarm_parameter_failure  = (regs[0] & (1 << 13)) ? 1 : 0;
    status->alarm_adjustment_failure = (regs[0] & (1 << 14)) ? 1 : 0;
    status->alarm_rom_failure        = (regs[0] & (1 << 15)) ? 1 : 0;
    // --- DO status (3801) ---
    status->alarm_pv_over      = (regs[1] & (1 << 0)) ? 1 : 0;
    status->alarm_pv_under     = (regs[1] & (1 << 1)) ? 1 : 0;
    status->alarm_cj_burnout   = (regs[1] & (1 << 2)) ? 1 : 0;
    status->alarm_rsp_over     = (regs[1] & (1 << 3)) ? 1 : 0;
    status->alarm_mfb_burnout  = (regs[1] & (1 << 4)) ? 1 : 0;
    status->alarm_motor_fail   = (regs[1] & (1 << 5)) ? 1 : 0;
    status->alarm_ct_over      = (regs[1] & (1 << 6)) ? 1 : 0;
    // --- DI status (3802) ---
    // status->some_DI_alarm = (regs[2] & (1 << 0)) ? 1 : 0; // przykład dodatkowego alarmu
}

void read_config(SDC35Status *status) {
    uint16_t reg;
    // --- Control action (145E) ---
    if (modbus_read_regs(0x145E, 1, &reg) != 0) {
        //printf("not available (model / user level) reading control action (145E)!\n");
        status->control_action = 0xFFFF;
    } else {
        status->control_action = reg;
    }
    // --- Output operation at PV alarm (145F) ---
    if (modbus_read_regs(0x145F, 1, &reg) != 0) {
        //printf("not available (model / user level) reading output operation at PV alarm (145F)!\n");
        status->output_operation_at_pv_alarm = 0xFFFF;
    } else {
        status->output_operation_at_pv_alarm = reg;
    }
    // --- Output at PV alarm (1460) ---
    if (modbus_read_regs(0x1460, 1, &reg) != 0) {
        //printf("not available (model / user level) reading output at PV alarm (1460)!\n");
        status->output_at_pv_alarm = 0xFFFF;
    } else {
        status->output_at_pv_alarm = reg;
    }
    // --- Heat/Cool control (146A) ---
    if (modbus_read_regs(0x146A, 1, &reg) != 0) {
        //printf("not available (model / user level) reading heat/cool control (146A)!\n");
        status->heat_cool_control = 0xFFFF;
    } else {
        status->heat_cool_control = reg;
    }
    // --- LSP system group (146E) ---
    if (modbus_read_regs(0x146E, 1, &reg) != 0) {
        //printf("not available (model / user level) reading LSP system group (146E)!\n");
        status->lsp_system_group = 0xFFFF;
    } else {
        status->lsp_system_group = reg;
    }
    // --- Preset manual value (1464) ---
    if (modbus_read_regs(0x1464, 1, &reg) != 0) {
        //printf("not available (model / user level) reading preset manual value (1464)!\n");
        status->preset_manual_value = 0xFFFF;
    } else {
        status->preset_manual_value = reg;
    }
    // --- PID output mode (1465) ---
    if (modbus_read_regs(0x1465, 1, &reg) != 0) {
        //printf("not available (model / user level) reading PID output mode (1465)!\n");
        status->pid_output_mode = 0xFFFF;
    } else {
        status->pid_output_mode = reg;
    }
    // --- Zone PID operation (1468) ---
    if (modbus_read_regs(0x1468, 1, &reg) != 0) {
        //printf("not available (model / user level) reading zone PID operation (1468)!\n");
        status->zone_pid_operation = 0xFFFF;
    } else {
        status->zone_pid_operation = reg;
    }
}

void read_parameter_bank(SDC35Status *status) {
    uint16_t regs[2];
    uint16_t reg;
    // --- Control method (1771) ---
    if (modbus_read_regs(0x1771, 1, &reg) != 0) {
        //printf("not available (model / user level) reading control method (1771)!\n");
        status->control_method = 0xFFFF;
    } else {
        status->control_method = reg;
    }
    // --- Differential (1774) ---
    if (modbus_read_regs(0x1774, 2, regs) != 0) {
        //printf("not available (model / user level) reading differential (1774)!\n");
        status->differential = -1;
    } else {
        // Połączenie dwóch rejestrów w float
        uint32_t tmp = ((uint32_t)regs[0] << 16) | regs[1];
        status->differential = *(float*)&tmp;
    }
    // --- Bank type (2135) ---
    if (modbus_read_regs(0x2135, 1, &reg) != 0) {
        //printf("not available (model / user level) reading bank type (2135)!\n");
        status->bank_type = 0xFFFF;
    } else {
        status->bank_type = reg;
    }
}

void read_communication_modbus(SDC35Status *status) {
    uint16_t regs[6]; // 6 rejestrów od 1490 do 1495

    if (modbus_read_regs(0x1490, 6, regs) != 0) {
        //printf("not available (model / user level) reading Modbus communication registers!\n");
        status->comm_type      = 0xFFFF;
        status->station_addr   = 0xFFFF;
        status->tx_speed       = 0xFFFF;
        status->data_length    = 0xFFFF;
        status->parity         = 0xFFFF;
        status->stop_bits      = 0xFFFF;
        return;
    }
    status->comm_type    = regs[0]; // 1490
    status->station_addr = regs[1]; // 1491
    status->tx_speed     = regs[2]; // 1492
    status->data_length  = regs[3]; // 1493
    status->parity       = regs[4]; // 1494
    status->stop_bits    = regs[5]; // 1495
}

void read_aux_outputs(SDC35Status *status) {
    uint16_t regs[2]; // dla odczytu 2-rejestrowego float
    uint16_t reg;

    // --- Aux output range (1484) ---
    if (modbus_read_regs(0x1484, 1, &reg) != 0) {
        //printf("not available (model / user level) reading aux output range (1484)!\n");
        status->aux_output_range = 0xFFFF;
    } else {
        status->aux_output_range = reg;
    }
    // --- Aux type (1485) ---
    if (modbus_read_regs(0x1485, 1, &reg) != 0) {
        //printf("not available (model / user level) reading aux type (1485)!\n");
         status->aux_type = 0xFFFF;
    } else {
        status->aux_type = reg;
    }
    // --- Aux scale low (1486) ---
    if (modbus_read_regs(0x1486, 2, regs) != 0) {
        //printf("not available (model / user level) reading aux scale low (1486)!\n");
        status->aux_scale_low = -1;
    } else {
        // połączenie dwóch rejestrów w float
        uint32_t tmp = ((uint32_t)regs[0] << 16) | regs[1];
        status->aux_scale_low = *(float*)&tmp;
    }
    // --- Aux scale high (1487) ---
    if (modbus_read_regs(0x1487, 2, regs) != 0) {
        printf("not available (model / user level) reading aux scale high (1487)!\n");
        status->aux_scale_high = -1;
    } else {
        uint32_t tmp = ((uint32_t)regs[0] << 16) | regs[1];
        status->aux_scale_high = *(float*)&tmp;
    }
    // --- Aux mv scale (1488) ---
    if (modbus_read_regs(0x1488, 2, regs) != 0) {
        //printf("not available (model / user level) reading aux mv scale (1488)!\n");
        status->aux_mv_scale = -1;
    } else {
        uint32_t tmp = ((uint32_t)regs[0] << 16) | regs[1];
        status->aux_mv_scale = *(float*)&tmp;
    }
    // --- Position proportional type (1489) ---
    if (modbus_read_regs(0x1489, 1, &reg) != 0) {
        //printf("not available (model / user level) reading position proportional type (1489)!\n");
        status->position_proportional_type = 0xFFFF;
    } else {
        status->position_proportional_type = reg;
    }
    // --- Motor auto adjust (148C) ---
    if (modbus_read_regs(0x148C, 1, &reg) != 0) {
        //printf("not available (model / user level) reading motor auto adjust (148C)!\n");
        status->motor_auto_adjust = 0xFFFF;
    } else {
        status->motor_auto_adjust = reg;
    }
}

void create_snapshot(SDC35Status *status) {
    memset(status, 0xFF, sizeof(*status));
    status->pv              = NAN;
    status->sv              = NAN;
    status->pv_lo_limit     = NAN;
    status->pv_hi_limit     = NAN;
    status->sv_lo_limit     = NAN;
    status->sv_hi_limit     = NAN;
    status->rsp_lo_limit    = NAN;
    status->rsp_hi_limit    = NAN;
    status->aux_scale_low   = NAN;
    status->aux_scale_high  = NAN;
    status->aux_mv_scale    = NAN;
    status->differential    = NAN;
    // string
    memset(status->timestamp, 0, sizeof(status->timestamp));

    read_setup(status);
    read_pv_sv_limits(status);
    read_operation_display(status);
    
        //read_setup_control(status);    
    //read_alarms(status);      
    read_config(status);
    read_parameter_bank(status);
    read_communication_modbus(status);
    read_aux_outputs(status);
    format_timestamp(status->timestamp, sizeof(status->timestamp));

}

void show_Snapshot_uart(const SDC35Status *status)
{
    printf("===== SDC35 Snapshot =====\n");

    // --- Operation display ---
    print_float1("PV", status->pv);
    print_float1("SV", status->sv);
    print_u16("LSP", status->lsp);
    print_u16("PID Group", status->pid_group);

    // --- Operation Display limits ---
    print_float1("PV Lo Max", status->pv_lo_max);
    print_float1("PV Hi Max", status->pv_hi_max);
    print_float1("SV Lo Max", status->sv_lo_max);
    print_float1("SV Hi Max", status->sv_hi_max);

    // --- Setup / Input configuration ---
    print_u16("PV Input Type", status->pv_input_type);
    print_float1("PV Range Low", status->pv_lo_limit);
    print_float1("PV Range High", status->pv_hi_limit);
    print_float1("SV Range Low", status->sv_lo_limit);
    print_float1("SV Range High", status->sv_hi_limit);
    print_u16("RSP Input Type", status->rsp_input_type);
    print_float1("RSP Range Low", status->rsp_lo_limit);
    print_float1("RSP Range High", status->rsp_hi_limit);
    print_u16("Decimal Point", status->decimal_point);

    // --- Control / Output configuration ---
    print_u16("Control Action", status->control_action);
    print_u16("Output at PV Alarm", status->output_at_pv_alarm);
    print_u16("Output Operation at PV Alarm", status->output_operation_at_pv_alarm);
    print_u16("Heat/Cool Control", status->heat_cool_control);
    print_u16("LSP System Group", status->lsp_system_group);
    print_u16("Preset Manual Value", status->preset_manual_value);
    print_u16("PID Output Mode", status->pid_output_mode);
    print_u16("Zone PID Operation", status->zone_pid_operation);

    // --- Auxiliary outputs ---
    print_u16("Aux Output Range", status->aux_output_range);
    print_u16("Aux Type", status->aux_type);
    print_float1("Aux Scale Low", status->aux_scale_low);
    print_float1("Aux Scale High", status->aux_scale_high);
    print_float1("Aux MV Scale", status->aux_mv_scale);
    print_u16("Position Proportional Type", status->position_proportional_type);
    print_u16("Motor Auto Adjust", status->motor_auto_adjust);

    // --- Communication / Modbus ---
    print_u16("Comm Type", status->comm_type);
    print_u16("Station Address", status->station_addr);
    print_u16("Transmission Speed", status->tx_speed);
    print_u16("Data Length", status->data_length);
    print_u16("Parity", status->parity);
    print_u16("Stop Bits", status->stop_bits);
    print_u16("User Level", status->user_level);

    // --- Parameter bank ---
    print_u16("Control Method", status->control_method);
    print_float1("Differential", status->differential);
    print_u16("Bank Type", status->bank_type);

    // --- Alarms ---
    printf("Alarms:\n");
    print_u16("Alarm Typical", status->alarm_typical);
    print_u16("Alarm D0", status->alarm_D0);
    print_u16("Alarm D1", status->alarm_D1);
    print_u16("Alarm Status", status->alarms_status);

    print_u8("PV Over", status->alarm_pv_over);
    print_u8("PV Under", status->alarm_pv_under);
    print_u8("CJ Burnout", status->alarm_cj_burnout);
    print_u8("RSP Over", status->alarm_rsp_over);
    print_u8("MFB Burnout", status->alarm_mfb_burnout);
    print_u8("Motor Fail", status->alarm_motor_fail);
    print_u8("CT Over", status->alarm_ct_over);

    print_u8("PV Failure", status->alarm_pv_failure);
    print_u8("Hardware Failure", status->alarm_hardware_failure);
    print_u8("Parameter Failure", status->alarm_parameter_failure);
    print_u8("Adjustment Failure", status->alarm_adjustment_failure);
    print_u8("ROM Failure", status->alarm_rom_failure);

    // --- Timestamp ---
    printf("Timestamp: %s\n", status->timestamp);
    printf("===========================\n");
}

void format_timestamp(char *buf, int buf_size){
    snprintf(buf, buf_size, "Status: Test Snapshot ");
}