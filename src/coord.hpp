/*
 * coord.hpp
 *
 *  Created on: Aug 11, 2018
 *      Author: sid
 */

#ifndef SRC_COORD_HPP_
#define SRC_COORD_HPP_


class Coord
{
private:
	double coord_coords2distance(const double lat1, const double lon1, const double lat2, const double lon2); // in metern
	double coord_coords2distance_deg(const double lat1, const double lon1, const double lat2, const double lon2); // in metern

public:
	float latitude;
	float longitude;

	Coord()	: latitude(0), longitude(0) { }
	Coord(float latitude, float longitude) : latitude(latitude), longitude(longitude) { }
	~Coord() { }

	/* distance */
	float distto(Coord &v)
	{
		return coord_coords2distance_deg(latitude, longitude, v.latitude, v.longitude);
	}
};


#endif /* SRC_COORD_HPP_ */
