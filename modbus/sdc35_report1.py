import serial
from datetime import datetime
import time
import subprocess
import os
import csv

PORT = '/dev/ttyACM0'
BAUDRATE = 9600

now = datetime.now()
filename = f"sdc35_report1_{now.strftime('%Y-%m-%d_%H-%M-%S')}.csv"
filename4 = "time_verification.csv"

ser = serial.Serial(PORT, BAUDRATE, timeout=2)
time.sleep(2)

print("Sending report1 command and logging data")

readcsv_result = {}
start = time.perf_counter()

with open(filename, 'w') as f:
    header_written = False

    try:
        # Send 'report1' ones
        ser.write(b"report1\n")

        #  Read data in file "report1 done"
        while True:
            line = ser.readline().decode(errors='ignore').strip()
            if not line:
                continue

            # End after done
            if line.startswith("report1 done") or line.startswith("Connected"):
                print("Report received completely.")
                break

            # Save header once
            if not header_written and ',' in line:
                f.write(line + '\n')
                f.flush()
                header_written = True
                print("Header written:", line)
                continue

            # Save CSV data
            if header_written and ',' in line:
                f.write(line + '\n')
                f.flush()
                print("Written line:", line)

    except KeyboardInterrupt:
        print("Data logging stopped by user.")
    finally:
        ser.close()
        print(f"Serial port closed. Data saved to {filename}")

        end = time.perf_counter()
        readcsv_result["readCSV_report1"] = round(end - start, 6)

        file_exists = os.path.isfile("time_verification.csv")
        with open(filename4, 'a', newline="") as f4:
            writer = csv.writer(f4)
            if not file_exists:
                writer.writerow(["function","time"])
            for name, t in readcsv_result.items():
                writer.writerow([name,t])