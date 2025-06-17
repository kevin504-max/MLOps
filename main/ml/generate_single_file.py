import serial
import time
import os
import csv
from datetime import datetime

port = '/dev/ttyUSB0'
baudrate = 115200
timeout = 5

def extract_csv_block(raw_output):
    start_marker = "<<<BEGIN>>>"
    end_marker = "<<<END>>>"
    start = raw_output.find(start_marker)
    end = raw_output.find(end_marker)
    if start != -1 and end != -1 and end > start:
        return raw_output[start + len(start_marker):end].strip()
    return None

def send_command(ser, command, timeout=5):
    ser.reset_input_buffer()
    print(f"📤 Sending command: {command}")
    ser.write((command + '\n').encode())

    response = b''
    start_time = time.time()

    while time.time() - start_time < timeout:
        if ser.in_waiting > 0:
            response += ser.read(ser.in_waiting)
            if b'<<<END>>>' in response:
                break
        time.sleep(0.1)

    return response.decode(errors='ignore')

def save_csv_file(content, filename):
    try:
        lines = content.strip().splitlines()
        if not lines:
            print(f"⚠️ No CSV data to save in {filename}")
            return False

        header = lines[0].split(",")
        rows = [line.split(",") for line in lines[1:]]
        valid_rows = [row for row in rows if len(row) == len(header)]

        with open(filename, 'w', newline='', encoding='utf-8') as f:
            writer = csv.writer(f)
            writer.writerow(header)
            writer.writerows(valid_rows)

        print(f"✅ File saved: {filename}")
        return True
    except Exception as e:
        print(f"❌ Error saving {filename}: {e}")
        return False

def unify_csv_files(output_filename="unified_data.csv"):
    csv_files = [f for f in os.listdir() if f.endswith('.csv') and f != output_filename]
    if not csv_files:
        print("⚠️ No CSV files found to unify.")
        return

    print(f"\n🔗 Unifying files: {csv_files}")

    with open(output_filename, "w", newline='', encoding='utf-8') as fout:
        writer = None
        for i, filename in enumerate(csv_files):
            with open(filename, "r", encoding='utf-8') as fin:
                reader = csv.reader(fin)
                header = next(reader)

                if writer is None:
                    writer = csv.writer(fout)
                    writer.writerow(header)

                for row in reader:
                    if len(row) == len(header):
                        writer.writerow(row)

    print(f"✅ Files unified into: {output_filename}")

def main():
    try:
        print("🔌 Connecting to serial port...")
        ser = serial.Serial(port, baudrate, timeout=timeout)

        print("📁 Listing files in /spiffs:")
        list_response = send_command(ser, 'esp_spiffs ls /spiffs')
        print("📜 Response from ESP:")
        print(list_response)

        csv_files = []
        for line in list_response.splitlines():
            line = line.strip()
            if ".csv" in line:
                parts = line.split()

                for part in parts:
                    if part.endswith(".csv") and part.startswith("/spiffs/"):
                        csv_files.append(part)
                        break

        if not csv_files:
            print("⚠️ No CSV files found.")
            return

        print(f"🔍 Found CSV files: {csv_files}")

        for esp_filename in csv_files:
            print(f"\n📄 Downloading {esp_filename}...")
            raw_csv_response = send_command(ser, f'esp_spiffs cat {esp_filename}', timeout=3)
            print("📥 Raw CSV response received.")
            csv_content = extract_csv_block(raw_csv_response)

            if csv_content:
                csv_header = "Timestamp,Temperature(C),Humidity(%),MQ4_Voltage(V),MQ7_Voltage(V),MQ7_CO_PPM,ExtraField"
                basename = os.path.basename(esp_filename)

                lines = csv_content.splitlines()
                clean_lines = [line.strip() for line in lines if line.strip().startswith("2025")]

                if clean_lines:
                    csv_content_filtered = csv_header + "\n" + "\n".join(clean_lines)
                else:
                    csv_content_filtered = csv_header  # só cabeçalho se não tiver dados

                save_csv_file(csv_content_filtered, basename)
                print(f"📊 Extracted CSV content from {esp_filename}")
            else:
                print(f"⚠️ No CSV content found in {esp_filename}")

        unify_csv_files()

    except serial.SerialException as e:
        print(f"❌ Serial communication error: {e}")
    finally:
        if 'ser' in locals() and ser.is_open:
            ser.close()
            print("🔌 Serial port closed.")

if __name__ == "__main__":
    main()