import serial
from datetime import datetime
import time

PORT = '/dev/ttyACM0'
BAUDRATE = 9600

# Tworzymy nazwę pliku z timestampem
now = datetime.now()
filename2 = f"sdc35_report2_{now.strftime('%Y-%m-%d_%H-%M-%S')}.csv"

# Otwieramy port szeregowy
ser = serial.Serial(PORT, BAUDRATE, timeout=2)
time.sleep(2)  # poczekaj aż Pico się ustabilizuje

print("Starting 1-minute logging, sending report1 every 5 seconds")

with open(filename2, 'w') as f:
    start_time = time.time()
    duration = 60  # 60 sekund
    interval = 5   # co 5 sekund

    header_written = False

    try:
        while time.time() - start_time < duration:
            # Wyślij komendę report1
            ser.write(b"report1\n")

            # Czytamy linie aż dostaniemy pełny snapshot
            while True:
                line = ser.readline().decode(errors='ignore').strip()
                if not line:
                    continue

                # Ignorujemy komunikaty typu report1 done
                if line.startswith("report1 done") or line.startswith("Connected"):
                    break

                # Zapisujemy nagłówek tylko raz
                if not header_written and line.startswith("time_ms"):
                    f.write(line + '\n')
                    f.flush()
                    header_written = True
                    continue

                # Zapisujemy dane CSV
                if ',' in line:
                    f.write(line + '\n')
                    f.flush()
                    print(f"Written line: {line}")

            # Czekamy do kolejnego interwału
            elapsed = time.time() - start_time
            sleep_time = interval - (elapsed % interval)
            if sleep_time > 0:
                time.sleep(sleep_time)

    except KeyboardInterrupt:
        print("Data logging stopped by user.")
    finally:
        ser.close()
        print(f"Serial port closed. Data saved to {filename2}")