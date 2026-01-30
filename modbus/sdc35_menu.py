import subprocess
import os
import time

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
report1_to_git = os.path.join(BASE_DIR, "Report1.pdf")
report2_to_git = os.path.join(BASE_DIR, "Report2.pdf")

def helper():
    print("command: exit, report1, report2, test, -help ")
    print(""" 
MODBUS TEMPERATURE CONTROL PROJECT
    
This script can generate 3 report:
- report 1: read current configuration and data,
- report 2: receive data for 5 min,
- report 3: receive data for 24 h

# each report generates one PDF file on GitHub!
""")

def push_report_to_github(file_to_git):
    if not os.path.exists(file_to_git):
        print(f"{file_to_git} does not exist, cannot push to GitHub.")
        return
    try:
        subprocess.run(["git", "add", file_to_git], cwd=BASE_DIR, check=True)
        subprocess.run(["git", "commit", "-m", f"Update report {os.path.basename(file_to_git)}"], cwd=BASE_DIR, check=True)
        subprocess.run(["git", "push", "report_remote", "HEAD:main"], cwd=BASE_DIR, check=True)
        print(f"{file_to_git} pushed to Github successfully.")
    except subprocess.CalledProcessError as e:
        print("Error while pushing to Github:", e)

def run_report1():
    print("Generating CSV for Report 1...")
    subprocess.run(["python3", "sdc35_report1.py"], check=True)

    time.sleep(2)
    print("Building PDF for Report 1...")
    subprocess.run(["python3", "sdc35_build_pdf1.py"], check=True)
    if os.path.exists(report1_to_git):
        print("Pushing Report 1 to GitHub...")
        push_report_to_github(report1_to_git)
    else:
        print(f"{report1_to_git} not found, cannot push.")

def run_report2():
    print("Generating CSV for Report 2...")
    subprocess.run(["python3", "sdc35_report2.py"], check=True)

    time.sleep(2)
    print("Building PDF for Report 2...")
    subprocess.run(["python3", "sdc35_build_pdf2.py"], check=True)
    if os.path.exists(report2_to_git):
        print("Pushing Report 2 to GitHub...")
        push_report_to_github(report2_to_git)
    else:
        print(f"{report2_to_git} not found, cannot push.")

def main():
    while True:
        print("\n=== Modbus Report Generator ===")
        print("1. Generate Report 1")
        print("2. Generate Report 2")
        print("Type 'help' or '-help' for assistance")
        print("3. Exit")
        choice = input("Select an option: ")
        
        if choice == "1":
            run_report1()
        elif choice == "2":
            #print("Report 2 generation is currently disabled.")
            run_report2()
        elif choice in ["help", "-help"]:
            helper()
        elif choice == "3":
            print("Exiting...")
            break
        else:
            print("Invalid choice, try again.")

if __name__ == "__main__":
    main()