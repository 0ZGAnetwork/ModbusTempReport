#ifndef DRIVER_CONTROL_H
#define DRIVER_CONTROL_H

#include "sdc35_status.h"

void create_snapshot(SDC35Status *status);
//void save_Snapshot_csv(const SDC35Status *status);
void show_Snapshot_uart(const SDC35Status *status);
void format_timestamp(char *buf, int buf_size);
void test_modbus_slave(void);

static void print_u16(const char *label, uint16_t v);
static void print_u8(const char *label, uint8_t v);
static void print_float1(const char *label, float v);
void print_bits(uint16_t value, const char *label);
void print_frame_hex(const uint8_t *frame, int len, const char *label);
uint32_t millis();
void modbus_flush_rx(void);
void create_snapshot(SDC35Status *status);
void read_operation_display(SDC35Status *status);
void read_setup(SDC35Status *status);
void read_setup_control(SDC35Status *status);
//int modbus_read_raw_regs(uart_inst_t *uart, uint16_t start_reg, int count, uint16_t *regs);
void read_alarms(SDC35Status *status);      
void read_config(SDC35Status *status);
void read_parameter_bank(SDC35Status *status);
void read_communication_modbus(SDC35Status *status);
void read_aux_outputs(SDC35Status *status);

void modbus_read(uint16_t start, uint16_t count);
int modbus_receive(uint8_t *buf, int expected_len, int timeout_ms);
int modbus_read_regs(uint16_t start, uint16_t count, uint16_t *out);

#endif // DRIVER_CONTROL_H      
    