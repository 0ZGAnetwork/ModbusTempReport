#ifdef DRIVER_H
#define DRIVER_H

int read_modbus_register(int slave_addr, int reg_addr, int *value);
void uart_send(const unsigned char *data, int len);
int uart_receive(unsigned char *buf, int max_len);
unsigned short crc16_modbus(const unsigned char *buf, int len);

#endif