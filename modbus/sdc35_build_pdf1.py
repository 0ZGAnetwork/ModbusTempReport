# """
# This script generate 3 report:
# - report 1: read current configuration and data,
# - report 2: receive data for 1 min - initial, test communication of process system,
# - report 3: receive data for specified process

# all report generate one pdf file on github !
# """
import serial                                        # seriall communication
import time
import csv
import os
import glob
import math
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
import subprocess    

# open current folder and find latest csv file
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
report1_to_git = os.path.join(BASE_DIR, "Report1.pdf")
#print(f"Report1 to git: {report1_to_git}") # debug
folder = "./" # find all files
files = glob.glob(os.path.join(folder, "sdc35_report1_*.csv"))
logo_file = os.path.join(BASE_DIR, "MTCPreports.jpg")
logo_file2 = os.path.join(BASE_DIR, "PT100.jpg")

if not files:
    print("Files not found.")
    exit()

files.sort(key=os.path.getmtime, reverse=True)
latest_file = files[0]
#print(f"Latest file: {latest_file}") # debugg

header_names = [
    "time_ms",
    "pv","sv","lsp","pid_group",
    "pv_lo_max","pv_hi_max","sv_lo_max","sv_hi_max",
    "pv_input_type","pv_lo_limit","pv_hi_limit","sv_lo_limit","sv_hi_limit",
    "rsp_input_type","rsp_lo_limit","rsp_hi_limit","decimal_point",
    "control_action","output_at_pv_alarm","output_operation_at_pv_alarm","heat_cool_control",
    "lsp_system_group","preset_manual_value","pid_output_mode","zone_pid_operation",
    "aux_output_range","aux_type","aux_scale_low","aux_scale_high","aux_mv_scale",
    "position_proportional_type","motor_auto_adjust",
    "comm_type","station_addr","tx_speed","data_length","parity","stop_bits","user_level",
    "control_method","differential","bank_type",
    "alarm_typical","alarm_D0","alarm_D1","alarms_status",
    "alarm_pv_over","alarm_pv_under","alarm_cj_burnout","alarm_rsp_over",
    "alarm_mfb_burnout","alarm_motor_fail","alarm_ct_over",
    "alarm_pv_failure","alarm_hardware_failure","alarm_parameter_failure",
    "alarm_adjustment_failure","alarm_rom_failure",
    "timestamp","unknow1","unknow2"
]
OPERATION_DISPLAY_KEYS = [
    "pv","sv","lsp","pid_group"
]
OPERATION_DISPLAY_DESCRIPTIONS = {
    "pv": "Process Variable",
    "sv": "Setpoint",
    "lsp": "1 to 8 – Local Set Point",
    "pid_group": "1 to 8 – Operation Process Group"
}
MODBUS_KEYS = [
    "comm_type", "station_addr", "tx_speed", "data_length",
    "parity", "stop_bits"
]
MODBUS_DESCRIPTIONS = {
    "comm_type": {
        0: "CPL",
        1: "Modbus (ASCII format)",
        2: "Modbus (RTU format)"
    },
    "station_addr": lambda v: str(v) if v is not None else "",  # 0-127, po prostu liczba
    "tx_speed": {
        0: "4800 bps",
        1: "9600 bps",
        2: "19200 bps",
        3: "38400 bps"
    },
    "data_length": {
        0: "7 bits",
        1: "8 bits"
    },
    "parity": {
        0: "Even parity",
        1: "Odd parity",
        2: "No parity"
    },
    "stop_bits": {
        0: "1 bit",
        1: "2 bits"
    }
}
ALARM_KEYS = [
    "alarm_typical", "alarm_D0", "alarm_D1", "alarms_status",
    "alarm_pv_over", "alarm_pv_under", "alarm_cj_burnout", "alarm_rsp_over",
    "alarm_mfb_burnout", "alarm_motor_fail", "alarm_ct_over", "alarm_pv_failure",
    "alarm_hardware_failure", "alarm_parameter_failure", "alarm_adjustment_failure", "alarm_rom_failure",
]

styles = getSampleStyleSheet()

styles.add(ParagraphStyle(
    name="SectionHeader",
    fontSize=14,
    leading=18,
    spaceAfter=10,
    spaceBefore=20,
    fontName="Helvetica-Bold"
))
table_counter = 1
graph_counter = 1
data_list = []
with open(latest_file, "r", encoding="utf-8", errors="ignore") as f:
    lines = f.readlines()
