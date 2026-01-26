About:
This project uses the Modbus RTU communication (board TTL485)" protocol to exchange data between the SDC35 and the Raspberry Pi Pico 2W, which acts as the central data acquisition and reporting unit.

Device used in project:
Raspberry pi 2W, Arduino Uno, TTl-485 v2.0, azbil SDC35, DR-120-24, sensor PT100

Run project:
- cd modbus
- source modbus_env/bin/activate
- python3 sdc35_menu.py

command to build project:
- cd modbus/build
- rm -rf *
- cmake .. -G "Ninja" -DPICO_BOARD=pico2_w
- ninja

command to copy the .uf2 and paste in BOOTSEL Raspberry
- lsblk
- cp Modbus_example.uf2 /media/$USER/RP2350/

modbus configuration:
para->setup>64-70
64: 2, modbus RTU format
65: 2, adres
66: 1, speed baudrate 9600
67: -, data format
68: 2, No parity
69: 0, 1 stop bit
70: -
 
