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

To do:
- Schemat of Hardware
- add reading mor than one register
- prepare data structure
- connect with report1

czyli jak ustawie w do : Eu1.1 na 0 to ponizej PV<28 bede mial wyswietlony ev1, a jezeli ustawie w do : Eu1.1 na 1 to przekraczajac temperature PV>25 mam przelaczany

w 'do' : Eu1.1 na 0 to PV < 28 → EV1 świeci, D1 nic nie robi
w 'do' : Eu1.1 na 1 to grzalka pracuje a EV1 tylko przelacza sie
dodatkowo majac wlaczone Eu1.1 na 1 i
'EuCF': E1.C1 na 4 (deviation limit ) jezeli PV(28) + 4 = przy 32 zaloczy sie D1 
