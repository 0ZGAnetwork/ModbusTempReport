# """
# This script generate 3 report:
# - report 1: read current configuration and data,
# - report 2: receive data for 5 min,
# - report 3: receive data for 24 h

# all report generate one pdf file on github !
# """
import serial
import time
import csv
from datetime import datetime
import matplotlib.pyplot as plt
import os

#  # Update as needed
SERIAL_PORT = 'COM6'
BAUD_RATE = 9600
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
OUTPUT_CSV = os.path.join(BASE_DIR,'snapshot.csv')
OUTPUT_CSV_TEST = os.path.join(BASE_DIR,'test_from_port.csv')
OUTPUT_CSV_TEST_RESULT = os.path.join(BASE_DIR,'test_to_csv.csv')

CSV_HEADERS = [
    'timestamp', 
    'pv_lo_max', 'pv_hi_max', 'sv_lo_max', 'sv_hi_max', 
    'config', 'alarm', 'alarm_pv_over', 'alarm_pv_under', 
    'alarm_cj_burnout', 'alarm_rsp_over', 'alarm_mfb_burnout', 
    'alarm_motor_fail', 'alarm_ct_over'
]

def report1_test(csv_writer, csv_file, test_file=OUTPUT_CSV_TEST):
    print("report 1 TEST: read current configuration and data --once")
    
    if not os.path.exists(test_file):
        print(f'File {test_file} do not exist')
        return

    with open(test_file,'r',encoding='utf-8-sig') as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("timestamp"):
                continue
            values = line.split(',')
            if len(values) == len(CSV_HEADERS):
                #timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
                #row = [timestamp] + values
                row = values
                csv_writer.writerow(row)
                csv_file.flush()
                print("Received", row)
                break

csvfile = open(OUTPUT_CSV_TEST_RESULT, 'w', newline='')
csv_writer = csv.writer(csvfile)
if csvfile.tell() == 0:
    csv_writer.writerow(CSV_HEADERS)

report1_test(csv_writer, csvfile, OUTPUT_CSV_TEST)

csvfile.close()
# def report1():
#         print("report 1: read current configuration and data --once")
        
#         while True:
#             #line = ser.readline().decode('utf-8').strip()   # configuration csv
            
#             if not line or line.startswith("Timestamp"):
#                 continue

#             values = line.split(',')    # get data
#             if len(values) == len(CSV_HEADERS) - 1:
#                 timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
#                 row = [timestamp] + values
#                 csv_writer.writerow(row)
#                 csvfile.flush()
#                 print("Received", row)
#                 break


# def report2():
#     print('rep2')

# def report3():
#     print('rep3')

# def test():
#     print('test')

# def read_port_COM():
#     try:
#         ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
#         print(f"Connected to {SERIAL_PORT} as {BAUD_RATE} bps")
#     except serial.SerialException as e:
#         print("Cant open the port: ",e)
#         exit(1)

#     csvfile = open(OUTPUT_CSV, 'a', newline='')
#     csv_writer = csv.writer(csvfile)
#     if csvfile.tell() == 0: 
#         csv_writer.writerow(CSV_HEADERS)

#     try:
#         while True:
#             line = ser.readline().decode('utf-8').strip()
#             if not line:
#                 continue

#             if line.lower() == 'report1':
#                 report1()
#             elif line.lower() == 'report2':
#                 report2()
#             elif line.lower() == 'report3':
#                 report3()
#             elif line.lower() == 'test':
#                 test()
#             else:
#                 print("unknow command: ",line)

#     except KeyboardInterrupt:
#         print('Exit')
#         csvfile.close()
#         ser.close()