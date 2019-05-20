/*
 * coord.cpp
 *
 *  Created on: Aug 13, 2018
 *      Author: sid
 */

#define _USE_MATH_DEFINES
#include <cmath>

#include "oab.hpp"

#define D_FAK			6371000.0f			// FAI Sphere, mean erath radis in meter

double Coord::coord_coords2distance(const double lat1, const double lon1, const double lat2, const double lon2) // in metern
{
	return (2.0*asin(sqrt( pow((sinf((lat1-lat2)/2.0)),2.0) +
			cos(lat1)*cos(lat2)*pow((sin((lon1-lon2)/2.0f)),2.0) ))*D_FAK);
}

double Coord::coord_coords2distance_deg(const double lat1, const double lon1, const double lat2, const double lon2) // in metern
{
	return coord_coords2distance(deg2rad(lat1), deg2rad(lon1), deg2rad(lat2), deg2rad(lon2));
}
