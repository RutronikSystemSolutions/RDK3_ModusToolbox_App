/*
 * gps_maths.c
 *
 *  Created on: 12 Jan 2024
 *      Author: jorda
 */

#include "gps_maths.h"

#include <math.h>

#define radians(angle) ((angle) * M_PI / 180.0)
#define degress(angle) ((angle) * 180.0 / M_PI)


double gps_maths_distance_between(double lat1, double long1, double lat2, double long2)
{
  // Using Harvesine formula
	const double R = 6371000; // radius in meters
	double phi1 = radians(lat1);
	double phi2 = radians(lat2);
	double deltaphi = radians(lat2 - lat1);
	double deltadelta = radians(long2 - long1);
	double a = sin(deltaphi/2) * sin(deltaphi/2) +
			cos(phi1) * cos(phi2) *
			sin (deltadelta/2) * sin(deltadelta/2);

	double c = 2 * atan2(sqrt(a), sqrt(1-a));

	return (R * c);
}

double gps_maths_bearing(double lat1, double long1, double lat2, double long2)
{
	double y = sin(radians(long2 - long1)) * cos(radians(lat2));
	double x = cos(radians(lat1)) * sin(radians(lat2)) - sin(radians(lat1)) * cos(radians(lat2)) * cos(radians(long2 - long1));
	double bearing = atan2(y,x);
	double angle_degress = degress(bearing);
	if (angle_degress < 0)
	{
		angle_degress = angle_degress + 360;
	}
	return angle_degress;
}
