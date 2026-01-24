#include "driver.h"
#include "pico/time.h"
#include <string.h>
#include <stdio.h>
#define MAX_REGS 16
#define TIMEOUT_US 100000  

void uart_init_max485() {
    uart_init(UART_PORT, 9600);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    uart_set_format(UART_PORT, 8, 1, UART_PARITY_NONE); // 8N1
}

int modbus_check_slave(int slave_addr) {
        int dummy;
        int result = read_modbus_registers(slave_addr, 9204, 1, &dummy); // próbujemy odczytać pierwszy rejestr
        if (result == 0) {
            return 1; // slave odpowiada
        } else {
            return 0; // brak odpowiedzi / błąd CRC / wyjątek
        }
    }

int read_modbus_registers(int slave_addr, int reg_addr, int num_regs, int *values) {
    int total_read = 0;

    while (total_read < num_regs) {

        int batch = (num_regs - total_read > MAX_REGS) ? MAX_REGS : (num_regs - total_read);

        unsigned char frame[8];
        frame[0] = (unsigned char)slave_addr;
        frame[1] = 0x03; // Read Holding Register (back to 0x03)
        int start_addr = reg_addr + total_read;
        frame[2] = (start_addr >> 8) & 0xFF;
        frame[3] = start_addr & 0xFF;
        frame[4] = (batch >> 8) & 0xFF;
        frame[5] = batch & 0xFF;

        unsigned short crc = crc16_modbus(frame, 6);
        frame[6] = crc & 0xFF;
        frame[7] = (crc >> 8) & 0xFF;

        while(uart_is_readable(UART_PORT)) uart_getc(UART_PORT); // czyszczenie bufora

        uart_send(frame, 8);
        printf("Sent frame (FC=03): ");
        for (int i = 0; i < 8; i++) printf("%02X ", frame[i]);
        printf("\n");

        unsigned char response[128];
        int resp_len = 2 * batch + 5; // expected length assuming correct byte_count
        int rcv = uart_read_modbus_response(response, sizeof(response), TIMEOUT_US); // 01 03 02 XX XX CRC_L CRC_H

        printf("Bytes received: %d\n", rcv);

        if (rcv <= 0) {
            printf("Timeout or incomplete response! Expected at least %d bytes, got %d error\n", resp_len, rcv);
            return -1;
        }
        // If device returned different byte_count, compute actual expected length
        int actual_expected = response[2] + 5;
        if (rcv != actual_expected) {
            printf("Warning: requested %d registers but device reported byte_count %d -> expected %d bytes, actual %d\n",
                   batch,
                   response[2],
                   actual_expected, rcv);
            // will continue and try to validate CRC on what we got
        }
        printf("Raw data, Full response: ");
        for (int i = 0; i < rcv; i++) printf("%02X ", response[i]);
        printf("\n");


        unsigned short crc_resp = crc16_modbus(response, actual_expected - 2);
        unsigned short crc_recv = response[actual_expected - 2] | (response[actual_expected - 1] << 8);
        //unsigned short crc_recv = (response[resp_len - 2] << 8) | response[resp_len - 1];

        printf("CRC calc=%04X, recv=%04X -> %s\n",
               crc_resp, crc_recv,
               (crc_resp == crc_recv) ? "OK" : "FAIL");
        if (crc_resp != crc_recv) return -2;

        if (response[1] != 0x03) return -3;
        if (response[1] & 0x80) {
            printf("Modbus exception: %02X\n", response[2]);
            return -4;
        }

        int regs_in_response = response[2] / 2;
        int to_copy = (regs_in_response < batch) ? regs_in_response : batch;
        for (int i = 0; i < to_copy; i++) {
            values[total_read + i] = response[3 + 2*i] << 8 | response[4 + 2*i];
            printf("Register[%d] bytes: %02X %02X -> %d (0x%04X)\n",
                   total_read + i,
                   response[3 + 2*i], response[4 + 2*i],
                   values[total_read + i], values[total_read + i]);
        }
        if (to_copy < batch) {
            printf("Warning: device returned fewer registers (%d) than requested (%d)\n", regs_in_response, batch);
            return -5;
        }

        total_read += batch;

        printf("--------------------------\n");
        printf("Raw data: ");
        for (int i = 0; i < rcv; i++) printf("%02X ", response[i]);
        printf("\n");
    }

    return 0;
}

