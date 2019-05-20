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