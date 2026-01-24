#ifndef DRIVER_CONTROL_H
#define DRIVER_CONTROL_H

#include "sdc35_status.h"

void create_snapshot(SDC35Status *status);
//void save_Snapshot_csv(const SDC35Status *status);
void save_Snapshot_uart(const SDC35Status *status);
void format_timestamp(char *buf, int buf_size);
void test_modbus_slave(void);

#endif