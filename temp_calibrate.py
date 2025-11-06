
import serial
import time

ser = serial.Serial('COM11', 115200, timeout=2)
time.sleep(2)
ser.write(b'CALIBRATE_TEMP 15\r\n')
time.sleep(3)
response = ser.read(ser.in_waiting)
print('Ð£×¼ÏìÓ¦:')
print(response.decode('utf-8', errors='ignore'))
ser.close()
