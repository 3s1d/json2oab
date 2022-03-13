/*
 * jsonparser.cpp
 *
 *  Created on: May 3, 2019
 *      Author: Stefan Seifert
 */

#include <cassert>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <boost/filesystem/fstream.hpp>
#include <boost/regex.hpp>
#include <boost/date_time.hpp>
#include <iostream>
#include <sstream>
#include <locale>
#include <ctime>
#include <sys/stat.h>
#include "jsonparser.h"
#include "activationtime.h"
#include "kmlcreator.h"

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
		
		printf("Parsing: %s\n", document["channame"].GetString());

		if(document.HasMember("isocode"))
		{
			if(std::strcmp(document["isocode"].GetString(), "CG") == 0)
			{
				std::cout << "Dropping " << fileName << std::endl;
				return;
			}

			lastIsoCode = document["isocode"].GetString();
		}
		
		for (auto& airspace : document["airspaces"].GetArray()) {
			bool skipAirspace = false;
			
			if (boost::iequals(document["channame"].GetString(), "Switzerland"))
			{
				if (airspace.HasMember("airchecktype"))
				{
					if (boost::iequals(airspace["airchecktype"].GetString(), "ignore")) {

						std::cout << "Switzerland: " << airspace["name"].GetString() << "IGNORE." << std::endl;
						skipAirspace = true;
						continue;
					}
				}

				/*
				 * exclude DABS from CH if no activations are known
				 * "descriptions": [{"airlanguage": "en", "airdescription": "Mil Radar\r\nTYPE:Q\r\nDABS activated\r\n"}],
				 * "activations": [],
				 */
				if (airspace.HasMember("descriptions"))
				{
					for (auto& description : airspace["descriptions"].GetArray())
					{
						if (description.HasMember("airdescription"))
						{
							std::string airDescription = description["airdescription"].GetString();

							if (boost::regex_match(airDescription, dabsExpr))
							{
								std::cout << "Found DABS: " << airspace["name"].GetString() << std::endl;
								if(airspace.HasMember("activations") == false or airspace["activations"].GetArray().Empty())
								{
									std::cout << "Found DABS skipping: " << airspace["name"].GetString() << std::endl;
									skipAirspace = true;
									break;
								}
							}
						}
					}
				}

			}
			
			OAB tempSpace;
			std::string airspaceName = airspace["name"].GetString();

			if (airspaceName.find("NOTAM") != std::string::npos)
			{
				std::cout << "Found Notam skipping: " << airspaceName << std::endl;
				skipAirspace = true;
				continue;
			}

			if (boost::starts_with(airspaceName, "FIS"))
			{
				std::cout << "Found FIS skipping: " << airspaceName << std::endl;
				skipAirspace = true;
				continue;
			}

			if (airspace.HasMember("descriptions"))
			{
				for (auto& description : airspace["descriptions"].GetArray())
				{
					if (description.HasMember("airdescription"))
					{
						std::string airDescription = description["airdescription"].GetString();

						if (boost::regex_match(airDescription, notamExpr))
						{
							std::cout << "Found Notam skipping: " << airspaceName << std::endl;
							skipAirspace = true;
							break;
						}
					}
				}
			}

			if(airspaceName == "AUSTRIAN BORDER")
			{
                                std::cout << "Found AT border skipping: " << airspaceName << std::endl;
                                skipAirspace = true;
                                continue;
			}

			if (skipAirspace)
			{
				continue;
			}

			/* Air ID */
			if (airspace.HasMember("airid"))
				tempSpace.header.airid = airspace["airid"].GetUint();
			else
				tempSpace.header.airid = 0;

			tempSpace.header.type = OAB::UNDEFINED;
			SetAirspaceName(tempSpace, airspace);
			SetAirspaceClass(tempSpace, airspace);
			double lowerAltitude = SetAirspaceLimits(tempSpace, airspace, AirspaceLimit::LowerLimit);
			double upperAltitude = SetAirspaceLimits(tempSpace, airspace, AirspaceLimit::UpperLimit);
			SetAirspacePolygons(tempSpace, airspace["polygon"]);
			SetAirspceActivations(tempSpace, airspace);

			if (tempSpace.header.type == OAB::IGNORE)
				continue;

			
			kmlCreator.AddAirspace(document["channame"].GetString(),
				document["description"].IsNull() ? "" : document["description"].GetString(),
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
			if (airspace["airchecktype"] == "ignore") {
				tempAirspace.header.type = OAB::IGNORE; //ignore
			}
			else {
				tempAirspace.header.type = OAB::PROHIBITED;
			}
		else if (airspace["airclass"] == "Q" || airspace["airclass"] == "DANGER")
			tempAirspace.header.type = OAB::DANGER;
		else if (airspace["airclass"] == "R" || airspace["airclass"] == "RESTRICTED") // Restricted
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
		else if (airspace["airclass"] == "GP" || airspace["airclass"] == "PROHIBITED")
			tempAirspace.header.type = OAB::PROHIBITED;
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
		else if (airspace["airclass"] == "ZSM")
			tempAirspace.header.type = OAB::RESTRICTED;
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
	case AirspaceLimit::UpperLimit:
		jsonLimit = "upperLimit";
		break;
	case AirspaceLimit::LowerLimit:
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
		case AirspaceLimit::UpperLimit:
			tempAirspace.header.flags |= altref << OAB_ALTREF_TOP_OFFSET;
			tempAirspace.header.altitudeTop_ft = altitudeFt;
			break;
		case AirspaceLimit::LowerLimit:
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

/*
 * Case Sensitive Implementation of endsWith()
 * It checks if the string 'mainStr' ends with given string 'toMatch'
 */
bool endsWith(const std::string &mainStr, const std::string &toMatch)
{
    if(mainStr.size() >= toMatch.size() &&
            mainStr.compare(mainStr.size() - toMatch.size(), toMatch.size(), toMatch) == 0)
            return true;
        else
            return false;
}

bool JsonParser::WriteOab(std::string fileName, std::ofstream *otbStream)
{
	/* nothing to do */
	if(airspaces.size() == 0)
	{
		std::cout << "Empty" << std::endl;
		return true;
	}

	/* split file */
	if(endsWith(fileName, ".oab") == false)
		fileName = fileName + "/" + lastIsoCode + ".oab";

	/* detect existing file */
	struct stat buffer;
	bool fileExists = stat(fileName.c_str(), &buffer) == 0;

	/* additional header */
	uint16_t numAsp = airspaces.size();
	OAB::bb_t bb;
	bb.topLat_rad = -M_PI;
	bb.bottomLat_rad = M_PI;
	bb.leftLon_rad =  M_2_PI;
	bb.rightLon_rad = -M_PI_2;
	for(auto &asp : airspaces)
	{
		for(auto &pt : asp.polygon)
		{
			if(pt.lat_rad > bb.topLat_rad)
				bb.topLat_rad = pt.lat_rad;
			if(pt.lat_rad < bb.bottomLat_rad)
				bb.bottomLat_rad = pt.lat_rad;
			if(pt.lon_rad > bb.rightLon_rad)
				bb.rightLon_rad = pt.lon_rad;
			if(pt.lon_rad < bb.leftLon_rad)
				bb.leftLon_rad = pt.lon_rad;
		}
	}

	/* open */
	std::fstream myFile(fileName, fileExists ? (std::ios::in | std::ios::out | std::ios::binary) : (std::ios::out | std::ios::binary) );
	if(fileExists == false)
	{
		OAB::writeFileHeader(myFile, bb, airspaces.size());
	}
	else
	{
		OAB::bb_t bbExist;
		uint16_t numAspExist;
		if(OAB::readFileHeader(myFile, bbExist, numAspExist) == false)
		{
			std::cerr << "Unable to read header" << std::endl;
			myFile.close();
			return false;
		}

		numAsp += numAspExist;
		if(bbExist.topLat_rad > bb.topLat_rad)
			bb.topLat_rad = bbExist.topLat_rad;
		if(bbExist.bottomLat_rad < bb.bottomLat_rad)
			bb.bottomLat_rad = bbExist.bottomLat_rad;
		if(bbExist.rightLon_rad > bb.rightLon_rad)
			bb.rightLon_rad = bbExist.rightLon_rad;
		if(bbExist.leftLon_rad < bb.leftLon_rad)
			bb.leftLon_rad = bbExist.leftLon_rad;

		/* update header */
		myFile.seekp(0, std::ios::beg);
		OAB::writeFileHeader(myFile, bb, numAsp);

		myFile.seekp(0, std::ios::end);
		std::cout<< "(continued)" << std::endl;
	}

	/* sort airspaces */
	sort(airspaces.begin(), airspaces.end(), [](const OAB &lhs, const OAB &rhs) { return lhs.header.airid < rhs.header.airid; });

	/* write airspaces */
	for (auto &airspace : airspaces)
	{
		airspace.write(myFile, otbStream == nullptr);
		if(otbStream != nullptr)
			airspace.writeActivations(otbStream);
	}

	myFile.close();
	std::cout << (fileExists? "Appended " : "Written ") << airspaces.size() << " airspaces (" << numAsp << " total)" << std::endl;

	/* cleanup */
	airspaces.clear();
	tempPolygoneCoordinates.clear();

	return true;
}