print(f"Header columns expected: {len(header_names)}")
for i, line in enumerate(lines): 
    row = line.strip().split(",")
    if len(row) < 2:
        print(f"Skipping line {i+1}: too few columns")
        continue
    row_without_last = row[:-1]
    print(f"Line {i+1} full row (without last column): {row_without_last}")
    if len(row_without_last) != len(header_names) - 1:
        print(f"Skipping line {i+1}: unexpected number of columns ({len(row_without_last)})")
        continue
    row_dict = {}
    for key, value in zip(header_names[:-1], row_without_last):
        value = value.strip()
        if value.lower() == 'nan' or value == '':
            row_dict[key] = None
        else:
            try:
                if '.' in value:
                    row_dict[key] = float(value)
                else:
                    row_dict[key] = int(value)
            except:
                row_dict[key] = value
    data_list.append(row_dict)
    
    if not data_list:
        print(f"No valid data found in {latest_file}. Aborting PDF generation.")
        exit()
    row = data_list[0]
    CSV_HEADERS = list(row.keys())

#print(f"Loaded {len(data_list)} records")
#if data_list:
 #   print("Example:", data_list[0])
# table = [(key, row.get(key)) for key in OPERATION_DISPLAY_KEYS]
# for name, value in table:
#     print(f"{name:25} | {value}")

def operation_display_table():
    global table_counter
    elements = []
    title = f"Table {table_counter}. Operational Display Parameters"
    elements.append(Paragraph(title, styles["SectionHeader"]))
    table_counter += 1
    data = [["Register", "Value"]]
    for key in OPERATION_DISPLAY_KEYS:
        value = row.get(key)
        data.append([key, "" if value is None else str(value)])
    tbl = Table(data, colWidths=[7*cm, 7*cm])
    tbl.setStyle(TableStyle([
        ("BACKGROUND", (0, 0), (-1, 0), colors.lightgrey),
        ("GRID", (0, 0), (-1, -1), 0.5, colors.grey),
        ("FONT", (0, 0), (-1, 0), "Helvetica-Bold"),
        ("ALIGN", (1, 1), (-1, -1), "CENTER"),
        ("VALIGN", (0, 0), (-1, -1), "MIDDLE"),
        ("BOTTOMPADDING", (0, 0), (-1, 0), 8),
    ]))
    elements.append(tbl)
    return elements

def operation_display_table_with_description():
    global table_counter
    elements = []
    title = f"Table {table_counter}. Operational Display Parameters"
    elements.append(Paragraph(title, styles["SectionHeader"]))
    table_counter += 1
    data = [["Register", "Value", "Description"]]
    for key in OPERATION_DISPLAY_KEYS:
        value = row.get(key)
        description = OPERATION_DISPLAY_DESCRIPTIONS.get(key, "")
        data.append([key, "" if value is None else str(value), description])
    tbl = Table(data, colWidths=[5*cm, 3*cm, 6*cm])
    tbl.setStyle(TableStyle([
        ("BACKGROUND", (0, 0), (-1, 0), colors.lightgrey),
        ("GRID", (0, 0), (-1, -1), 0.5, colors.grey),
        ("FONT", (0, 0), (-1, 0), "Helvetica-Bold"),
        ("ALIGN", (1, 1), (-1, -1), "CENTER"),
        ("VALIGN", (0, 0), (-1, -1), "MIDDLE"),
        ("BOTTOMPADDING", (0, 0), (-1, 0), 8),
    ]))
    elements.append(tbl)
    return elements

def modbus_table():
    global table_counter
    elements = []
    title = f"Table {table_counter}. Modbus Communication Parameters"
    elements.append(Paragraph(title, styles["SectionHeader"]))
    table_counter += 1
    data = [["Register", "Value"]]
    for key in MODBUS_KEYS:
        value = row.get(key)
        data.append([key, "" if value is None else str(value)])
    tbl = Table(data, colWidths=[7*cm, 7*cm])
    tbl.setStyle(TableStyle([
        ("BACKGROUND", (0, 0), (-1, 0), colors.lightgrey),
        ("GRID", (0, 0), (-1, -1), 0.5, colors.grey),
        ("FONT", (0, 0), (-1, 0), "Helvetica-Bold"),
        ("ALIGN", (1, 1), (-1, -1), "CENTER"),
        ("VALIGN", (0, 0), (-1, -1), "MIDDLE"),
        ("BOTTOMPADDING", (0, 0), (-1, 0), 8),
    ]))
    elements.append(tbl)
    return elements

