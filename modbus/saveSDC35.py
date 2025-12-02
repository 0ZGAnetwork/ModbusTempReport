# """
# This script generate 3 report:
# - report 1: read current configuration and data,
# - report 2: receive data for 5 min,
# - report 3: receive data for 24 h

# all report generate one pdf file on github !
# """
import serial                                        # seriall communication
import time
import csv
import os
from datetime import datetime
import matplotlib.pyplot as plt                      # plot graph
from matplotlib.backends.backend_pdf import PdfPages 
from reportlab.lib.pagesizes import A4               # generate PDF:
from reportlab.pdfgen import canvas
from reportlab.lib.utils import ImageReader

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
    'alarm_motor_fail', 'alarm_ct_over', 'error_e1_min', 'error_e1_max'
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

def report2_test(csv_writer, csv_file, test_file=OUTPUT_CSV_TEST):
    print('report 2 TEST: receive data for 5 min')

    if not os.path.exists(test_file):
        print(f'File {test_file} does not exist')
        return

    with open(test_file, 'r', encoding='utf-8-sig') as f:
        lines = [line.strip() for line in f if line.strip() and not line.startswith('timestamp')]
        # strip- delete white sign, startswith- ignore empty line and timestamp
    start_time = time.time()
    i = 0
    while time.time() - start_time < 5*60: 
        timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')

        if i < len(lines):
            values = lines[i].split(',')
            if len(values) == len(CSV_HEADERS) - 1:
                row = [timestamp] + values
            else:
                row = [timestamp] + [0]*(len(CSV_HEADERS)-1)
            i += 1
        else:
           
            row = [timestamp] + [0]*(len(CSV_HEADERS)-1)

        csv_writer.writerow(row)
        csv_file.flush()
        print("Received", row)

        time.sleep(1)  


def report2_generate():
    timestamps, pv_lo_max_values, alarms = [], [], []
    error_e1_min, error_e1_max = [], []

    OUTPUT_CSV_TEST= 'test_200_samples.csv'
    with open(OUTPUT_CSV_TEST, 'r', encoding='utf-8-sig') as f:
        reader = csv.DictReader(f)
        for row in reader:
            timestamps.append(datetime.strptime(row['timestamp'], '%Y-%m-%d %H:%M:%S'))
            pv_lo_max_values.append(float(row['pv_lo_max']))
            alarms.append(int(row["alarm"]))
            error_e1_min.append(float(row["error_e1_min"]))
            error_e1_max.append(float(row["error_e1_max"]))

    fig, ax = plt.subplots(figsize=(8,3))
    report2_plot(ax, timestamps, pv_lo_max_values, alarms, error_e1_min[0], error_e1_max[0])
    plot_filename = "report2_plot.png"
    fig.savefig(plot_filename, bbox_inches='tight')
    plt.close(fig)

    pdf_file = "Report2_ReportLab.pdf"
    c = canvas.Canvas(pdf_file, pagesize=A4)
    width, height = A4
    y = height - 50

    def add_section(title, text):
        nonlocal y
        c.setFont("Helvetica-Bold", 14)
        c.drawString(50, y, title)
        y -= 20
        c.line(50, y, width-50, y)
        y -= 20
        c.setFont("Helvetica", 12)
        c.drawString(50, y, text)
        y -= 40

    add_section("Summary", "Słowo1  Słowo2")
    add_section("Alarms: Error handling", "Słowo3  Słowo4")

    # Wstawiamy wykres
    y -= 10
    img = ImageReader(plot_filename)
    c.drawImage(img, 50, y-200, width=500, height=200) 
    c.save()
    print(f"PDF save such as {pdf_file} with plot.")
        
def report2_plot(ax, timestamps, pv_lo_max_values, alarms, error_min, error_max):
    ax.plot(timestamps, pv_lo_max_values, color='blue', linestyle='-', label='PV')
    error_min = 10
    error_max = 15
    ax.axhline(y=error_min, linestyle='--', color='red', label='Error Min')
    ax.axhline(y=error_max, linestyle='--', color='red', label='Error Max')

    for i in range(1, len(timestamps)):
        t0, t1 = timestamps[i-1], timestamps[i]
        y0, y1 = pv_lo_max_values[i-1], pv_lo_max_values[i]
        if alarms[i-1] == 1 or alarms[i] == 1 or y0 > error_max or y1 > error_max:
            color = 'red'
        elif y0 < error_min or y1 < error_min:
            color = 'darkblue'
        else:
            color = 'blue'
        ax.plot([t0, t1], [y0, y1], color=color)

    for i in range(len(timestamps)):
        y = pv_lo_max_values[i]
        if alarms[i] == 1 or y > error_max:
            point_color = 'red'
        elif y < error_min:
            point_color = 'darkblue'
        else:
            point_color = 'blue'
        ax.scatter(timestamps[i], y, color=point_color)

    ax.set_ylim(0,32)
    ax.set_title('Report2 Process Template ')
    ax.set_xlabel('Time')
    ax.set_ylabel('Temp [°C]')
    ax.grid(True)
    ax.legend()

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


csvfile = open(OUTPUT_CSV_TEST_RESULT, 'w', newline='')
csv_writer = csv.writer(csvfile)
if csvfile.tell() == 0:
    csv_writer.writerow(CSV_HEADERS)

report2_generate()
#report2_test(csv_writer, csvfile, OUTPUT_CSV_TEST)

csvfile.close()