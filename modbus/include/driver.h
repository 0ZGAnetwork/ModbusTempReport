#ifndef DRIVER_H
#define DRIVER_H

#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"

// --- UART/MAX485-SDC35 ---
#define UART_PORT uart0
#define UART_BAUD 9600
#define UART_TX_PIN 0
#define UART_RX_PIN 1
#define MAX485_DE_RE_PIN 22

// --- UART/modbus ---
int read_modbus_register(int slave_addr, int reg_addr, int num_regs,  int *values);
void uart_send(const unsigned char *data, int len);
int uart_received_timeout(unsigned char *buf, int expected_len,unsigned int timeout_us);
unsigned short crc16_modbus(const unsigned char *buf, int len);

#endif