int write_modbus_register(int slave_addr, int reg_addr, int num_regs,int *value){
    //implementation of UART write
    // don't implement a code right now
    return 0;
}

void uart_send(const unsigned char *data, int len) {
    // Implementation of UART send for TTL-485 v2.0 (auto direction control)
    uart_write_blocking(UART_PORT, data, len);
    uart_tx_wait_blocking(UART_PORT);
    
    // TTL-485 v2.0 needs time to switch from TX to RX mode
    //sleep_ms(200);  // Wait 200ms for direction to switch and settle
    
    // Clear any echo/residual data from RX buffer
    int cleared = 0;
    while(uart_is_readable(UART_PORT)) {
        uart_getc(UART_PORT);
        cleared++;
    }
    if (cleared > 0) {
        printf("DEBUG: Cleared %d echo bytes from RX buffer\n", cleared);
    }
        sleep_ms(200);  // Give SDC35 device 200ms to process and respond
}

int uart_received_timeout(unsigned char *buf, int expected_len,unsigned int timeout_us) {
    // Implementation of UART receive that waits until expected_len bytes are read
    int received = 0;
    absolute_time_t start = get_absolute_time();
    absolute_time_t last_byte_time = start;

    while (received < expected_len) {
        if (uart_is_readable(UART_PORT)) {
            buf[received++] = uart_getc(UART_PORT);
            last_byte_time = get_absolute_time();  // Reset timer on each byte received
            printf("DEBUG: Received byte %d/%d = 0x%02X\n", received, expected_len, buf[received-1]);
        }
        
        // Timeout: either total time exceeded OR no byte for 500ms
        uint64_t total_time = absolute_time_diff_us(start, get_absolute_time());
        uint64_t byte_idle_time = absolute_time_diff_us(last_byte_time, get_absolute_time());
        
        if (total_time > timeout_us || byte_idle_time > 500000) {  // 500ms idle timeout
            printf("DEBUG: Timeout - total_time=%lld, byte_idle_time=%lld, received=%d\n", total_time, byte_idle_time, received);
            return received > 0 ? received : -1;
        }
    }
    return received;
}

// Read entire Modbus response in two stages: header (3 bytes) then data+CRC based on byte_count
int uart_read_modbus_response(unsigned char *buf, int buf_size, unsigned int timeout_us) {
    if (buf_size < 5) return -1; // minimum modbus frame size
    // read header (slave + func + byte count)
    int rcv = uart_received_timeout(buf, 3, timeout_us);
    if (rcv <= 0) return -1; // timeout or fail
    if (rcv < 3) {
        // partial header
        int rcv2 = uart_received_timeout(buf + rcv, 3 - rcv, timeout_us);
        if (rcv2 <= 0) return -1;
        rcv += rcv2;
    }
    
    printf("DEBUG: Full header received: %02X %02X %02X\n", buf[0], buf[1], buf[2]);
    
    // Check if this is an exception response (function code | 0x80)
    if (buf[1] & 0x80) {
        printf("DEBUG: Exception response detected! Function=0x%02X, Exception Code=0x%02X\n", buf[1], buf[2]);
        // Exception response has only 5 bytes total: slave + func + exception_code + crc(2)
        if (rcv < 5) {
            int rcv2 = uart_received_timeout(buf + rcv, 5 - rcv, timeout_us);
            if (rcv2 <= 0) return -1;
            rcv += rcv2;
        }
        return rcv;
    }
    
    // we now have header: buf[2] = byte_count (payload length)
    int byte_count = buf[2];
    printf("DEBUG: byte_count = %d (0x%02X)\n", byte_count, byte_count);
    int total_expected = byte_count + 5; // slave+func+byte_count + data(byte_count) + crc(2)
    if (total_expected > buf_size) {
        printf("Response too long for buffer: expected %d > %d\n", total_expected, buf_size);
        return -2;
    }
    // read remaining bytes
    if (rcv < total_expected) {
        int remaining = total_expected - rcv;
        printf("DEBUG: Need to read %d more bytes (already have %d, expecting total %d)\n", remaining, rcv, total_expected);
        int rcv2 = uart_received_timeout(buf + rcv, remaining, timeout_us);
        if (rcv2 <= 0) return -1;
        rcv += rcv2;
    }
    return rcv == total_expected ? rcv : -1;
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