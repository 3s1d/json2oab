/*
 * kmlcreator.hpp
 *
 *  Created on: Mai 13, 2019
 *      Author: Stefan Seifert
 */

#pragma once
#include <kml/dom.h>
#include "oab.hpp"
#include <rapidjson/document.h>

class KmlCreator
{
private:
	kmldom::KmlFactory* kmlFactory;
	kmldom::DocumentPtr kmlDocument;
	std::map<string, kmldom::FolderPtr> folderPtrMap;
	void CreateStyle(std::string airClass, const char lineABGR[], const char polyABGR[]);
	void BuildPolygonSides(kmldom::FolderPtr airspaceFolder, std::string airspaceClass, Coord coordinate1, Coord coordinate2, double floorAltitude, double topAltitude);
	kmldom::PolygonPtr BuildPolygonTopBottom(std::vector<Coord> coordinates, double altitude);
	void SetAirspaceStyle(kmldom::PlacemarkPtr placemark, std::string airspaceClass);
	std::string CreateDescription(std::string airspaceClass, float floorLimitM, float topLimitM);

public:
	KmlCreator();
	~KmlCreator();

	void CreateKml(std::string fileName);
	void AddAirspace(std::string channel,
		std::string countryDescription,
		rapidjson::Value& RapidJson,
		std::vector<Coord> coordinates, 
		double floorAltitude, 
		double topAltitude);
};
