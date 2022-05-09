/*
 * oab.hpp
 *
 *  Created on: Aug 11, 2018
 *      Author: sid, Stefan Seifert
 */

#ifndef SRC_OAB_HPP_
#define SRC_OAB_HPP_

#define _USE_MATH_DEFINES
#include <cmath>

//todo type mapping! enum!

/*
 * OAB Format:
 * +++++++++++++++++++++++++++
 * 3byte char      'OAB'
 * 1byte uint8_t   version number, currently 4
 * 8byte time_t	   built time
 * 4byte float     top (latitude rad)
 * 4byte float     bottom (latitude rad)
 * 4byte float     left (longitude rad)
 * 4byte float     right (longitude rad)
 * 2byte uint16_t  #airspaces
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
 * 2byte  uint16_t flags
 * 2byte  uint16_t number of polygons
 * 4byte  uint32_t airId
  * --------------------------------------
 * 60bytes
 *
 * Additional bottom altitude (optional, only if OAB_ALTREF_ALTGND in OAB_ALTREF_BOTTOM_OFFSET is set)
 * 2byte int16_t   alternative bottom altitude ft
 * -------------------------
 * 2bytes
 *
 * Polygon (only, last closing point is not present):
 * 4byte float latitude rad
 * 4byte float longitude rad
 * --------------------------
 * 8bytes
 * 
 * Activation times (only if number of polygons > 0 and < 0x3F)
 * time_t start activation
 * time_t end activation
 * --------------------------
 * 16 Bytes
 *
 *
 *
 *
 *
 * External activation times file -> otb.hpp
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
#define OAB_ALTREF_ALTGND		0x0004		//only valid for bottom

#define OAB_ALTREF_BOTTOM_OFFSET	0		//bits 0..2
#define OAB_ALTREF_TOP_OFFSET		3		//bits 3..5	(1bit unused)
#define OAB_ALTREF_MASK				0x07
#define OAB_NUMACIVATIONS			6		//bits 6..11 	-> 0x3F means external  file
#define OAB_NUMACIVATIONS_MASK		0x3F

#define OAB_MAGICNUMBER_FLAG		0xA000		//bits 12..15

class OAB
{
public:
#ifdef _WIN32
#pragma pack(1)
#endif
	/* bounding box */
 	typedef struct
	{
		float topLat_rad;
		float bottomLat_rad;
		float leftLon_rad;
		float rightLon_rad;
#ifdef _WIN32
	} bb_t;
#pragma pack()
#else
	} __attribute__((packed)) bb_t;
#endif

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
		uint32_t airid;
#ifdef _WIN32
	} oab_header_t;
#pragma pack()
#else
	} __attribute__((packed)) oab_header_t;
#endif
	int16_t altitudeBottomAlt_ft = -1;

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
	void finalize(bool includeActivationTimes);
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

	
	void write(std::fstream &file, bool includeActivations = true);
	void writeActivations(std::ofstream *file);

	static void writeFileHeader(std::fstream& file, const OAB::bb_t &bb, uint16_t nElem);
	static void writeFileHeader(std::fstream& file, time_t buildTime, const OAB::bb_t &bb, uint16_t nElem);
	static bool readFileHeader(std::fstream &file, OAB::bb_t &bb, uint16_t &nElem);
};

#endif /* SRC_OAB_HPP_ */
