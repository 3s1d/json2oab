/*
 * kmlcreator.cpp
 *
 *  Created on: Mai 13, 2019
 *      Author: Stefan Seifert
 */

#include "kmlcreator.h"
#include <fstream>
#include <string>
#include <kml/dom.h>
#include <zlib.h>

void KmlCreator::CreateStyle(std::string airClass, const char lineABGR[], const char polyABGR[])
{
	kmldom::StylePtr style = kmlFactory->CreateStyle();
	style->set_id(airClass);
	kmldom::LineStylePtr lineStyle = kmlFactory->CreateLineStyle();
	lineStyle->set_width(1.5);
	lineStyle->set_color(kmlbase::Color32(lineABGR[0], lineABGR[1], lineABGR[2], lineABGR[3]));
	style->set_linestyle(lineStyle);

	kmldom::PolyStylePtr polystyle = kmlFactory->CreatePolyStyle();
	polystyle->set_color(kmlbase::Color32(polyABGR[0], polyABGR[1], polyABGR[2], polyABGR[3]));
	style->set_polystyle(polystyle);
	kmlDocument->add_styleselector(style);
}

KmlCreator::KmlCreator()
{
	kmlFactory = kmldom::KmlFactory::GetFactory();
	kmlDocument = kmlFactory->CreateDocument();

	CreateStyle("default", "\x7F\xFF\x00\x00", "\x7F\xFF\x00\x00");

	CreateStyle("R", "\xFF\xFF\x00\xFF", "\x10\xFF\x00\xFF");
	CreateStyle("Q", "\xFF\x60\xB0\xFF", "\x40\x60\xB0\xFF");
	CreateStyle("P", "\x7D\x00\x00\x33", "\x40\x00\x00\x30");
	CreateStyle("A", "\x7D\x00\x00\x44", "\x40\x00\x00\x44");
	CreateStyle("B", "\x7D\x00\x00\x55", "\x40\x00\x00\x55");
	CreateStyle("C", "\xFF\xCC\x30\x30", "\x20\xAA\x00\x00");
	CreateStyle("D", "\xFF\xCC\x30\x30", "\x20\xAA\x00\x00");
	CreateStyle("E", "\xFF\xCC\x30\x30", "\x40\xAA\x00\x00");
	CreateStyle("GP", "\x7D\x00\x00\x99", "\x40\x00\x00\x99");
	CreateStyle("CTR", "\xFF\xFF\x00\xFF", "\x40\xFF\x00\xFF");
	CreateStyle("W", "\xFF\x40\xD0\xFF", "\x40\x40\xD0\xFF");
	CreateStyle("DCTR", "\xFF\xFF\x00\xFF", "\x40\xFF\x00\xFF");
	CreateStyle("TMZ", "\xFF\30\xCC\x30", "\x40\x00\xAA\x00");
}

KmlCreator::~KmlCreator()
{

}

void KmlCreator::SetAirspaceStyle(kmldom::PlacemarkPtr placemark, std::string airspaceClass)
{
	if (airspaceClass.find("R") != string::npos)
	{
		placemark->set_styleurl("#R");
	}
	else if (airspaceClass.find("Q") != string::npos)
	{
		placemark->set_styleurl("#Q");
	}
	else if (airspaceClass.find("P") != string::npos)
	{
		placemark->set_styleurl("#P");
	}
	else if (airspaceClass.find("A") != string::npos)
	{
		placemark->set_styleurl("#A");
	}
	else if (airspaceClass.find("B") != string::npos)
	{
		placemark->set_styleurl("#B");
	}
	else if (airspaceClass.find("C") != string::npos)
	{
		placemark->set_styleurl("#C");
	}
	else if (airspaceClass.find("D") != string::npos)
	{
		placemark->set_styleurl("#D");
	}
	else if (airspaceClass.find("E") != string::npos)
	{
		placemark->set_styleurl("#E");
	}
	else if (airspaceClass.find("GP") != string::npos)
	{
		placemark->set_styleurl("#GP");
	}
	else if (airspaceClass.find("CTR") != string::npos)
	{
		placemark->set_styleurl("#CTR");
	}
	else if (airspaceClass.find("W") != string::npos)
	{
		placemark->set_styleurl("#W");
	}
	else if (airspaceClass.find("DCTR") != string::npos)
	{
		placemark->set_styleurl("#DCTR");
	}
	else if (airspaceClass.find("TMZ") != string::npos)
	{
		placemark->set_styleurl("#TMZ");
	}
	else {
		placemark->set_styleurl("#default");
	}
}

