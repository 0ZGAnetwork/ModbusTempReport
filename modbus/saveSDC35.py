import serial
import time
import csv
from datetime import datetime

SERIAL_PORT = 'COM6' # Update as needed
BAUD_RATE = 9600
OUTPUT_CSV = 'snapshot.csv'

with serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1) as ser:
    print(f"Connected to {SERIAL_PORT} as {BAUD_RATE} bps")
    
    with open(OUTPUT_CSV, 'a', newline='') as csvfile:
        csv_writer = csv.writer(csvfile)
        
        csv_writer.writerow([
            'timestamp', 
            'pv_lo_max', 'pv_hi_max', 'sv_lo_max', 'sv_hi_max', 
            'config', 'alarm', 'alarm_pv_over', 'alarm_pv_under', 
            'alarm_cj_burnout', 'alarm_rsp_over', 'alarm_mfb_burnout', 
            'alarm_motor_fail', 'alarm_ct_over'
        ])
        
        try:
            while True:
                line = ser.readline().decode('utf-8').strip()
                if line:
                    
                    if line.startswith("Timestamp"):
                        continue
                    
                    values = line.split(',')
                    if len(values) == 14:
                        current_time = datetime.now().strftime('%Y-%m-%d %H:%M:%S")')
                        csv_writer.writerow([current_time] + values)
                        csvfile.flush()
                        print("Received:", [current_time] + values)
                
        except KeyboardInterrupt:
            print("Exiting...")