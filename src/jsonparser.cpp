/*
 * jsonparser.cpp
 *
 *  Created on: May 3, 2019
 *      Author: Stefan Seifert
 */

#include "jsonparser.h"
#include <cassert>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <boost/filesystem/fstream.hpp>
#include "boost/date_time.hpp"
#include <iostream>
#include <sstream>
#include <locale>
#include "activationtime.h"
#include <ctime>
#include "kmlcreator.h"
#include "boost/regex.hpp"


JsonParser::JsonParser()
{
	kmlCreator = KmlCreator();
}


JsonParser::~JsonParser()
{
}

void JsonParser::Parse(std::string fileName)
{
	boost::filesystem::ifstream ifs(fileName);
	rapidjson::IStreamWrapper isw(ifs);

	rapidjson::Document document;
	document.ParseStream(isw);

	if (document.HasMember("channame")) {
		assert(document.HasMember("channame"));
		printf("Parsing: %s\n", document["channame"].GetString());

		for (auto& airspace : document["airspaces"].GetArray()) {
			OAB tempSpace;
			std::string airspaceName = airspace["name"].GetString();

			if (airspaceName.find("NOTAM") != string::npos)
			{
				continue;
			}

			if (airspaceName.find("PARA Dinkelsbuehl") != string::npos)
			{
				std::cout << "found you";
			}

			if (airspace.HasMember("descriptions"))
			{
				for (auto& description : airspace["descriptions"].GetArray())
				{
					if (description.HasMember("airdescription"))
					{
						string airDescription = description["airdescription"].GetString();

						if (boost::regex_match(airDescription, notamExpr))
						{
							std::cout << "Found Notam skipping" << std::endl;
							continue;
						}
					}
				}
			}

			tempSpace.header.type = OAB::UNDEFINED;
			SetAirspaceName(tempSpace, airspace);
			SetAirspaceClass(tempSpace, airspace);
			double lowerAltitude = SetAirspaceLimits(tempSpace, airspace, LowerLimit);
			double upperAltitude = SetAirspaceLimits(tempSpace, airspace, UpperLimit);
			SetAirspacePolygons(tempSpace, airspace["polygon"]);
			SetAirspceActivations(tempSpace, airspace);

			if (tempSpace.header.type == OAB::IGNORE)
				continue;

			kmlCreator.AddAirspace(document["channame"].GetString(),
				document["description"].GetString(),
				airspace,
				tempPolygoneCoordinates, 
				lowerAltitude / 3.2808,
				upperAltitude/ 3.2808);

			airspaces.push_back(tempSpace);
		}
	}
	else
	{
		std::cerr << "Channame does not exist." << std::endl;
		exit(EXIT_FAILURE);
	}
}

