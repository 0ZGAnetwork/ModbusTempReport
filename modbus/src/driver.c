#include "driver.h"
#include "pico/time.h"
#include <string.h>

void uart_init_max485() {
    uart_init(UART_PORT, 9600);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    gpio_init(MAX485_DE_RE_PIN);
    gpio_set_dir(MAX485_DE_RE_PIN, GPIO_OUT);
    gpio_put(MAX485_DE_RE_PIN, 0);
    uart_set_format(UART_PORT, 8, 1, UART_PARITY_NONE);
}

int read_modbus_register(int slave_addr, int reg_addr, int *value) {
    // Implementation of reading a Modbus register
    unsigned char frame[8];
    unsigned char response[16];
    int len;

    frame[0] = (unsigned char)slave_addr;
    frame[1] = 0x03;               // Read Holding Register
    frame[2] = (reg_addr >> 8) & 0xFF;
    frame[3] = reg_addr & 0xFF;
    frame[4] = 0x00;
    frame[5] = 0x01;

    unsigned short crc = crc16_modbus(frame, 6);
    frame[6] = crc & 0xFF;
    frame[7] = (crc >> 8) & 0xFF;

    uart_send(frame, 8);
    len = uart_receive(response, 7);
    if (len < 7) return -1;

    unsigned short crc_resp = crc16_modbus(response, len - 2);
    unsigned short crc_recv = response[len - 2] | (response[len - 1] << 8);
    if (crc_resp != crc_recv) return -2;  // invalid CRC
    if (response[1] != 0x03) return -3;   // error in response

    *value = (response[3] << 8) | response[4];
    return 0;
}

void uart_send(const unsigned char *data, int len) {
    // Implementation of UART send
    gpio_put(MAX485_DE_RE_PIN, 1);
    uart_write_blocking(UART_PORT, data, len);
    gpio_put(MAX485_DE_RE_PIN, 0);
}

int uart_receive(unsigned char *buf, int max_len) {
    // Implementation of UART receive
    return uart_read_blocking(UART_PORT, buf, max_len);
}

unsigned short crc16_modbus(const unsigned char *buf, int len) {
    // Implementation of CRC16 Modbus calculation
    unsigned short crc = 0xFFFF;
    for (int pos = 0; pos < len; pos++) {
        crc ^= (unsigned short)buf[pos];
        for (int i = 0; i < 8; i++) {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xA001;
            else
                crc = crc >> 1;
        }
    }
    return crc;

}