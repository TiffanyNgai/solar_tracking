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

def rotational_angle():
    # info from axial tilt file
    time_zone = 4

    # optimal date 
    year = 2023
    month = 7
    day = 6

    # other inputs
    beta_ax = 20 
    az_ax = 180 
    
    hrs = np.arange(0+time_zone,24+time_zone)
    mins = np.arange(0,60)

    pos_every_min = np.array([sunPosition(year,month,day,hr,mn) 
        for hr,mn in zip(np.repeat(hrs,60),np.tile(mins,24))])

    # get elevation
    el = pos_every_min[:,1][pos_every_min[:,1]>0]
    # get az
    az = pos_every_min[:,0][pos_every_min[:,1]>0]

    R = R_opt(beta_ax,az_ax,el,az)

    line_best_fit(R)

# Run the rotational_angle function
rotational_angle()