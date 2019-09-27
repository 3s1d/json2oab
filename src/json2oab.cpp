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
#include <experimental/filesystem>


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
	cerr << "json to open air binary (OAB) converter" << endl;

	if (argc != 2 || !boost::filesystem::is_directory(argv[1]))
	{
		cerr << "usage: " << argv[0] << " Json Folder " << endl;
		return 1;
	}
	boost::filesystem::path jsonDirectory = argv[1];

	// note: no subfolders supported
	vector<string> files;
	read_directory(jsonDirectory, files);

	JsonParser jsonAirspaceParser;

	for(const auto& file : files)
	{
		if(!boost::iends_with(file, ".json"))
			continue;

		boost::filesystem::path pathToFile = string(argv[1]);
		
		pathToFile.append(file);
		std::cout << "Converting file: " << pathToFile << std::endl;
		

		jsonAirspaceParser.Parse(pathToFile.string());
	}

	jsonAirspaceParser.WriteOba("world.oab");

	return 0;
}



