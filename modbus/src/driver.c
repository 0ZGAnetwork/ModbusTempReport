#include "driver.h"

int read_modbus_register(int slave_addr, int reg_addr, int *value) {
    // Implementation of reading a Modbus register
    unsigned char frame[8];
    unsigned char response[16];
    int len;

    frame[0] = (unsigned char)slave_addr;
    frame[1] = 0x03;               // Read Holding Register
    frame[2] = (reg_addr >> 8) & 0xFF;
    frame[3] = reg_addr & 0xFF;
    frame[4] = 0x00;               // HIGH byte of number of registers to read
    frame[5] = 0x01;

    unsigned short crc = crc16_modbus(frame, 6);
    frame[6] = crc & 0xFF;
    frame[7] = (crc >> 8) & 0xFF;

    uart_send(frame, 8);
    len = uart_receive(response, sizeof(response));
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
}

int uart_receive(unsigned char *buf, int max_len) {
    // Implementation of UART receive
    return 0;
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