#include "driver.h"
#include "pico/time.h"
#include <string.h>
#include <stdio.h>
#define MAX_REGS 16
#define TIMEOUT_US 2000000

void uart_init_max485() {
    uart_init(UART_PORT, 9600);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    //gpio_init(MAX485_DE_RE_PIN);
    //gpio_set_dir(MAX485_DE_RE_PIN, GPIO_OUT);
    //gpio_put(MAX485_DE_RE_PIN, 0);
    //uart_set_format(UART_PORT, 8, 2, UART_PARITY_NONE); //8N2
    uart_set_format(UART_PORT, 8, 1, UART_PARITY_EVEN); // 8E1
}

int read_modbus_registers(int slave_addr, int reg_addr, int num_regs,  int *values) {
    // Implementation of reading a Modbus register
    int total_read = 0;

    while( total_read < num_regs) {
        
        int batch = (num_regs - total_read > MAX_REGS) ? MAX_REGS : (num_regs - total_read);

        unsigned char frame[8];
        frame[0] = (unsigned char)slave_addr;
        frame[1] = 0x03;               // Read Holding Register
        int start_addr = reg_addr + total_read;
        frame[2] = (start_addr >> 8) & 0xFF;
        frame[3] = start_addr& 0xFF;
        frame[4] = (batch >> 8) & 0xFF;
        frame[5] = batch & 0xFF;

        unsigned short crc = crc16_modbus(frame, 6);
        frame[6] = crc & 0xFF;
        frame[7] = (crc >> 8) & 0xFF;

        uart_send(frame, 8);
        printf("Sent frame : ");
        for (int i = 0; i < 8; i++) printf("%02X ", frame[i]);
        printf("\n");
        //dynamic wait time based on number of registers
        unsigned char response[64];
        int resp_len = 2 * batch + 5;
        int rcv = uart_received_timeout(response, resp_len, TIMEOUT_US);
        //if (rcv < resp_len) return -1; //timeout
        if (rcv <= 0 || rcv < resp_len) {
            printf("rcv = %d\n", rcv);
            printf("Timeout or incomplete response! Expected %d, got %d\n", resp_len, rcv);
        for (int i = 0; i < rcv; i++) {
            printf("%02X ", response[i]);
        }
        printf("\n");
        return -1;
    }
        unsigned short crc_resp = crc16_modbus(response, resp_len - 2);
        unsigned short crc_recv = response[resp_len - 2] | (response[resp_len - 1] << 8);
        if (crc_resp != crc_recv) return -2;  // invalid CRC
        if (response[1] != 0x03) return -3;   // error in response

        for (int i = 0; i < batch; i++) {
            values[total_read + i] = response[3 + 2*i] << 8 | response[4 + 2*i];
        }

        total_read += batch;
        printf("--------------------------\n");
        printf("Raw data: ");
        for (int i = 0; i < rcv; i++) {
            printf("%02X ", response[i]);
        }
        printf("\n");

        printf("CRC calc=%04X, recv=%04X -> %s\n",
            crc_resp, crc_recv,
            (crc_resp == crc_recv) ? "OK" : "FAIL");

        for (int i = 0; i < batch; i++) {
            printf("Register[%d] = %d (0x%04X)\n",
                total_read + i, values[total_read + i], values[total_read + i]);
        }
        printf("--------------------------\n");
    }
    return 0;
}

int write_modbus_register(int slave_addr, int reg_addr, int num_regs,int *value){
    //implementation of UART write
    // don't implement a code right now
    return 0;
}

void uart_send(const unsigned char *data, int len) {
    // Implementation of UART send
    //gpio_put(MAX485_DE_RE_PIN, 1);
    uart_write_blocking(UART_PORT, data, len);
    //while (!uart_is_writable(UART_PORT)); 
    uart_tx_wait_blocking(UART_PORT);
    //gpio_put(MAX485_DE_RE_PIN, 0);
    sleep_us(3500);
}

int uart_received_timeout(unsigned char *buf, int expected_len,unsigned int timeout_us) {
    // Implementation of UART receive
    int received = 0;
    absolute_time_t start = get_absolute_time();

    while (received < expected_len) {
        if (uart_is_readable(UART_PORT)) {
            buf[received++] = uart_getc(UART_PORT);
        }
        if (absolute_time_diff_us(start, get_absolute_time()) > timeout_us) {
            return -1; // timeout
        }
    }
    return received;
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