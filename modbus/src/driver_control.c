#include "driver_control.h"
#include "driver.h"
#include <stdio.h>
#include "sdc35_status.h"

#define SLAVE_ADDR 2
//1494 parity even
//1490 comunication type
//1451 pv, 51 -> PT100 -50:200
//238D Operation Display: PV | 234 REAL 23.4
void test_modbus_slave() {
    unsigned char frame[8] = {SLAVE_ADDR, 0x03, 0x23, 0x8D, 0x00, 0x01};
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

void create_snapshot(SDC35Status *status) {
      
    if (!modbus_check_slave(SLAVE_ADDR)) {
        printf("DEBUG: Slave %d nie odpowiada!\n", SLAVE_ADDR);
        status->pv = -1;
        format_timestamp(status->timestamp, sizeof(status->timestamp));
        return;
    }
    printf("Slave %d is responsive.\n", SLAVE_ADDR);
    
    status->pv = -1;
    format_timestamp(status->timestamp, sizeof(status->timestamp));
        
//     int address = 1455;
//     // Build Modbus RTU frame for address 0x05AF (1455)
//     unsigned char frame[8];
//     frame[0] = SLAVE_ADDR;           // Slave address
//     frame[1] = 0x03;                 // Function code (read holding registers)
//     frame[2] = (address >> 8) & 0xFF;  // Address high byte
//     frame[3] = address & 0xFF;         // Address low byte
//     frame[4] = 0x00;                 // Read 1 register (high byte)
//     frame[5] = 0x01;                 // Read 1 register (low byte)
    
//     // Calculate and add CRC
//     unsigned short crc = crc16_modbus(frame, 6);
//     frame[6] = crc & 0xFF;
//     frame[7] = (crc >> 8) & 0xFF;
    
//     // Debug: print sent frame
//     printf("Sending Modbus frame (address 0x%04X): ", address);
//     for (int i = 0; i < 8; i++) printf("%02X ", frame[i]);
//     printf("\n");
    
//     // Send frame
//     uart_send(frame, 8);
    
//     // Read response
//     printf("Response: ");
//     int count = 0;
//     while(uart_is_readable(UART_PORT) && count < 50) {
//         unsigned char byte = uart_getc(UART_PORT);
//         printf("%02X ", byte);
//         count++;
//     }
//     printf("(%d bytes)\n", count);
    
//     status->pv = -1;
//     format_timestamp(status->timestamp, sizeof(status->timestamp));
 }

    // ignore:
    // void save_Snapshot_csv(const SDC35Status *status){
    //    FILE *f = fopen("snapshot.csv", "w");
    //    if (f) {
    //        fprintf(f, "Timestamp,PV,SV,Alarm\n");
    //        fprintf(f, "%s,%.1f,%.1f,%d\n",
    //                status->timestamp, status->pv, status->sv, status->alarm);
    //        fclose(f);
    //    }
    // }

 void save_Snapshot_uart(const SDC35Status *status){
//     printf("pv\n");

//     printf("%.1f\n",
//            status->pv);//pv_lo_max?
 }

 void format_timestamp(char *buf, int buf_size){
     snprintf(buf, buf_size, "Status: Test Snapshot ");
 }