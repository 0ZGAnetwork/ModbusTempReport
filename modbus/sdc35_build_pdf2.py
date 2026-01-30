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
report2_to_git = os.path.join(BASE_DIR, "Report2.pdf")
graph = os.path.join(BASE_DIR, "report2_plot.png")
#print(f"Report2 to git: {report2_to_git}") # debug
folder = "./" # find all files
files = glob.glob(os.path.join(folder, "sdc35_report2_*.csv"))
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
def generate_report2_pdf(plot_file, logo_left, logo_right, table_funcs=[]):
    #--create plot
    timestamps = []
    pv_values = []

    with open(latest_file, "r", encoding="utf-8", errors="ignore") as f:
        lines = f.readlines()

    for line in lines:
        cols = line.strip().split(",")
        if len(cols) < 2:
            continue
        try:
            # kolumna 0 = czas w ms, kolumna 1 = PV
            timestamps.append(int(cols[0]) / 1000)  # z ms na s
            pv_values.append(float(cols[1]))
        except:
            continue

    if not pv_values:
        print("No PV data found.")
        exit()

    # --- rysuj wykres ---
    plt.figure(figsize=(10, 5))
    plt.plot(timestamps, pv_values, marker='o', linestyle='-', color='red', label="PV")
    plt.xlabel("Time [s]")
    plt.ylabel("PV - Process Variable")
    plt.title("PV over time")
    plt.grid(True)
    plt.legend()
    plt.tight_layout()

    plot_file = graph
    plt.savefig(plot_file)
    plt.close()
    print(f"Plot saved as {plot_file}")
    #--end plot
    pdf_file = "Report2.pdf"
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
    #--- plot ---
    plt.figure(figsize=(10, 5))
    plt.plot(timestamps, pv_values, marker='o', linestyle='-', color='red', label="PV")
    plt.xlabel("Time [s]")
    plt.ylabel("PV - Process Variable")
    plt.title("PV over time")
    plt.grid(True)
    plt.legend()
    plt.tight_layout()

    # Zapis do pliku PNG, żeby potem wrzucić do PDF
    plot_file = "report2_plot.png"
    plt.savefig(plot_file)
    plt.close()

    print(f"Plot saved as {plot_file}")

    # --- Styles and content ---
    styles = getSampleStyleSheet()
    styles.add(ParagraphStyle(name="SectionHeader", fontSize=14, leading=18,
                              spaceAfter=10, spaceBefore=20, fontName="Helvetica-Bold"))
    content = []
    content.append(Paragraph("Report2: Process Control", styles["Title"]))
    content.append(Paragraph(f"Date: {datetime.now().strftime('%Y-%m-%d')} | Report 2 | Measure the temperature of the PV process for 1 minute", styles["BodyText"]))
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
        content.append(PageBreak())
        content.append(Spacer(1, 56)) 
        content.append(Paragraph("Plot", styles["SectionHeader"]))
        img = Image(plot_file, width=16*cm, height=6*cm)
        content.append(img)
    # --- build PDF ---
    doc.build(content)
    print(f"PDF saved as {pdf_file}")

generate_report2_pdf(
    plot_file="report2_plot.png",
    logo_left=logo_file,
    logo_right=logo_file2,
    # table_funcs=[
    #     lambda: table_2_params(row_index=0, rows_per_col=6),
    #     lambda: table_alarms(row_index=0)
    # ]
)