"""
This script generate 3 report:
- report 1: read current configuration and data,
- report 2: receive data for 5 min,
- report 3: receive data for 24 h

all report generate one pdf file on github !
"""
import serial
import time
import csv
from datetime import datetime
import matplotlib.pyplot as plt

 # Update as needed
SERIAL_PORT = 'COM6'
BAUD_RATE = 9600
OUTPUT_CSV = 'snapshot.csv'

CSV_HEADERS = [
    'timestamp', 
    'pv_lo_max', 'pv_hi_max', 'sv_lo_max', 'sv_hi_max', 
    'config', 'alarm', 'alarm_pv_over', 'alarm_pv_under', 
    'alarm_cj_burnout', 'alarm_rsp_over', 'alarm_mfb_burnout', 
    'alarm_motor_fail', 'alarm_ct_over'
]

try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    print(f"Connected to {SERIAL_PORT} as {BAUD_RATE} bps")
except serial.SerialException as e:
    print("Cant open the port: ",e)
    exit(1)

csvfile = open(OUTPUT_CSV, 'a', newline='')
csv_writer = csv.writer(csvfile)
if csvfile.tell() == 0: 
    csv_writer.writerow(CSV_HEADERS)

plt.ion()
fig, ax = plt.subplots()
times = []
pv_hi_max_vals = []

try: 
    while True:
        line = ser.readline().decode('utf-8').strip()   # configuration csv
        if not line or line.startswith("Timestamp"):
            continue

        values = line.split(',')    # get data
        if len(values) == len(CSV_HEADERS) - 1:
            timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
            row = [timestamp] + values
            csv_writer.writerow(row)
            csvfile.flush()
            print("Received", row)

            times.append(timestamp)
            pv_hi_max_vals.append(float(values[1]))

        #     ax.clear()
        #     ax.plot(times, pv_hi_max_vals, label='PV_HI_MAX')
        #     ax.set_xlabel('Time')
        #     ax.set_ylabel('PV_HI_MAX')
        #     ax.set_title('PV_HI_MAX w czasie rzeczywistym')
        #     ax.legend()
        #     plt.xticks(rotation=45)
        #     plt.pause(0.1)
        # else:
        #     print('Fotmat error')

except KeyboardInterrupt:
    print("Exit")
    # plt.ioff()
    # plt.show()
    csvfile.close()
    ser.close()