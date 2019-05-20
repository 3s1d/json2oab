/*
 * oab.hpp
 *
 *  Created on: Aug 11, 2018
 *      Author: sid
 */

#ifndef SRC_OAB_HPP_
#define SRC_OAB_HPP_

#define _USE_MATH_DEFINES
#include <cmath>

//todo type mapping! enum!

/*
 * Format:
 * 3byte char      'OAB'
 * 1byte uint8_t   version number, currently 1
 * 8byte time_t	   built time
 *
 * Airspace:
 * Header:
 * 31byte char     name
 * 1byte  char     type
 * 2byte  int16_t  top altitude ft
 * 2byte  int16_t  bottom altitude ft
 * 4byte  float    top (latitude rad)
 * 4byte  float    bottom (latitude rad)
 * 4byte  float    left (longitude rad)
 * 4byte  float    right (longitude rad)
 * 2byte  uint16_t  flags
 * 2byte  uint16_t number of polygons
 * 2byte  uint16_t number of activation times
 * 2byte  uint16_t [RESERVED]
 * --------------------------------------
 * 60bytes
 *
 * Polygon(only, last closing point is not present):
 * 4byte float latitude rad
 * 4byte float longitude rad
 * --------------------------
 * 8bytes
 * 
 * Activation times
 * time_t start activation
 * time_t end activation
 * --------------------------
 * 16 Bytes
 */

#include <vector>

#include "coord.hpp"
#include <stdint.h>
#include <string>


#define deg2rad(angleDegrees) (angleDegrees * M_PI / 180.0)
#define rad2deg(angleRadians) (angleRadians * 180.0 / M_PI)

#define OAB_ALTREF_GND			0x0001
#define OAB_ALTREF_MSL			0x0002
#define OAB_ALTREF_FL			0x0003

#define OAB_ALTREF_BOTTOM_OFFSET	0
#define OAB_ALTREF_TOP_OFFSET		3		//1bit reserved

#define OAB_MAGICNUMBER_FLAG		0xA000

class OAB
{
public:
	typedef enum : char
	{
		CLASSA = 'A',
		CLASSB = 'B',
		CLASSC = 'C',
		CLASSD = 'D',
		CLASSE = 'E',
		CLASSF = 'F',
		CLASSG = 'G',
		DANGER = 'Q',
		PROHIBITED = 'P',
		RESTRICTED = 'R',
		CTR = 'c',
		TMA = 'a',
		TMZ = 'z',
		RMZ = 'r',
		ATZ = 't',
		FIR = 'f', 			// from here on we drop everything
		UIR = 'u',
		OTH = 'o',
		GLIDING = 'g',
		NOGLIDER = 'n',
		WAVE = 'W',
		UNKNOWN = '?', 			// "UNKNOWN" can be used in OpenAir files
		UNDEFINED = '?',		// also the last one
		IGNORE = '-'
	} airspace_type_t;

	static const char* id;

#ifdef _WIN32
#pragma pack(1)
#endif
 	typedef struct
	{
		/* name */
		char name[31];
		airspace_type_t type;

		/* altitude */
		int16_t altitudeTop_ft;
		int16_t altitudeBottom_ft;

		/* bounding box */
		float topLat_rad;
		float bottomLat_rad;
		float leftLon_rad;
		float rightLon_rad;

		uint16_t flags;
		uint16_t numPoly;
		uint16_t numberOfActivationTimes;
		uint16_t reservedUnused;
#ifdef _WIN32
	} oab_header_t;
#pragma pack()
#else
	} __attribute__((packed)) oab_header_t;
#endif


	typedef struct
	{
		float lat_rad;
		float lon_rad;
	} oab_edge_t;

	typedef struct
	{
		time_t startActivationZulu;
		time_t endActivationZulu;
	} oab_activationTimes_t;

private:
	void finalize(void);
	void add2RadVec(Coord &coord);

	/* last position */
	Coord last_p;

	/* last inored position */
	Coord last_ign_p;
	bool last_ign;

public:
	oab_header_t header = {{0}};
	std::vector<oab_edge_t> polygon;	//rad
	std::vector<oab_activationTimes_t> activationTimes;
	float polygonSample_dist = 0.0f;

	void reset(void);
	void setName(std::string &name);
	void add(Coord &coord);

	
	void write(std::ofstream &file);

	static void writeFileHeader(std::ofstream& file);
	static void writeFileHeader(std::ofstream& file, time_t buildTime);
};

#endif /* SRC_OAB_HPP_ */