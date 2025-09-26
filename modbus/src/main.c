#include "sdc35_status.h"
#include "driver_control.h"


int main (void){
    stdio_init_all();
    uart_init_max485();
    
    SDC35Status status;
    create_snapshot(&status);
    save_Snapshot_csv(&status);
    
    return 0;
}
