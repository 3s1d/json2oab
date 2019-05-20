/*
 * JsonParser.h
 *
 *  Created on: May 3, 2019
 *      Author: xoration
 */

#pragma once
#include "oab.hpp"
#include <rapidjson/document.h>
#include "kmlcreator.h"
#include "boost/regex.hpp"

class JsonParser
{
private:
	//OAB tempAirspace;
	std::vector<OAB> airspaces;
	std::vector<Coord> tempPolygoneCoordinates;
	enum AirspaceLimit {UpperLimit, LowerLimit};
	boost::regex notamExpr{ ".+/\d+\s*NOTAM" };

	time_t ParseTime(std::string& time);

	KmlCreator kmlCreator;

public:
	JsonParser();
	~JsonParser();

	void Parse(std::string fileName);
	void SetAirspaceClass(OAB& tempAirspace, rapidjson::Value& airspace);
	void SetAirspaceName(OAB& tempAirspace, rapidjson::Value& airspace);
	double SetAirspaceLimits(OAB& tempAirspace, rapidjson::Value& airspace, AirspaceLimit limit);
	void SetAirspacePolygons(OAB& tempAirspace, rapidjson::Value&);
	void SetAirspceActivations(OAB& tempAirspace, rapidjson::Value&);
	bool WriteOba(std::string fname);
};

