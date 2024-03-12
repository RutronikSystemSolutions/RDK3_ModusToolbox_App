/*
 * gps_maths.h
 *
 *  Created on: 12 Jan 2024
 *      Author: jorda
 */

#ifndef UM980_GPS_MATHS_H_
#define UM980_GPS_MATHS_H_

double gps_maths_distance_between(double lat1, double long1, double lat2, double long2);

/**
 * @brief Get the bearing
 *
 * The bearing is the compass direction to apply to go from the position 1 (lat1, long1) to the position 2 (lat2, long2)
 *
 * @param [in] lat1 Latitude of position 1
 * @param [in] long1 Longitude of position 1
 * @param [in] lat2 Latitude of position 2
 * @param [in] long2 Longitude of position 2
 *
 * @retval Bearing in degrees. The value is between 0° and 360°
 * 0°/360° -> North
 * 90° -> East
 * 180° -> South
 * 270° -> West
 */
double gps_maths_bearing(double lat1, double long1, double lat2, double long2);

#endif /* UM980_GPS_MATHS_H_ */
