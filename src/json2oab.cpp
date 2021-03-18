/*
 * json2oab.cpp
 *
 *  Created on: May 3, 2019
 *      Author: Stefan Seifert
 */


#include <iostream>
#include <vector>
#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include "rapidjson/document.h"
#include "jsonparser.h"
//#include <experimental/filesystem>


using namespace std;

struct path_leaf_string
{
    std::string operator()(const boost::filesystem::directory_entry& entry) const
    {
        return entry.path().leaf().string();
    }
};

void read_directory(const boost::filesystem::path& path, vector<string>& v)
{
	boost::filesystem::directory_iterator start(path);
	boost::filesystem::directory_iterator end;
	std::transform(start, end, std::back_inserter(v), path_leaf_string());
}

int main(int argc, char* argv[])
{
	/* init + paramter */
	cerr << "json to open air binary (OAB) converter" << endl;
	if (argc < 2 || argc > 3 || !boost::filesystem::is_directory(argv[1]))
	{
		cerr << "usage: " << argv[0] << " JsonFolder [outFolder4Split]" << endl;
		return 1;
	}
	boost::filesystem::path jsonDirectory = argv[1];
	bool split = argc >= 3;
	string subFolder = string(argc>=3 ? (string(argv[2])+"/") : "");

	/* remove old files */
	system(("exec rm -r " + subFolder + "*.oab").c_str());
	system(("exec rm -r " + subFolder + "world.otb").c_str());
	system(("exec rm -r " + subFolder + "world.oab.kml").c_str());

	/* scan all jsons */
	// note: no subfolders supported
	vector<string> files;
	read_directory(jsonDirectory, files);

	JsonParser jsonAirspaceParser;

	/* prepare extern activation times writter */
	std::ofstream otbFile(subFolder + "world.otb", std::ios::out | std::ios::binary);
	OTB::writeFileHeader(otbFile);

	/* main loop */
	for(const auto& file : files)
	{
		if(!boost::iends_with(file, ".json") || boost::starts_with(file, "."))
			continue;

		/* build source filename */
		boost::filesystem::path pathToFile = string(argv[1]);
		pathToFile.append(file);
		std::cout << "Converting file: " << pathToFile << std::endl;
		
		/* parse file and write in multi file scenario */
		jsonAirspaceParser.Parse(pathToFile.string());
		if(split == true)
			jsonAirspaceParser.WriteOab(subFolder, &otbFile);
	}

	/* activation time writter */
	otbFile.close();

	/* write all airspace and activation times into a single file */
	if(split == false)
	{
		remove((subFolder + "world.otb").c_str());

		jsonAirspaceParser.WriteOab("world.oab");
	}

	/* create KML */
	jsonAirspaceParser.kmlCreator.CreateKml(subFolder + "world.oab.kml");

	return 0;
}



