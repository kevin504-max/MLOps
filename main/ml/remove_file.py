import serial

ser = serial.Serial('/dev/ttyUSB0', 115200, timeout=2)

def delete_file(filename):
    cmd = f'esp_spiffs rm {filename}\n'
    ser.write(cmd.encode())
    response = ser.read_until(b'\n').decode()
    print(response)

delete_file('/spiffs/data_25_06_10-19_30.csv')