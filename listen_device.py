import serial
import time

print('Listening to device serial output...')
ser = serial.Serial('COM11', 115200, timeout=1)
print('Serial connection opened. Listening for 15 seconds...')

start_time = time.time()
try:
    while time.time() - start_time < 15:
        data = ser.read_all()
        if data:
            print(data.decode('utf-8', errors='ignore'), end='', flush=True)
        time.sleep(0.1)
finally:
    ser.close()
    print('\nSerial connection closed.')