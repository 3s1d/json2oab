/*
 * activationtime.h
 *
 *  Created on: Mai 3, 2019
 *      Author: Stefan Seifert
 */


#pragma once

#include <ctime>

class ActivationTime
{
public:
	time_t startActivation;
	time_t endActivation;

	ActivationTime();
	~ActivationTime();
};