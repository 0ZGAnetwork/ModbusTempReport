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
from reportlab.pdfgen.canvas import Canvas
from reportlab.pdfgen import canvas
from reportlab.platypus import Paragraph, Spacer, Image, PageBreak, SimpleDocTemplate, BaseDocTemplate, PageTemplate, Frame, Table, TableStyle
from reportlab.lib import colors
from reportlab.lib.styles import getSampleStyleSheet, ParagraphStyle
from reportlab.lib.pagesizes import A4               # generate PDF:
from reportlab.lib.utils import ImageReader
from reportlab.lib.units import cm
from PIL import Image as PILImage                   # convert background for .png
from reportlab.pdfbase.pdfmetrics import stringWidth
from reportlab.pdfbase.ttfonts import TTFont
from reportlab.pdfbase import pdfmetrics
import subprocess                                   # for upload report to github

#  # Update as needed
SERIAL_PORT = 'COM6'
BAUD_RATE = 9600
BASE_DIR = os.path.dirname(os.path.abspath(__file__))

OUTPUT_CSV = os.path.join(BASE_DIR,'snapshot.csv')
OUTPUT_CSV_TEST = os.path.join(BASE_DIR,'test_200_samples.csv')
OUTPUT_CSV_TEST_RESULT = os.path.join(BASE_DIR,'test_to_csv.csv')


logo_file = os.path.join(BASE_DIR, "MTCPreports.jpg")
logo_file2 = os.path.join(BASE_DIR, "PT100.jpg")

all_files  = [OUTPUT_CSV,OUTPUT_CSV_TEST,OUTPUT_CSV_TEST_RESULT, logo_file, logo_file2]

missing_files = [f for f in all_files if not os.path.exists(f)]

if missing_files:
    print("The following required files are missing:")
    for f in missing_files:
        print(f" - {f}")
else:
    print("All required files exist.")


if not os.path.exists(logo_file):
    raise FileNotFoundError(f"File does not exist: {logo_file}")
if not os.path.exists(logo_file2):
    raise FileNotFoundError(f"File does not exist: {logo_file2}")

CSV_HEADERS = [
    'timestamp', 
    'pv_lo_max', 'pv_hi_max', 'sv_lo_max', 'sv_hi_max', 
    'config', 'alarm', 'alarm_pv_over', 'alarm_pv_under', 
    'alarm_cj_burnout', 'alarm_rsp_over', 'alarm_mfb_burnout', 
    'alarm_motor_fail', 'alarm_ct_over', 'error_e1_min', 'error_e1_max'
]
EVENT_INTERNAL = [
    'event_pv_high', 'event_pv_low',
    'event_sv_high', 'event_sv_low',
    'event_mv_high', 'event_mv_low'
]
BASE = [
    'mv - manipulated value %', 'RunStop', 'Mode',
    'config', 'OperationMode', 'grupPID',
    'PID P', 'PID I', 'PID D'
]
OPERATION = [
    'lsp_group_selection',       # LSP group selection
    'lsp_value_in_use',          # LSP value in use
    'manual_mv',                 # Manual manipulated variable (MV)
    'run_ready',                 # Run / Ready status
    'auto_manual',               # Auto / Manual mode
    'at_stop_start',             # AT stop / start
    'lsp_rsp'                    # LSP / RSP selection
]

INSTRUMENTAL_STATUS_1 = [
    'run_ready',        # RUN/READY
    'auto_manual',      # AUTO/MANUAL
    'at_stop_start',    # AT stop/start
    'lsp_rsp',          # LSP/RSP
    'pv',               # Process Value
    'sp',               # SP (Target value)
    'mv'                # Manipulated Variable (MV)
]

def table_alarms(row_index=0,min_key_width=3, min_val_width=1.5):
    with open('test_200_samples.csv', 'r', encoding='utf-8-sig') as f:
        reader = list(csv.DictReader(f))
        if row_index >= len(reader):
            raise IndexError("row does not exist in CSV")
        row = reader[row_index]
    ## find variable for alarms
    alarm_columns = [h for h in CSV_HEADERS if "alarm" in h.lower()]

    ## Alarms: True False
    table_data = []
    for key in alarm_columns:
        value = row[key]
        occurred = 'Yes' if str(value) in ('1', 'True', 'true') else 'No'
        table_data.append([key, occurred])
    
    ## dynamical columns
    font_name = 'Helvetica'
    font_size = 10

    key_widths = [max(stringWidth(r[0], font_name, font_size)/28.35, min_key_width) for r in table_data]
    val_widths = [max(stringWidth(r[1], font_name, font_size)/28.35, min_val_width) for r in table_data]

    col_widths = [max(key_widths)*cm, max(val_widths)*cm]

    ## Table
    table = Table(table_data, colWidths=col_widths)
    table.setStyle(TableStyle([
        ('ALIGN', (0,0), (-1,-1), 'LEFT'),
        ('FONTNAME', (0,0), (-1,-1), 'Helvetica'),
        ('FONTSIZE', (0,0), (-1,-1), 9),
        ('GRID', (0,0), (-1,-1), 0.3, colors.grey),
        ('VALIGN', (0,0), (-1,-1), 'TOP')
    ]))
    return table


