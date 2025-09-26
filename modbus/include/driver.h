#ifdef DRIVER_H
#define DRIVER_H

#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"

// --- UART/MAX485-SDC35 ---
#define UART_PORT uart0
#define UART_BAUD 9600
#define UART_TX_PIN 0
#define UART_RX_PIN 1
#define MAX485_DE_RE_PIN 22

// --- UART/modbus ---
int read_modbus_register(int slave_addr, int reg_addr, int *value);
void uart_send(const unsigned char *data, int len);
int uart_receive(unsigned char *buf, int max_len);
unsigned short crc16_modbus(const unsigned char *buf, int len);


#endif