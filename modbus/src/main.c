#include "sdc35_status.h"
#include "driver_control.h"

int main (void){
    SDC35Status status;
    create_snapshot(&status);
    save_snapshot_csv(&status);
    
    return 0;
}
