from sun import sunPosition
import numpy as np
from datetime import datetime, timedelta

def R_opt(beta_ax, az_ax, el, az,limit=90):
    beta_ax = np.radians(beta_ax)
    az_ax = np.radians(az_ax)
    
    zen = np.radians(90 - el)
    
    azrad = np.radians(az)
        
    arg = np.sin(zen)*np.sin(azrad-az_ax)/ \
            (np.sin(zen)*np.cos(azrad-az_ax)*np.sin(beta_ax) \
             + np.cos(zen)*np.cos(beta_ax))
    
    phi = np.where((arg < 0) & ((azrad-az_ax) > 0) , 180, 
            np.where((arg > 0) & ((azrad-az_ax) < 0), -180,0))
    
    
    R = np.degrees(np.arctan(arg)) + phi
    
    R[R>90] = limit
    R[R<-90] = -limit
    
    return R

def get_current_time():
    now = datetime.datetime.now()
    return now.year, now.month, now.day, now.hour, now.minute

def rotational_angle():
    # set by what the user inputted from axial tilt
    # TODO: we need to somehow get this info from that file
    time_zone = 4
    latitude = 43.4643
    longitude = 80.5204

    # inputs
    beta_ax = 20 # TODO: get from axial tilt file
    az_ax = 180 # axis azimuth 

   # Get current time and add 10 minutes
    year, month, day, hour, minute = get_current_time()
    hour += time_zone
    future_time = datetime.datetime(year, month, day, hour, minute) + datetime.timedelta(minutes=10)
    future_year, future_month, future_day, future_hour, future_minute = future_time.year, future_time.month, future_time.day, future_time.hour, future_time.minute

    pos = sunPosition(future_year, future_month, future_day, future_hour, future_minute, lat=latitude, long=longitude)
    el = pos[1]
    az = pos[0]

    R = R_opt(beta_ax,az_ax,el,az)

# Run the rotational_angle function
rotational_angle()