def table_2_params(row_index=0, rows_per_col=6, min_key_width=2.5, min_val_width=1.5):
    with open('test_200_samples.csv', 'r', encoding='utf-8-sig') as f:
        reader = list(csv.DictReader(f))
        if row_index >= len(reader):
            raise IndexError("row does not exist in CSV")
        row = reader[row_index]

    keys = CSV_HEADERS
    font_name = 'Helvetica'
    font_size = 10
    page_width, _ = A4
    left_margin = right_margin = 2*cm  # możesz zmienić marginesy
    usable_width = page_width - left_margin - right_margin

    # lista par [key, value]
    kv_list = [[k, str(row[k])] for k in keys]
    num_vars = len(kv_list)

    # dynamiczna szerokość dla każdej nazwy i wartości
    key_widths = [max(stringWidth(kv[0], font_name, font_size)/28.35, min_key_width) for kv in kv_list]
    val_widths = [max(stringWidth(kv[1], font_name, font_size)/28.35, min_val_width) for kv in kv_list]

    # oblicz liczbę kolumn tak, żeby zmieściły się w szerokości strony
    col_pairs_widths = [(kw + vw) for kw, vw in zip(key_widths, val_widths)]
    max_pair_width = max(col_pairs_widths)
    max_cols = int(usable_width // (max_pair_width*cm))
    num_cols = min(max_cols, (num_vars + rows_per_col - 1) // rows_per_col)
    if num_cols < 1:
        num_cols = 1

    # liczba wierszy w kolumnie
    rows_per_col_actual = (num_vars + num_cols - 1) // num_cols

    # budowanie tabeli
    table_data = []
    for i in range(rows_per_col_actual):
        row_data = []
        for c in range(num_cols):
            idx = c*rows_per_col_actual + i
            if idx < num_vars:
                row_data.extend(kv_list[idx])
            else:
                row_data.extend(['',''])
        table_data.append(row_data)

    # szerokości kolumn dla Table
    col_widths = []
    for c in range(num_cols):
        idx_start = c*rows_per_col_actual
        idx_end = min(idx_start + rows_per_col_actual, num_vars)
        kw = max(key_widths[idx_start:idx_end])
        vw = max(val_widths[idx_start:idx_end])
        col_widths.extend([kw*cm, vw*cm])

    table = Table(table_data, colWidths=col_widths)
    table.setStyle(TableStyle([
        ('ALIGN', (0,0), (-1,-1), 'LEFT'),
        ('FONTNAME', (0,0), (-1,-1), 'Helvetica'),
        ('FONTSIZE', (0,0), (-1,-1), 9),
        ('GRID', (0,0), (-1,-1), 0.3, colors.grey),
        ('VALIGN', (0,0), (-1,-1), 'TOP')
    ]))
    return table


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
    ## data init
    timestamps, pv_lo_max_values, alarms = [], [], []
    error_e1_min, error_e1_max = [], []

    with open(OUTPUT_CSV_TEST, 'r', encoding='utf-8-sig') as f:
        reader = csv.DictReader(f)
        for row in reader:
            timestamps.append(datetime.strptime(row['timestamp'], '%Y-%m-%d %H:%M:%S'))
            pv_lo_max_values.append(float(row['pv_lo_max']))
            alarms.append(int(row["alarm"]))
            error_e1_min.append(float(row["error_e1_min"]))
            error_e1_max.append(float(row["error_e1_max"]))

    ## gen plots
    fig, ax = plt.subplots(figsize=(9,4))
    report2_plot(ax, timestamps, pv_lo_max_values, alarms, error_e1_min[0], error_e1_max[0])
    plot_filename = "report2_plot.png"
    fig.savefig(plot_filename, bbox_inches='tight')
    plt.close(fig)

    ## PDF by platypus
    pdf_file = "Report2_5min.pdf"
    doc = SimpleDocTemplate(
        pdf_file,
        pagesize=A4,
        leftMargin=50,
        rightMargin=50,
        topMargin=50,
        bottomMargin=50)
    
    ## add logo
    width, height = A4
    def draw_header_logo(canvas: Canvas, doc):
        logo_width = 50
        logo_height = 50
        x = width - 50 - logo_width
        y = height - 50 - logo_height + 10

        draw_right_logo = logo_file
        canvas.drawImage(
            draw_right_logo,
            x,
            y,
            width=logo_width,
            height=logo_height
        )

        draw_left_logo = logo_file2
        x_left = 50

        canvas.drawImage(
            draw_left_logo,
            x_left,
            y,
            width=logo_width,
            height=logo_height
        )
        # Optional: Title & header
        canvas.setFont("Helvetica-Bold", 12)
        canvas.drawString(50, height - 50 + 15, "Project: Modbus Temperature Control Project")
        
    # Frame & PageTemplate
    frame = Frame(
        doc.leftMargin, doc.bottomMargin,
        width - doc.leftMargin - doc.rightMargin,
        height - doc.topMargin - doc.bottomMargin,
        id = 'normal_frame'
    )
    
    template = PageTemplate(
        id="normal",
        frames=[frame],
        onPage=draw_header_logo
    )
    doc.addPageTemplates([template])
    
    ## Styles
    styles = getSampleStyleSheet()
    styles.add(ParagraphStyle(name="SectionHeader",
                              fontSize=14,
                              leading=18,
                              spaceAfter=10,
                              spaceBefore=20,
                              fontName="Helvetica-Bold"
                              ))
   
    ## content
    content = []
    content.append(Paragraph("Report: TEMPLATE", styles["Title"]))
    content.append(Paragraph("Summary - read from test/SDC35", styles["Heading2"]))
    content.append(Paragraph(f"Date: {datetime.now().strftime('%Y-%m-%d')} | Report 2 | receive data for 5 min - test", styles["BodyText"]))
    content.append(Paragraph("Autor: Paweł Ozga", styles["BodyText"]))

    content.append(Paragraph("Process Parameters", styles["SectionHeader"]))
    content.append(table_2_params(row_index=0, rows_per_col=6))
    content.append(Paragraph("Alarm Status", styles["SectionHeader"]))
    content.append(table_alarms(row_index=0,min_key_width=3, min_val_width=1.5))
    content.append(Spacer(1, 2))

    ## function -add section
    def add_section(title, text):
        content.append(Paragraph(title, styles["SectionHeader"]))
        content.append(Paragraph(text, styles["BodyText"]))
        content.append(Spacer(1, 2))

    Summary_text = "The process ran normally for 5 minutes. Minor deviations in PV were observed. No critical alarms occurred."
    add_section("Summary", Summary_text)
    Alarms_text = "Słowo3  Słowo4"
    add_section("Alarms: Error handling", Alarms_text)

    ## Figure
    content.append(Spacer(1, 2))
    content.append(Paragraph("Plot", styles["SectionHeader"]))
    img = Image(plot_filename, width=16*cm, height=6*cm)
    content.append(img)

    ## gen PDF
    doc.build(content)
    print(f"PDF saved as {pdf_file}")
     
def report2_plot(ax, timestamps, pv_lo_max_values, alarms, error_min, error_max):
    ax.plot(timestamps, pv_lo_max_values, color='blue', linestyle='-', label='PV')
    error_min = 10
    error_max = 15
    ax.axhline(y=error_min, linestyle='--', color='darkblue', label='Error Min')
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
        ax.scatter(timestamps[i], y, color=point_color, s=2)

    ax.set_ylim(0,32)
    ax.set_title('Report2 Process Template ')
    ax.set_xlabel('Time [min]')
    ax.set_ylabel('Temp [°C] | Error')
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

report2_to_git = os.path.join(BASE_DIR, "Report2_5min.pdf")

def push_report_to_github(report2_to_git):
    if not os.path.exists(report2_to_git):
        print(f"{report2_to_git} does not exist, cannot push to GitHub.")
        return
    
    try:
        subprocess.run(["git", "add", report2_to_git], cwd=BASE_DIR, check=True)

        subprocess.run(["git", "commit", "-m", f"Update report {os.path.basename(report2_to_git)}"], cwd=BASE_DIR, check=True)

        subprocess.run(["git", "push", "origin", "main"], cwd=BASE_DIR, check=True)
        print(f"{report2_to_git} pushed to Github succesfully.")

    except subprocess.CalledProcessError as e:
        print("Error while pushing to Github:", e)


def helper():
    print("command: exit, report1, report2, test, -help ")
    print(""" 
        MODBUS TEMPERATURE CONTEROL PROJECT
          
        This script can generate 3 report:
        - report 1: read current configuration and data,
        - report 2: receive data for 5 min,
        - report 3: receive data for 24 h

        # each report generate one pdf file on github !
          
        run:
          report1,
          report2,
          report3,
          exit,
          help
          """)
    
def main():
    csvfile = open(OUTPUT_CSV_TEST_RESULT, 'w', newline='')
    csv_writer = csv.writer(csvfile)
    if csvfile.tell() == 0:
        csv_writer.writerow(CSV_HEADERS)

        try:
            while True:
                
                line = input("Enter command or -help: ")

                if line.lower() == 'report1':
                    print("report1")
                elif line.lower() == 'report2':
                    print("report2 - TEMPLATE\n")
                    report2_generate()
                    #push_report_to_github()
                elif line.lower() == 'report3':
                    print("report3")
                elif line.lower() == 'test':
                    #test()
                    print("test")
                elif line.lower() == '-help':
                    print("helper")
                    helper()
                elif line.lower() == 'exit':
                    break
                else:
                    print("unknow command: ",line)

        except KeyboardInterrupt:
            print('Exit')
            csvfile.close()

if __name__ == '__main__':
    main()