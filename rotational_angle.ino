#include <math.h>
#include <TimeLib.h>

double R_opt(double beta_ax, double az_ax, double el, double az, double limit = 90)
{
    beta_ax = radians(beta_ax);
    az_ax = radians(az_ax);

    double zen = radians(90 - el);
    double azrad = radians(az);

    double arg = sin(zen) * sin(azrad - az_ax) /
                 (sin(zen) * cos(azrad - az_ax) * sin(beta_ax) +
                  cos(zen) * cos(beta_ax));

    double phi = (arg < 0 && (azrad - az_ax) > 0) ? 180 : ((arg > 0 && (azrad - az_ax) < 0) ? -180 : 0);

    double R = degrees(atan(arg)) + phi;

    if (R > 90)
    {
        R = limit;
    }
    if (R < -90)
    {
        R = -limit;
    }

    return R;
}

void get_current_time(int &year, int &month, int &day, int &hour, int &minute)
{
    time_t now = now();
    tmElements_t currentTime;
    breakTime(now, currentTime);
    year = currentTime.Year + 1970;
    month = currentTime.Month;
    day = currentTime.Day;
    hour = currentTime.Hour;
    minute = currentTime.Minute;
}

void rotational_angle()
{
    // Set by what the user inputted from axial tilt
    // TODO: We need to somehow get this info from that file
    int time_zone = 4;
    double latitude = 43.4643;
    double longitude = 80.5204;

    // Inputs
    double beta_ax = 20; // TODO: Get from axial tilt file
    double az_ax = 180;  // axis azimuth

    // Get current time and add 10 minutes
    int year, month, day, hour, minute;
    get_current_time(year, month, day, hour, minute);
    hour += time_zone;
    time_t future_time = now() + (hour * 3600) + (minute * 60) + (10 * 60);
    tmElements_t futureTime;
    breakTime(future_time, futureTime);
    int future_year = futureTime.Year + 1970;
    int future_month = futureTime.Month;
    int future_day = futureTime.Day;
    int future_hour = futureTime.Hour;
    int future_minute = futureTime.Minute;

    // Calculate sun position
    double pos[2];
    sunPosition(future_year, future_month, future_day, future_hour, future_minute, latitude, longitude, pos);
    double el = pos[1];
    double az = pos[0];

    double R = R_opt(beta_ax, az_ax, el, az);

    // Use R value as needed
}

void setup()
{
    // Initialize necessary libraries or settings
}

void loop()
{
    // Call your function here or include the necessary logic
    rotational_angle();

    // Other code for the main loop
}
