# 1. User would input the location, time zone, and intended time of use
# 2. Program calculates the optimal axial tilt
# 3. Program send signal taken from the accelerometer, and user adjust the 
#    tilt angle accordingly

from sun import sunPosition
import numpy as np

def inc_ang(beta, panel_az, el, az):
    
    beta = np.radians(beta)
    panel_az = np.radians(panel_az)
    zen = np.radians(90 - el)
    azrad = np.radians(az)
    
    arg = np.cos(beta)*np.cos(zen)+ \
            np.sin(beta)*np.sin(zen)*np.cos(azrad-panel_az)
    deg = np.degrees(np.arccos(arg))
    
    # max value is 90, means output is zero 
    deg[deg>90] = 90
    
    return deg

def tilt_angle():
    time_zone = int(input("Enter your timezone (4 for GMT -4): "))
    latitude = float(input("Enter the latitude of the location: "))
    longitude = float(input("Enter the longitude of the location: "))
    start_date = input("Enter the start date which the device is going to be attached to the sign (mm/dd/yyyy): ")

    if len(start_date.split("/")) != 3:
        print("Error: Invalid start date")
        return
    
    [start_month, start_day, start_year] = start_date.split("/")
    start_month = int(start_month)
    start_day = int(start_day)
    start_year = int(start_year)
    
    print(latitude)
    print(type(latitude))




    # # Jun 1st
    # time = np.repeat(np.arange(0,24),60) + np.tile(np.arange(0,60),24)*1/60
    # beta0601_opt = 43.5 - 23.5
    # pos0601 = np.array([sunPosition(2023,6,1,hr,mn) for hr,mn in zip(np.repeat(hrs,60),np.tile(mins,24))])
    # beta0601_list = np.linspace(0, 40,5)
    # for beta in beta0601_list:
    #     inc_ang0601 = inc_ang(beta, 180, pos0601[:,1], pos0601[:,0])
    #     plt.plot(time, inc_ang0601, '.',markersize=2,label=beta)
    # plt.xlabel('Time (hr)')
    # plt.ylabel('Incidence angle (degree)')
    # plt.title(f'Incidence angle at Jun 1st, optimal tilt angle: {beta0601_opt}')
    # plt.legend(title='Tilt angle (degree)')
    # plt.grid()
    # plt.show()



    return



if __name__ == "__main__":
    tilt_angle()

