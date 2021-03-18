/*
 * otb.cpp
 *
 *  Created on: 17 Mar 2021
 *      Author: sid
 */

#include <iostream>
#include <fstream>
#include <cstring>
#include <string>

#include "otb.hpp"

const char *OTB::id = "OTB\x1";

void OTB::writeFileHeader(std::ofstream &file)
{
	writeFileHeader(file, time(nullptr));
}

void OTB::writeFileHeader(std::ofstream& file, time_t buildTime)
{
	file.write(id, strlen(id));

	file.write((char*)& buildTime, sizeof(time_t));
}

void OTB::writeHeader(std::ofstream *file)
{
	/* write header */
	file->write((char *) &hdr, sizeof(otb_header_t));
}
