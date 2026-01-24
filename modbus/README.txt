command to build:
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
65: 1, adres
66: 1, speed baudrate 9600
67: , data format
68: 2, No parity
69: 0, 1 stop bit
70: -

