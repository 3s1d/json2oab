/*
 * JsonParser.h
 *
 *  Created on: May 3, 2019
 *      Author: Stefan Seifert
 */

#pragma once
#include <rapidjson/document.h>

#include "kmlcreator.h"
#include "boost/regex.hpp"

#include "otb.hpp"
#include "oab.hpp"

class JsonParser
{
private:
	//OAB tempAirspace;
	std::vector<OAB> airspaces;
	std::vector<Coord> tempPolygoneCoordinates;
	enum class AirspaceLimit {UpperLimit, LowerLimit};
	boost::regex notamExpr{ ".+/\\d+\\s+NOTAM.+" };
	string lastIsoCode = string();

	time_t ParseTime(std::string& time);
	
	double SetAirspaceLimits(OAB& tempAirspace, rapidjson::Value& airspace, AirspaceLimit limit);
	void SetAirspaceClass(OAB& tempAirspace, rapidjson::Value& airspace);
	void SetAirspaceName(OAB& tempAirspace, rapidjson::Value& airspace);
	void SetAirspacePolygons(OAB& tempAirspace, rapidjson::Value&);
	void SetAirspceActivations(OAB& tempAirspace, rapidjson::Value&);
	
	
public:
	KmlCreator kmlCreator;

	JsonParser();
	~JsonParser();

	void Parse(std::string fileName);
	bool WriteOab(std::string fname, std::ofstream *otbStream = nullptr);
};

