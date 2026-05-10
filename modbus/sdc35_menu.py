import subprocess
import os
import time
import csv

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
report1_to_git = os.path.join(BASE_DIR, "Report1.pdf")
report2_to_git = os.path.join(BASE_DIR, "Report2.pdf")
report3_to_git = os.path.join(BASE_DIR, "Report3.pdf")

def helper():
    print("command: exit, report1, report2, test, -help ")
    print(""" 
MODBUS TEMPERATURE CONTROL PROJECT
    
This script can generate 3 report:
- report 1: read current configuration and data,
- report 2: receive data for 5 min,
- report 3: receive data with timestamp

# each report generates one PDF file on GitHub!
""")

def push_report_to_github(file_to_git):
    if not os.path.exists(file_to_git):
        print(f"{file_to_git} does not exist, cannot push to GitHub.")
        return
    try:
        subprocess.run(["git", "add", file_to_git], cwd=BASE_DIR, check=True)
        subprocess.run(["git", "commit", "-m", f"Update report {os.path.basename(file_to_git)}"], cwd=BASE_DIR, check=True)
        subprocess.run(["git", "push", "origin", "HEAD:main"], cwd=BASE_DIR, check=True)
        print(f"{file_to_git} pushed to Github successfully.")
    except subprocess.CalledProcessError as e:
        print("Error while pushing to Github:", e)

def run_report1():
    print("Generating CSV for Report1...")
    subprocess.run(["python3", "sdc35_report1.py"], check=True)
    time.sleep(2)
    print("Building PDF for Report1...")
    subprocess.run(["python3", "sdc35_build_pdf1.py"], check=True)
    if os.path.exists(report1_to_git):
        print("Pushing Report1 to GitHub...")
        push_report_to_github(report1_to_git)
    else:
        print(f"{report1_to_git} not found, cannot push.")
    

def run_report2():
    print("Generating CSV for Report2...")
    subprocess.run(["python3", "sdc35_report2.py"], check=True)
    time.sleep(2)
    print("Building PDF for Report2...")
    subprocess.run(["python3", "sdc35_build_pdf2.py"], check=True)
    if os.path.exists(report2_to_git):
        print("Pushing Report2 to GitHub...")
        push_report_to_github(report2_to_git)
    else:
        print(f"{report2_to_git} not found, cannot push.")

def run_report3():
    print("Generating CSV for Report3...")
    subprocess.run(["python3", "sdc35_report3.py"], check=True)
    time.sleep(2)
    print("Building PDF for Report3...")
    subprocess.run(["python3", "sdc35_build_pdf3.py"], check=True)
    if os.path.exists(report3_to_git):
        print("Pushing Report3 to GitHub...")
        push_report_to_github(report3_to_git)
    else:
        print(f"{report3_to_git} not found, cannot push.")

def main():
    genCSV_result = {}

    while True:
        print("\n=== Modbus Report Generator ===")
        print("1. Generate Report 1")
        print("2. Generate Report 2")
        print("3. Generate Report 3")
        print("Type 'help' or '-help' for assistance")
        print("4. Exit")
        choice = input("Select an option: ")
        
        if choice == "1":
            start = time.perf_counter()
            run_report1()
            # time.sleep(2)
            end = time.perf_counter()
            genCSV_result["genCSV_report1"] = round(end - start, 6)
        elif choice == "2":
            start = time.perf_counter()
            run_report2()
            # time.sleep(2)
            end = time.perf_counter()
            genCSV_result["genCSV_report2"] = round(end - start, 6)
        elif choice == "3":
            start = time.perf_counter()
            run_report3()
            # time.sleep(2)
            end = time.perf_counter()
            genCSV_result["genCSV_report3"] = round(end - start, 6)
        elif choice in ["help", "-help"]:
            helper()
        elif choice == "4":
            print("Exiting...")
            break
        else:
            print("Invalid choice, try again.")

        file_exists = os.path.isfile("time_verification.csv")
        with open("time_verification.csv", "a", newline="") as f:
            writer = csv.writer(f)
            if not file_exists:
                writer.writerow(["function", "time"])
            for name, t in genCSV_result.items():
                # print("writing:", name, t)
                writer.writerow([name, t])
        

if __name__ == "__main__":
    main()