void JsonParser::SetAirspaceClass(OAB & tempAirspace, rapidjson::Value& airspace)
{
	if (airspace.HasMember("airclass")) {
		if (airspace["airclass"] == "A")
			tempAirspace.header.type = OAB::CLASSA;
		else if (airspace["airclass"] == "B")
			tempAirspace.header.type = OAB::CLASSB;
		else if (airspace["airclass"] == "C")
			tempAirspace.header.type = OAB::CLASSC;
		else if (airspace["airclass"] == "D")
			tempAirspace.header.type = OAB::CLASSD;
		else if (airspace["airclass"] == "E")
			tempAirspace.header.type = OAB::IGNORE; //ignore
		else if (airspace["airclass"] == "F")
			tempAirspace.header.type = OAB::IGNORE; //ignore
		else if (airspace["airclass"] == "G")
			tempAirspace.header.type = OAB::IGNORE; //ignore
		else if (airspace["airclass"] == "P")
			tempAirspace.header.type = OAB::PROHIBITED;
		else if (airspace["airclass"] == "Q")
			tempAirspace.header.type = OAB::DANGER;
		else if (airspace["airclass"] == "R") // Restricted
			tempAirspace.header.type = OAB::RESTRICTED;
		else if (airspace["airclass"] == "CTR")
			tempAirspace.header.type = OAB::CTR;
		else if (airspace["airclass"] == "TMA")
			tempAirspace.header.type = OAB::TMA;
		else if (airspace["airclass"] == "TMZ")
			tempAirspace.header.type = OAB::TMZ;
		else if (airspace["airclass"] == "RMZ")
			tempAirspace.header.type = OAB::RMZ;
		else if (airspace["airclass"] == "ATZ")
			tempAirspace.header.type = OAB::ATZ;
		else if (airspace["airclass"] == "WAVE")
			tempAirspace.header.type = OAB::IGNORE; //ignore
		else if (airspace["airclass"] == "GLIDING")
			tempAirspace.header.type = OAB::IGNORE; //ignore
		else if (airspace["airclass"] == "FIR")
			tempAirspace.header.type = OAB::IGNORE; //ignore
		else if (airspace["airclass"] == "LZ")		//XContest Airspaces Ignore
			tempAirspace.header.type = OAB::IGNORE;
		else if (airspace["airclass"] == "GP")
			tempAirspace.header.type = OAB::IGNORE;
		else if (airspace["airclass"] == "W")
			tempAirspace.header.type = OAB::IGNORE;
		else if (airspace["airclass"] == "E2")
			tempAirspace.header.type = OAB::IGNORE;
		else if (airspace["airclass"] == "E3")
			tempAirspace.header.type = OAB::IGNORE;
		else if (airspace["airclass"] == "E4")
			tempAirspace.header.type = OAB::IGNORE;
		else if (airspace["airclass"] == "NoLZ")
			tempAirspace.header.type = OAB::IGNORE;
		else if (airspace["airclass"] == "Powerline")
			tempAirspace.header.type = OAB::IGNORE;
		else if (airspace["airclass"] == "MATZ")
			tempAirspace.header.type = OAB::IGNORE;
		else if (airspace["airclass"] == "TSA")
			tempAirspace.header.type = OAB::IGNORE;
		else if (airspace["airclass"] == "ZP")
			tempAirspace.header.type = OAB::IGNORE;
		else if (airspace["airclass"] == "T")
			tempAirspace.header.type = OAB::IGNORE;
		else if (airspace["airclass"] == "O")
			tempAirspace.header.type = OAB::IGNORE;
		else if (airspace["airclass"] == "EmgyLZ")
			tempAirspace.header.type = OAB::IGNORE;
		else
		{

			std::cerr << "unknown airspace type: " << airspace["airclass"].GetString() << std::endl;
			tempAirspace.header.type = OAB::IGNORE;
		}
	}
	else
	{
		std::cerr << "Airclass does not exist for oaid: " << airspace["oaid"].GetInt64() << std::endl;
		exit(EXIT_FAILURE);
	}
}

void JsonParser::SetAirspaceName(OAB & tempAirspace, rapidjson::Value & airspace)
{
	if (airspace.HasMember("name")) {
		std::string airspaceName = std::string(airspace["name"].GetString(), airspace["name"].GetStringLength());


		if ((tempAirspace.header.type == OAB::TMZ && airspaceName.compare(0, 3, "TMZ") == 0) ||
			(tempAirspace.header.type == OAB::TMA && airspaceName.compare(0, 3, "TMA") == 0) ||
			(tempAirspace.header.type == OAB::CTR && airspaceName.compare(0, 3, "CTR") == 0))
		{
			airspaceName = airspaceName.substr(4);
		}

		tempAirspace.setName(airspaceName);
	}
	else
	{
		std::cerr << "Airspace name does not exist for oaid: " << airspace["oaid"].GetInt64() << std::endl;
		exit(EXIT_FAILURE);
	}
}