std::string KmlCreator::CreateDescription(std::string airspaceClass, float floorLimitM, float topLimitM, double floorLimitAglM)
{
	std::ostringstream stringStream;
	stringStream << "<table cellpadding=0 cellspacing=0 width=300>" << std::endl;
	stringStream << "<tr>" << std::endl;
	stringStream << "<td>" << "Airspace Class" << "</td>" << std::endl;
	stringStream << "<td>" << airspaceClass << "</td>" << std::endl;
	stringStream << "</tr>" << std::endl;

	stringStream << "<tr>" << std::endl;
	stringStream << "<td>" << "Ceiling" << "</td>" << std::endl;
	stringStream << "<td>" << round(topLimitM) << "m" << "</td>" << std::endl;
	stringStream << "</tr>" << std::endl;

	stringStream << "<tr>" << std::endl;
	stringStream << "<td>" << "Floor" << "</td>" << std::endl;
	stringStream << "<td>" << round(floorLimitM) << "m" << "</td>" << std::endl;
	stringStream << "</tr>" << std::endl;
	if(std::isnan(floorLimitAglM) == false and floorLimitAglM > 0.0)
	{
		stringStream << "<tr>" << std::endl;
		stringStream << "<td>" << "</td>" << std::endl;
		stringStream << "<td>" << round(floorLimitAglM) << "m AGL" << "</td>" << std::endl;
		stringStream << "</tr>" << std::endl;
	}

	stringStream << "<tr bgcolor = '#D7E1EE'>" << std::endl;
	stringStream << "<td> </td>" << std::endl;
	stringStream << "<td>" << std::endl;
	stringStream << "<div align = 'right'>KMZ file made by <a href = 'http://www.skytraxx.eu'>skytraxx.eu</a></div>";
	stringStream << "</td></tr>";

	stringStream << "</table>" << std::endl;

	return stringStream.str();
}

void KmlCreator::BuildPolygonSides(kmldom::FolderPtr airspaceFolder, std::string airspaceClass, Coord coordinate1, Coord coordinate2, double floorAltitude, double topAltitude)
{
	kmldom::CoordinatesPtr airspaceCoordinates = kmlFactory->CreateCoordinates();

	airspaceCoordinates->add_latlngalt(coordinate1.latitude, coordinate1.longitude, floorAltitude);
	airspaceCoordinates->add_latlngalt(coordinate1.latitude, coordinate1.longitude, topAltitude);
	airspaceCoordinates->add_latlngalt(coordinate2.latitude, coordinate2.longitude, topAltitude);
	airspaceCoordinates->add_latlngalt(coordinate2.latitude, coordinate2.longitude, floorAltitude);
	airspaceCoordinates->add_latlngalt(coordinate1.latitude, coordinate1.longitude, floorAltitude);

	kmldom::LinearRingPtr linearRing = kmlFactory->CreateLinearRing();

	linearRing->set_coordinates(airspaceCoordinates);
	kmldom::OuterBoundaryIsPtr outerboundaryis = kmlFactory->CreateOuterBoundaryIs();
	outerboundaryis->set_linearring(linearRing);

	kmldom::PolygonPtr polygon = kmlFactory->CreatePolygon();
	polygon->set_tessellate(true);
	polygon->set_altitudemode(kmldom::ALTITUDEMODE_ABSOLUTE);

	polygon->set_outerboundaryis(outerboundaryis);

	kmldom::PlacemarkPtr placemark = kmlFactory->CreatePlacemark();
	placemark->set_name(airspaceFolder->get_name() + "_Side");
	placemark->set_geometry(polygon);
	SetAirspaceStyle(placemark, airspaceClass);

	airspaceFolder->add_feature(placemark);
}

kmldom::PolygonPtr KmlCreator::BuildPolygonTopBottom(std::vector<Coord> coordinates, double altitude)
{
	kmldom::CoordinatesPtr airspaceCoordinates = kmlFactory->CreateCoordinates();

	for (auto coordinate: coordinates)
	{
		airspaceCoordinates->add_latlngalt(coordinate.latitude, coordinate.longitude, altitude);
	}
	airspaceCoordinates->add_latlngalt(coordinates[0].latitude, coordinates[0].longitude, altitude);

	kmldom::LinearRingPtr linearRing = kmlFactory->CreateLinearRing();
	linearRing->set_coordinates(airspaceCoordinates);
	kmldom::OuterBoundaryIsPtr outerboundaryis = kmlFactory->CreateOuterBoundaryIs();
	outerboundaryis->set_linearring(linearRing);

	kmldom::PolygonPtr polygon = kmlFactory->CreatePolygon();
	polygon->set_outerboundaryis(outerboundaryis);

	return polygon;
}

