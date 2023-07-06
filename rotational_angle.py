from sun import sunPosition
import numpy as np
from datetime import datetime, timedelta
from scipy.optimize import curve_fit
import matplotlib.pyplot as plt

def line_best_fit(R):
    # Define the function you want to fit (linear function: y = mx + b)
    def linear_function(x, m, b):
        return m * x + b

    # Create the x values corresponding to the R array indices
    x = np.arange(len(R))

    # Perform curve fitting
    fit_params, _ = curve_fit(linear_function, x, R)

    # Extract the fitted parameters
    fitted_m, fitted_b = fit_params

    # Generate the curve using the fitted parameters
    fitted_curve = linear_function(x, fitted_m, fitted_b)

    # Plot the original data and the fitted curve
    plt.plot(x, R, 'ro', label='Original Data')
    plt.plot(x, fitted_curve, 'b-', label='Fitted Curve')
    plt.xlabel('Index')
    plt.ylabel('R Values')
    plt.legend()
    plt.show()

    # Print the equation of the fitted line
    print("Equation of the fitted line:")
    print("y =", fitted_m, "x +", fitted_b)
    return fitted_m, fitted_b



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

def optimal_rotational_angle(optimal_date, beta_ax, time_zone=4, latitude=43.5, longitude=-80.5):
    # optimal date 
    year = optimal_date.year
    month = optimal_date.month
    day = optimal_date.day

    # other inputs
    az_ax = 180 
    
    hrs = np.arange(0+time_zone,24+time_zone)
    mins = np.arange(0,60)

    pos_every_min = np.array([sunPosition(year,month,day,hr,mn,lat=latitude,long=longitude) 
        for hr,mn in zip(np.repeat(hrs,60),np.tile(mins,24))])

    daylight_start_min = next((i for i, value in enumerate(pos_every_min[:,1]) if value > 0), None)

    # get elevation
    el = pos_every_min[:,1][pos_every_min[:,1]>0]
    # get az
    az = pos_every_min[:,0][pos_every_min[:,1]>0]

    R = R_opt(beta_ax,az_ax,el,az)

    print(el.shape)
    print(R.shape)

    fitted_m, fitted_b = line_best_fit(R)

    # print(f"Opt date: {optimal_date}")
    # print(f"beta_ax: {beta_ax}")
    # print(f"Slope: {fitted_m}")
    # print(f"Intercept: {fitted_b}")

    return daylight_start_min, fitted_m, fitted_b