double JsonParser::SetAirspaceLimits(OAB & tempAirspace, rapidjson::Value & airspace, AirspaceLimit limit)
{
	std::string jsonLimit;

	switch (limit) {
	case UpperLimit:
		jsonLimit = "upperLimit";
		break;
	case LowerLimit:
		jsonLimit = "lowerLimit";
		break;
	default:
		std::cerr << "Unknown limit." << std::endl;
		exit(EXIT_FAILURE);
	}

	const char* jsonLimit_c = jsonLimit.c_str();

	if (airspace.HasMember(jsonLimit_c))
	{
		int16_t returnAltitudeFeet;
		int16_t altitudeFt = airspace[jsonLimit_c]["hfeet"].GetInt();
		returnAltitudeFeet = altitudeFt;

		std::string htype = std::string(airspace[jsonLimit_c]["htype"].GetString(), airspace[jsonLimit_c]["htype"].GetStringLength());

		uint16_t altref = 0;
		if (htype.compare("FL") == 0)
		{
			altref = OAB_ALTREF_FL;
			altitudeFt = altitudeFt / 100;
		}
		else if (htype.compare("AMSL") == 0 || htype.compare("MAX") == 0)
		{
			altref = OAB_ALTREF_MSL;
		}
		else if (htype.compare("AGL") == 0)
		{
			altref = OAB_ALTREF_GND;
		}
		else
		{
			std::cerr << "unknown htype: " << htype << std::endl;
			exit(EXIT_FAILURE);
		}

		switch (limit) {
		case UpperLimit:
			tempAirspace.header.flags |= altref << OAB_ALTREF_TOP_OFFSET;
			tempAirspace.header.altitudeTop_ft = altitudeFt;
			break;
		case LowerLimit:
			tempAirspace.header.flags |= altref << OAB_ALTREF_BOTTOM_OFFSET;
			tempAirspace.header.altitudeBottom_ft = altitudeFt;
			break;
		default:
			std::cerr << "Unknown limit." << std::endl;
			exit(EXIT_FAILURE);
		}

		return returnAltitudeFeet;
	}
	else
	{
		std::cerr << "limit unknown" << std::endl;
		exit(EXIT_FAILURE);
	}
}


void JsonParser::SetAirspacePolygons(OAB & tempAirspace, rapidjson::Value & polygons)
{
	tempPolygoneCoordinates.clear();

	for (auto& coordinates : polygons.GetArray())
	{
		Coord coord;

		coord.latitude = coordinates[0].GetFloat();
		coord.longitude = coordinates[1].GetFloat();

		tempPolygoneCoordinates.push_back(coord);
	}

	tempAirspace.polygonSample_dist = -100.0f;
	while (tempAirspace.polygon.size() > 500 || tempAirspace.polygonSample_dist < 0.0)					//TODO magic number here
	{
		tempAirspace.polygonSample_dist += 100.0;
		tempAirspace.polygon.clear();

		for (auto coord : tempPolygoneCoordinates)
			tempAirspace.add(coord);
	}


	if (tempPolygoneCoordinates.size() != tempAirspace.polygon.size())
		std::cerr << "Reducing polygon size of " << tempAirspace.header.name << ": " << tempPolygoneCoordinates.size() << " -> " << tempAirspace.polygon.size() <<
		" (" << tempAirspace.polygonSample_dist << "m)" << std::endl;
}

void JsonParser::SetAirspceActivations(OAB & tempAirspace, rapidjson::Value & jsonActivationTimes)
{
	if (!jsonActivationTimes.HasMember("activations"))
		return;

	std::vector<ActivationTime> activationTimes;

	for (auto& jsonActivationTime : jsonActivationTimes["activations"].GetArray())
	{
		OAB::oab_activationTimes_t activationTime;
		std::string startTime = jsonActivationTime[0].GetString();
		std::string endTime = jsonActivationTime[1].GetString();

		activationTime.startActivationZulu = ParseTime(startTime);
		activationTime.endActivationZulu = ParseTime(endTime);

		tempAirspace.activationTimes.push_back(activationTime);
	}
}

time_t JsonParser::ParseTime(std::string & time)
{
	auto posixTimeFacet = new boost::posix_time::time_input_facet();

	std::stringstream boostInput(time);
	boostInput.imbue(std::locale(std::locale(), posixTimeFacet));

	posixTimeFacet->format("%Y-%m-%dT%H:%M:%SZ");

	boost::posix_time::ptime timestamp;
	boostInput >> timestamp;
	time_t posixTimeUTC = to_time_t(timestamp);

	return posixTimeUTC;
}


bool JsonParser::WriteOba(std::string fileName)
{
	kmlCreator.CreateKml("world.oab.kml");

	std::ofstream myFile(fileName, std::ios::out | std::ios::binary);

	OAB::writeFileHeader(myFile);

	for (auto airspace : airspaces)
		airspace.write(myFile);

	myFile.close();

	std::cout << "Written " << airspaces.size() << " airspaces" << std::endl;

	return true;
}