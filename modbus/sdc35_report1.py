import serial
from datetime import datetime
import time
import subprocess
import os

PORT = '/dev/ttyACM0'
BAUDRATE = 9600

# Tworzymy nazwę pliku z timestampem
now = datetime.now()
filename = f"sdc35_report1_{now.strftime('%Y-%m-%d_%H-%M-%S')}.csv"

# Otwieramy port szeregowy
ser = serial.Serial(PORT, BAUDRATE, timeout=2)
time.sleep(2)  # poczekaj aż Pico się ustabilizuje

print("Sending report1 command and logging data")

with open(filename, 'w') as f:
    header_written = False

    try:
        # Wyślij komendę report1 tylko raz
        ser.write(b"report1\n")

        # Czytamy linie aż pojawi się "report1 done"
        while True:
            line = ser.readline().decode(errors='ignore').strip()
            if not line:
                continue

            # Kończymy po komunikacie końcowym
            if line.startswith("report1 done") or line.startswith("Connected"):
                print("Report received completely.")
                break

            # Zapisujemy nagłówek tylko raz
            if not header_written and ',' in line:
                f.write(line + '\n')
                f.flush()
                header_written = True
                print("Header written:", line)
                continue

            # Zapisujemy dane CSV
            if header_written and ',' in line:
                f.write(line + '\n')
                f.flush()
                print("Written line:", line)

    except KeyboardInterrupt:
        print("Data logging stopped by user.")
    finally:
        ser.close()
        print(f"Serial port closed. Data saved to {filename}")