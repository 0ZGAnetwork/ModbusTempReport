import serial
from datetime import datetime
import time
import os
import csv

PORT = '/dev/ttyACM0'
BAUDRATE = 9600

now = datetime.now()
filename2 = f"sdc35_report2_{now.strftime('%Y-%m-%d_%H-%M-%S')}.csv"
filename4 = "time_verification.csv"
ser = serial.Serial(PORT, BAUDRATE, timeout=2)
time.sleep(2)  

print("Starting 1-minute logging, sending report1 every 5 seconds")

readcsv_result = {}
start = time.perf_counter()

with open(filename2, 'w') as f:
    start_time = time.time()
    duration = 60
    interval = 5 

    header_written = False

    try:
        while time.time() - start_time < duration:
            ser.write(b"report1\n")

            while True:
                line = ser.readline().decode(errors='ignore').strip()
                if not line:
                    continue

                if line.startswith("report1 done") or line.startswith("Connected"):
                    break

                if not header_written and line.startswith("time_ms"):
                    f.write(line + '\n')
                    f.flush()
                    header_written = True
                    continue

                if ',' in line:
                    f.write(line + '\n')
                    f.flush()
                    print(f"Written line: {line}")

            elapsed = time.time() - start_time
            sleep_time = interval - (elapsed % interval)
            if sleep_time > 0:
                time.sleep(sleep_time)

    except KeyboardInterrupt:
        print("Data logging stopped by user.")
    finally:
        ser.close()
        print(f"Serial port closed. Data saved to {filename2}")

        end = time.perf_counter()
        readcsv_result["readCSV_report2"] = round(end - start, 6)

        file_exists = os.path.isfile("time_verification.csv")
        with open(filename4, 'a', newline="") as f4:
            writer = csv.writer(f4)
            if not file_exists:
                writer.writerow(["function","time"])
            for name, t in readcsv_result.items():
                writer.writerow([name,t])