def modbus_table_with_description():
    global table_counter
    elements = []
    title = f"Table {table_counter}. Modbus Communication Parameters"
    elements.append(Paragraph(title, styles["SectionHeader"]))
    table_counter += 1
    data = [["Register", "Value", "Description"]]
    for key in MODBUS_KEYS:
        value = row.get(key)
        if key in MODBUS_DESCRIPTIONS and value is not None:
            desc_map = MODBUS_DESCRIPTIONS[key]
            if callable(desc_map):
                description = desc_map(value)
            else:
                description = desc_map.get(int(value), "Unknown")
        else:
            description = ""
        data.append([key, "" if value is None else str(value), description])
    tbl = Table(data, colWidths=[5*cm, 3*cm, 6*cm])
    tbl.setStyle(TableStyle([
        ("BACKGROUND", (0, 0), (-1, 0), colors.lightgrey),
        ("GRID", (0, 0), (-1, -1), 0.5, colors.grey),
        ("FONT", (0, 0), (-1, 0), "Helvetica-Bold"),
        ("ALIGN", (1, 1), (-1, -1), "CENTER"),
        ("VALIGN", (0, 0), (-1, -1), "MIDDLE"),
        ("BOTTOMPADDING", (0, 0), (-1, 0), 8),
    ]))
    elements.append(tbl)
    return elements

def sensor_table():
    global table_counter
    elements = []
    title = f"Table {table_counter}. Sensor Parameters (Register 51)"
    elements.append(Paragraph(title, styles["SectionHeader"]))
    table_counter += 1
    data = [["Register", "Value"]]
    register_name = "pv_input_type"  
    value = row.get(register_name)
    data.append([register_name, "" if value is None else str(value)])
    tbl = Table(data, colWidths=[7*cm, 7*cm])
    tbl.setStyle(TableStyle([
        ("BACKGROUND", (0, 0), (-1, 0), colors.lightgrey),
        ("GRID", (0, 0), (-1, -1), 0.5, colors.grey),
        ("FONT", (0, 0), (-1, 0), "Helvetica-Bold"),
        ("ALIGN", (1, 1), (-1, -1), "RIGHT"),
        ("VALIGN", (0, 0), (-1, -1), "MIDDLE"),
        ("BOTTOMPADDING", (0, 0), (-1, 0), 8),
    ]))
    elements.append(tbl)
    return elements

def sensor_table_with_description():
    global table_counter
    elements = []
    title = f"Table {table_counter}. Sensor Parameters"
    elements.append(Paragraph(title, styles["SectionHeader"]))
    table_counter += 1
    data = [["Register", "Value", "Description"]]
    register_name = "pv_input_type"
    value = row.get(register_name)
    description = "PT100: (-50+200 °C / -50+400 K)"  
    data.append([register_name, "" if value is None else str(value), description])
    tbl = Table(data, colWidths=[5*cm, 3*cm, 6*cm])
    tbl.setStyle(TableStyle([
        ("BACKGROUND", (0, 0), (-1, 0), colors.lightgrey),
        ("GRID", (0, 0), (-1, -1), 0.5, colors.grey),
        ("FONT", (0, 0), (-1, 0), "Helvetica-Bold"),
        ("ALIGN", (1, 1), (-1, -1), "CENTER"),
        ("VALIGN", (0, 0), (-1, -1), "MIDDLE"),
        ("BOTTOMPADDING", (0, 0), (-1, 0), 8),
    ]))
    elements.append(tbl)
    return elements

