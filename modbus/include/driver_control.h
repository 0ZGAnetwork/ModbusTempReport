#ifdef DRIVER_CONTROL_H
#define DRIVER_CONTROL_H

#include "sdc35_status.h"

void create_snapshot(sdc35status *status);
void save_Snapshot_csv(const sdc35status *status);

#endif