void KmlCreator::AddAirspace(std::string channel, std::string countryDescription, rapidjson::Value& airspace, std::vector<Coord> coordinates,
		double floorAltitude, double topAltitude, double floorAltitudeAgl)
{
	float lowerLimitM = floorAltitude;		//airspace["lowerLimit"]["hfeet"].GetInt() / 3.28084;
	float upperLimitM = topAltitude;		//airspace["upperLimit"]["hfeet"].GetInt() / 3.28084;
	std::string description = CreateDescription(airspace["airclass"].GetString(), lowerLimitM, upperLimitM, floorAltitudeAgl);

	kmldom::FolderPtr channelFolder;
	if (folderPtrMap.count(channel) == 0)
	{
		channelFolder = kmlFactory->CreateFolder();
		channelFolder->set_name(channel);
		channelFolder->set_description(countryDescription);
		folderPtrMap.insert({ channel, channelFolder });
	}

	kmldom::FolderPtr airspaceFolder = kmlFactory->CreateFolder();
	airspaceFolder->set_name(airspace["name"].GetString());
	//airspaceFolder->set_description(description);

	if (floorAltitude > 0)
	{
		kmldom::PolygonPtr polygonFloor = BuildPolygonTopBottom(coordinates, floorAltitude);
		kmldom::PolygonPtr polygonTop = BuildPolygonTopBottom(coordinates, topAltitude);;

		polygonFloor->set_altitudemode(kmldom::ALTITUDEMODE_ABSOLUTE);
		polygonTop->set_altitudemode(kmldom::ALTITUDEMODE_ABSOLUTE);

		kmldom::PlacemarkPtr placemarkFloor = kmlFactory->CreatePlacemark();
		kmldom::PlacemarkPtr placemarkTop = kmlFactory->CreatePlacemark();

		placemarkFloor->set_name(airspace["name"].GetString());
		placemarkFloor->set_geometry(polygonFloor);

		placemarkTop->set_name(airspace["name"].GetString());
		placemarkTop->set_geometry(polygonTop);

		SetAirspaceStyle(placemarkFloor, airspace["airclass"].GetString());
		SetAirspaceStyle(placemarkTop, airspace["airclass"].GetString());

		placemarkFloor->set_description(description);
		placemarkTop->set_description(description);

		airspaceFolder->add_feature(placemarkFloor);
		airspaceFolder->add_feature(placemarkTop);

		int numCoordinates = coordinates.size();
		for (int i = 0; i < numCoordinates; i++)
		{
			if (i != numCoordinates - 1) {
				BuildPolygonSides(airspaceFolder, airspace["airclass"].GetString(), coordinates[i], coordinates[i + 1], lowerLimitM, upperLimitM);
			} else
			{
				BuildPolygonSides(airspaceFolder, airspace["airclass"].GetString(), coordinates[i], coordinates[0], lowerLimitM, upperLimitM);
			}
		}
		
	}
	else
	{
		kmldom::PolygonPtr polygonTop = BuildPolygonTopBottom(coordinates, topAltitude);

		polygonTop->set_extrude(true);
		polygonTop->set_tessellate(true);
		polygonTop->set_altitudemode(kmldom::ALTITUDEMODE_RELATIVETOGROUND);

		kmldom::PlacemarkPtr placemark = kmlFactory->CreatePlacemark();
		placemark->set_name(airspace["name"].GetString());
		placemark->set_geometry(polygonTop);

		SetAirspaceStyle(placemark, airspace["airclass"].GetString());
		placemark->set_description(description);

		airspaceFolder->add_feature(placemark);
	}

	folderPtrMap[channel]->add_feature(airspaceFolder);
}

void KmlCreator::CreateKml(string fileName)
{
	assert(!fileName.empty());

	for (auto folder : folderPtrMap)
	{
		kmlDocument->add_feature(folder.second);
	}

	kmldom::KmlPtr kml = kmlFactory->CreateKml();
	kml->set_feature(kmlDocument);

	std::ofstream kmlFileStream(fileName, std::ios::out);
	kmlFileStream << kmldom::SerializePretty(kml);

	kmlFileStream.close();
}
