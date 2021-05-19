/*
 * oab.cpp
 *
 *  Created on: Aug 11, 2018
 *      Author: sid, Stefan Seifert
 */

#define _USE_MATH_DEFINES
#include <cmath>
#include <string>
#include <iostream>
#include <fstream>
#include <cstring>
#include <time.h>

#include "otb.hpp"
#include "oab.hpp"


const char *OAB::id = "OAB\x3";

Coord proj_deg(Coord &src, Coord &poi)
{
	Coord ret;
	ret.longitude = (src.longitude - poi.longitude) * cos(deg2rad((src.latitude + poi.latitude)/2.0));
	ret.latitude = (poi.latitude - src.latitude);

	return ret;
}


float angle(Coord &a, Coord &b, Coord &c)
{
	//const p2df_t ab = { .x = b->x - a->x, .y = b->y - a->y };
	//const p2df_t cb = { .x = b->x - c->x, .y = b->y - c->y };

	Coord ab = proj_deg(a, b);
	Coord cb = proj_deg(c, b);

	const float dot = (ab.longitude * cb.longitude + ab.latitude * cb.latitude); 		// dot product
	const float cross = (ab.longitude * cb.latitude - ab.latitude * cb.longitude); 		// cross product

	return atan2(cross, dot);
}

void OAB::reset(void)
{
	polygon.clear();
	polygonSample_dist = 0.0f;
	std::fill(header.name, header.name + sizeof(header.name), 0);
	header.flags = 0;
}

void OAB::setName(std::string &name)
{
	snprintf(header.name, sizeof(header.name), "%s", name.c_str());
}

void OAB::add2RadVec(Coord &coord)
{
	oab_edge_t point;
	point.lat_rad = deg2rad(coord.latitude);
	point.lon_rad = deg2rad(coord.longitude);

	polygon.push_back(point);
}


void OAB::writeFileHeader(std::ofstream &file, const OAB::bb_t &bb)
{
	writeFileHeader(file, time(nullptr), bb);
}

void OAB::writeFileHeader(std::ofstream& file, time_t buildTime, const OAB::bb_t &bb)
{
	file.write(id, strlen(id));

	file.write((char*)& buildTime, sizeof(time_t));

	file.write((char*)& bb, sizeof(bb_t));
}

void OAB::write(std::ofstream &file, bool includeActivations)
{
	/* finalize header */
	finalize(includeActivations);

	/* write header */
	file.write((char *) &header, sizeof(oab_header_t));

	/* write polygons */
	for (auto poly : polygon) {
		file.write((char*)& poly, sizeof(oab_edge_t));
	}

	/* write activation times */
	if(includeActivations)
	{
		for(auto activationTime : activationTimes)
			file.write((char *)& activationTime, sizeof(oab_activationTimes_t));
	}
}

void OAB::writeActivations(std::ofstream *file)
{
	if(activationTimes.size() == 0)
		return;

	/* header */
	OTB otb = OTB(header.airid, activationTimes.size());
	otb.writeHeader(file);

	/* activation times */
	for(auto activationTime : activationTimes)
		file->write((char *)& activationTime, sizeof(oab_activationTimes_t));
}

void OAB::finalize(bool includeActivationTimes)
{
	/* complete polygon */
	if(last_ign)
	{
		add2RadVec(last_ign_p);
		last_ign = false;
	}
	if(polygon.size() > 2)
	{
		/* remove end point if same as start point */
		oab_edge_t estart = polygon.front();
		oab_edge_t eend = polygon.back();
		Coord start(rad2deg(estart.lat_rad), rad2deg(estart.lon_rad));
		Coord end(rad2deg(eend.lat_rad), rad2deg(eend.lon_rad));
		if(start.distto(end) < 100.0f)
			polygon.pop_back();
	}
	header.numPoly = polygon.size();

	/* add magic number to flags */
	//note: only settings bits. might get corrupted!
	header.flags |= OAB_MAGICNUMBER_FLAG;

	/* bounding box */
	header.topLat_rad = -M_PI;
	header.bottomLat_rad = M_PI;
	header.leftLon_rad = M_PI;
	header.rightLon_rad = -M_PI;
	for(auto poly : polygon)
	{
		if(poly.lat_rad < header.bottomLat_rad)
			header.bottomLat_rad = poly.lat_rad;
		if(poly.lat_rad > header.topLat_rad)
			header.topLat_rad = poly.lat_rad;
		if(poly.lon_rad < header.leftLon_rad)
			header.leftLon_rad = poly.lon_rad;
		if(poly.lon_rad > header.rightLon_rad)
			header.rightLon_rad = poly.lon_rad;
	}

	if(includeActivationTimes == true)
		header.flags |= (activationTimes.size() & OAB_NUMACIVATIONS_MASK) << OAB_NUMACIVATIONS;
	else if(activationTimes.size() > 0)
		header.flags |= OAB_NUMACIVATIONS_MASK << OAB_NUMACIVATIONS;			//indicate external file
}

void OAB::add(Coord &coord)
{
	/* first node */
	if(polygon.size() == 0)
	{
		add2RadVec(coord);
		last_p = coord;
		last_ign = false;
		return;
	}

	if(coord.distto(last_p) > polygonSample_dist)
	{
		/* angle greater then 45deg */
		//note: in case of !sharp_edges, last_ign will never be true
		if(last_ign && fabs(angle(last_p, last_ign_p, coord)) < 2.3)
		{
			/* add last ignored point in order to guaranty sharp edges */
			add2RadVec(last_ign_p);
		}

		/* current point */
		add2RadVec(coord);
		last_p = coord;
		last_ign = false;
	}
	else
	{
		/* no need to draw that point. however, store it... */
		last_ign_p = coord;
		last_ign = true;

	}
}
