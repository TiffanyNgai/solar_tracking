# axial tilt imports
# from sun import sunPosition
# from rotational_angle import optimal_rotational_angle
import numpy as np
import serial
import serial.tools.list_ports
import time
from datetime import datetime, timedelta, date

# sun position imports
import math
import numpy as np
from matplotlib import pyplot as plt
import matplotlib.cm as cm
import pandas as pd

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
    
def tilt_angle(time_zone, latitude, longitude, start_date, duration):
    if len(start_date.split("/")) != 3: #TODO: might need to put this to the backend file
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
    current_inc_ang = 90
    for beta in beta_list:
        inc_ang = find_inc_ang(beta, 180, pos[1], pos[0])
        if inc_ang < current_inc_ang:
            opt_tilt_angle = beta
            current_inc_ang = inc_ang

    return opt_tilt_angle


def save_info(ser):
    daylight_start_min, fitted_m, fitted_b = optimal_rotational_angle(opt_date, opt_tilt_angle, time_zone, latitude, longitude)

    fitted_m = round(fitted_m, 3)
    fitted_b = round(fitted_b, 3)

    start_time = str(datetime.now().strftime("%m-%d-%Y-%H-%M-%S"))
    ser.write(start_time.encode('utf-8'))
    ser.write(b'\n')
    ser.write(str(time_zone).encode('utf-8'))
    ser.write(b'\n')
    ser.write(str(latitude).encode('utf-8'))
    ser.write(b'\n')
    ser.write(str(longitude).encode('utf-8'))
    ser.write(b'\n')
    ser.write(str(opt_tilt_angle).encode('utf-8'))
    ser.write(b'\n')
    ser.write(str(daylight_start_min).encode('utf-8'))
    ser.write(b'\n')
    ser.write(str(fitted_m).encode('utf-8'))
    ser.write(b'\n')
    ser.write(str(fitted_b).encode('utf-8'))
    ser.write(b'\n')

    while(1):
        getData=ser.readline()
        data = getData.decode('utf-8')[:-2]
        print(data)


def accelerometer():
    #TODO: before running this function, we need to tell the user to put the hinge back to the horizontal position so we can get the angle for the horizontal and put it into horizontal_angle
    #TODO: in the backend file, we need to set a max runtime for accelerometer
    #TODO: we're at the right angle only if the abs(opt_tilt_angle - actual_angle) < MAX_ERROR ~0.5 for at least 4 times consecutively

    # time.sleep(1)

    # ports = serial.tools.list_ports.comports()
    # for i, port in enumerate(ports):
    #     if 'usb' in port.device:
    #         usb_port = port.device
    
    # comm_rate = 115200
    # ser = serial.Serial(usb_port, comm_rate)

    # getData=ser.readline()
    # horizontal_angle = float(getData.decode('utf-8')[:-2])

    absolute_tilt_arr = np.arange(0, 91, 0.5)

    for absolute_tilt in absolute_tilt_arr:
        # getData=ser.readline()
        # actual_tilt = (float(getData.decode('utf-8')[:-2]) - horizontal_angle + 180) % 360 - 180
        yield absolute_tilt

        time.sleep(1)


def call_accelerometer():
    # before pressing start button
    horizontal_angle = accelerometer()
    # press start button
    actual_tilt = accelerometer() - horizontal_angle
    
# sun position stuff
def sunPosition(year, month, day, hour=12, m=0, s=0,lat=43.5, long=-80.5):
    twopi = 2 * math.pi
    deg2rad = math.pi / 180

    # Get day of the year, e.g. Feb 1 = 32, Mar 1 = 61 on leap years
    len_month = [0,31,28,31,30,31,30,31,31,30,31,30]
    day = day + np.cumsum(len_month)[month-1]
    leapdays = (year % 4 == 0 and (year % 400 == 0 | year % 100 != 0) \
                and day >= 60 and not (month==2 and day==60))
    day += int(leapdays == True)

    # Get Julian date - 2400000
    hour = hour + m / 60 + s / 3600 # hour plus fraction
    delta = year - 1949
    leap = math.floor(delta / 4) # former leapyears
    jd = 32916.5 + delta * 365 + leap + day + hour / 24

    # The input to the Atronomer's almanach is the difference between
    # the Julian date and JD 2451545.0 (noon, 1 January 2000)
    time = jd - 51545.

    # Ecliptic coordinates

    # Mean longitude
    mnlong = 280.460 + .9856474 * time
    mnlong = mnlong % 360
    mnlong += int(mnlong < 0)*360

    # Mean anomaly
    mnanom = 357.528 + .9856003 * time
    mnanom = mnanom % 360
    mnanom += int(mnanom < 0)*360
    mnanom = mnanom * deg2rad

    # Ecliptic longitude and obliquity of ecliptic
    eclong = mnlong + 1.915 * math.sin(mnanom) + 0.020 * math.sin(2 * mnanom)
    eclong = eclong % 360
    eclong += int(eclong < 0)*360
    oblqec = 23.439 - 0.0000004 * time
    eclong = eclong * deg2rad
    oblqec = oblqec * deg2rad

    # Celestial coordinates
    # Right ascension and declination
    num = math.cos(oblqec) * math.sin(eclong)
    den = math.cos(eclong)
    ra = math.atan(num / den)
    ra += int(den < 0)*math.pi
    ra += int(den >= 0 and num < 0)*twopi
    dec = math.asin(math.sin(oblqec) * math.sin(eclong))

    # Local coordinates
    # Greenwich mean sidereal time
    gmst = 6.697375 + .0657098242 * time + hour
    gmst = gmst % 24
    gmst += int(gmst < 0)*24

    # Local mean sidereal time
    lmst = (gmst + long / 15.)
    lmst = lmst % 24.
    lmst += int(lmst < 0)*24.
    lmst = lmst * 15. * deg2rad

    # Hour angle
    ha = lmst - ra
    ha += int(ha < -math.pi)*twopi
    ha -= int(ha > math.pi)*twopi

    # Latitude to radians
    lat = lat * deg2rad

    # Azimuth and elevation
    el = math.asin(math.sin(dec) * math.sin(lat) + math.cos(dec) * math.cos(lat) * math.cos(ha))
    az = math.asin(-math.cos(dec) * math.sin(ha) / math.cos(el))

    # For logic and names, see Spencer, J.W. 1989. Solar Energy. 42(4):353
    cosAzPos = (0 <= math.sin(dec) - math.sin(el) * math.sin(lat))
    sinAzNeg = (math.sin(az) < 0)
    az += int(cosAzPos and sinAzNeg)*twopi
    if (not cosAzPos):
        az = math.pi - az 

    el = el / deg2rad
    az = az / deg2rad
    lat = lat / deg2rad

    return az, el