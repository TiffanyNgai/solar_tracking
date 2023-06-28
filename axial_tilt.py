# 1. User would input the location, time zone, and intended time of use
# 2. Program calculates the optimal axial tilt
# 3. Program send signal taken from the accelerometer, and user adjust the 
#    tilt angle accordingly

from sun import sunPosition
import numpy as np
import serial
import serial.tools.list_ports
import time
from datetime import datetime, timedelta

def find_inc_ang(beta, panel_az, el, az):
    
    beta = np.radians(beta)
    panel_az = np.radians(panel_az)
    zen = np.radians(90 - el)
    azrad = np.radians(az)
    
    arg = np.cos(beta)*np.cos(zen)+ \
            np.sin(beta)*np.sin(zen)*np.cos(azrad-panel_az)
    deg = np.degrees(np.arccos(arg))
    
    # max value is 90, means output is zero 
    if deg > 90: deg = 90
    
    return deg

def tilt_angle():
    time_zone = int(input("Enter your timezone (4 for GMT -4): "))
    latitude = float(input("Enter the latitude of the location: "))
    longitude = float(input("Enter the longitude of the location: "))
    start_date = input("Enter the start date which the device is being attached to the sign (mm/dd/yyyy): ")
    duration = int(input("Enter the estimated number of day(s) the device is staying on the sign: "))

    if len(start_date.split("/")) != 3:
        print("Error: Invalid start date")
        return
    
    [start_month, start_day, start_year] = start_date.split("/")
    start_month = int(start_month)
    start_day = int (start_day)
    start_year = int(start_year)
    start_date = datetime(start_year, start_month, start_day)

    # Use the optimal tile angle calculated on start_date + duration/2 
    opt_date = start_date + timedelta(days=int(duration/2))

    year = opt_date.year
    month = opt_date.month
    day = opt_date.day

    pos = sunPosition(year,month,day,12+time_zone,0, lat=latitude, long=longitude)
    beta_list = np.arange(0.0, 90.0, 0.5)
    opt_tilt_angle = 0
    current_inc_ang = 90
    for beta in beta_list:
        inc_ang = find_inc_ang(beta, 180, pos[1], pos[0])
        if inc_ang < current_inc_ang:
            opt_tilt_angle = beta
            current_inc_ang = inc_ang

    return opt_tilt_angle

def accelerometer(opt_tilt_angle):
    print("Now we're going to adjust the tilt angle. Move the hinge back to the horizontal position.")

    time.sleep(1)

    ports = serial.tools.list_ports.comports()
    for port in ports:
        if 'usb' in port.device:
            usb_port = port.device
    
    comm_rate = 9600
    ser = serial.Serial(usb_port, comm_rate)

    start_time = time.time()
    MAX_RUNTIME = 300

    getData=ser.readline()
    horizontal_angle = float(getData.decode('utf-8')[:-2])

    while(True):
        
        getData=ser.readline()
        actual_tilt = horizontal_angle - float(getData.decode('utf-8')[:-2])

        print(actual_tilt)

        difference = opt_tilt_angle - actual_tilt
        MAX_ERROR = 0.5

        if abs(difference) < MAX_ERROR:
            print("You're at the right angle, hold on...")
            time.sleep(1)
            if abs(difference) < MAX_ERROR:
                print("Congrats, the panel is at the right position.")
                break
        elif difference < 0:
            print(f"Move DOWN the panel. Actual angle: {actual_tilt}, target angle: {opt_tilt_angle}")
        elif difference > 0:
            print(f"Move UP the panel. Actual angle: {actual_tilt}, target angle: {opt_tilt_angle}")

        if (time.time() - start_time) > MAX_RUNTIME:
            print("Program automatically shuts down because it's been running for too long.")
            break

    return


if __name__ == "__main__":
    # opt_tilt_angle = tilt_angle()
    opt_tilt_angle = 20
    accelerometer(opt_tilt_angle)
