/*
 * otb.hpp
 *
 *  Created on: 17 Mar 2021
 *      Author: sid
 */

#ifndef OTB_HPP_
#define OTB_HPP_

#include <vector>

#include "oab.hpp"

/*
 * Format:
 * +++++++++++++++++++++++++++++++
 * 3byte char      'OTB'
 * 1byte uint8_t   version number, currently 1
 * 8byte time_t	   built time
 *
 * Activation times:
 * 4byte  uint32_t airId
 * 1byte  uint8_t number of activations
 * 1byte  uint8_t magic 0xAA
 * --------------------------------------
 * 6bytes
 *
 * Activation times
 * time_t start activation
 * time_t end activation
 * --------------------------
 * 16 Bytes
 *
 *
 *
 */

class OTB
{
public:
#ifdef _WIN32
#pragma pack(1)
#endif
 	typedef struct
	{
 		/* id */
		uint32_t airId;

 		/* activation times */
		uint8_t numTimes;

		uint8_t magic;
#ifdef _WIN32
	} oab_header_t;
#pragma pack()
#else
	} __attribute__((packed)) otb_header_t;
#endif

	static const char* id;

private:

	otb_header_t hdr;
public:
	OTB(uint32_t airId, uint16_t numActs)
	{
		hdr.airId = airId;
		hdr.numTimes = numActs;
		hdr.magic = 0xAA;
	}

	static void writeFileHeader(std::ofstream &file);
	static void writeFileHeader(std::ofstream& file, time_t buildTime);

	void writeHeader(std::ofstream *file);
};



#endif /* OTB_HPP_ */