## --- Generate report ---
def generate_report1_pdf(plot_file, logo_left, logo_right, table_funcs=[]):
    pdf_file = "Report1.pdf"
    width, height = A4
    # --- Document setup ---
    def draw_header(c: canvas.Canvas, doc):
        logo_width = 50
        logo_height = 50
        if os.path.exists(logo_left):
            c.drawImage(logo_left, 50, height - 50 - logo_height + 10,
                        width=logo_width, height=logo_height)
        if os.path.exists(logo_right):
            c.drawImage(logo_right, width - 50 - logo_width, height - 50 - logo_height + 10,
                        width=logo_width, height=logo_height)
        c.setFont("Helvetica-Bold", 12)
        c.drawCentredString(width / 2, height - 50 + 15, "Project: Modbus Temperature Control Project")
    doc = BaseDocTemplate(pdf_file, pagesize=A4,
                          leftMargin=50, rightMargin=50,
                          topMargin=50, bottomMargin=50)
    frame = Frame(doc.leftMargin, doc.bottomMargin,
                  width - doc.leftMargin - doc.rightMargin,
                  height - doc.topMargin - doc.bottomMargin,
                  id='normal_frame')
    template = PageTemplate(id="normal", frames=[frame], onPage=draw_header)
    doc.addPageTemplates([template])
    # --- Styles and content ---
    styles = getSampleStyleSheet()
    styles.add(ParagraphStyle(name="SectionHeader", fontSize=14, leading=18,
                              spaceAfter=10, spaceBefore=20, fontName="Helvetica-Bold"))
    content = []
    content.append(Paragraph("Report1: Status Check", styles["Title"]))
    content.append(Paragraph(f"Date: {datetime.now().strftime('%Y-%m-%d')} | Report 1 | Status Check - parameter, communication", styles["BodyText"]))
    content.append(Paragraph("Autor: Pawel Ozga index:266078", styles["BodyText"]))
    content.append(Spacer(1, 12))
    table_funcs = [operation_display_table_with_description, modbus_table_with_description, sensor_table_with_description]
    for table_func in table_funcs:
        elems = table_func()
        if elems:
            for el in elems:
                content.append(el)
            content.append(Spacer(1, 12))
    if os.path.exists(plot_file):
        content.append(Paragraph("Plot", styles["SectionHeader"]))
        img = Image(plot_file, width=16*cm, height=6*cm)
        content.append(img)
    # --- build PDF ---
    doc.build(content)
    print(f"PDF saved as {pdf_file}")

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
    left_margin = right_margin = 2 * cm
    usable_width = page_width - left_margin - right_margin

    kv_list = [[k, str(row.get(k, ""))] for k in keys]
    num_vars = len(kv_list)

    key_widths = [
        max(stringWidth(k, font_name, font_size) / 28.35, min_key_width)
        for k, _ in kv_list
    ]
    val_widths = [
        max(stringWidth(v, font_name, font_size) / 28.35, min_val_width)
        for _, v in kv_list
    ]

    col_pair_widths = [kw + vw for kw, vw in zip(key_widths, val_widths)]
    max_pair_width = max(col_pair_widths)

    max_cols = int(usable_width // (max_pair_width * cm))
    num_cols = min(max_cols, (num_vars + rows_per_col - 1) // rows_per_col)
    num_cols = max(num_cols, 1)

    rows_per_col_actual = (num_vars + num_cols - 1) // num_cols

    table_data = []
    for i in range(rows_per_col_actual):
        row_data = []
        for c in range(num_cols):
            idx = c * rows_per_col_actual + i
            if idx < num_vars:
                row_data.extend(kv_list[idx])
            else:
                row_data.extend(["", ""])
        table_data.append(row_data)

    col_widths = []
    for c in range(num_cols):
        idx_start = c * rows_per_col_actual
        idx_end = min(idx_start + rows_per_col_actual, num_vars)
        col_widths.extend([
            max(key_widths[idx_start:idx_end]) * cm,
            max(val_widths[idx_start:idx_end]) * cm
        ])

    table = Table(table_data, colWidths=col_widths)
    table.setStyle(TableStyle([
        ('ALIGN', (0, 0), (-1, -1), 'LEFT'),
        ('FONTNAME', (0, 0), (-1, -1), 'Helvetica'),
        ('FONTSIZE', (0, 0), (-1, -1), 9),
        ('GRID', (0, 0), (-1, -1), 0.3, colors.grey),
        ('VALIGN', (0, 0), (-1, -1), 'TOP')
    ]))

    return table

generate_report1_pdf(
    plot_file="report1_plot.png",
    logo_left=logo_file,
    logo_right=logo_file2,
    # table_funcs=[
    #     lambda: table_2_params(row_index=0, rows_per_col=6),
    #     lambda: table_alarms(row_index=0)
    # ]
)

def push_report_to_github(report1_to_git):
    if not os.path.exists(report1_to_git):
        print(f"{report1_to_git} does not exist, cannot push to GitHub.")
        return
    try:
        subprocess.run(["git", "add", report1_to_git], cwd=BASE_DIR, check=True)

        subprocess.run(["git", "commit", "-m", f"Update report {os.path.basename(report1_to_git)}"], cwd=BASE_DIR, check=True)

        subprocess.run(["git", "push", "origin", "main"], cwd=BASE_DIR, check=True)
        print(f"{report1_to_git} pushed to Github succesfully.")

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