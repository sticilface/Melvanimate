#pragma once

#include "EffectHandler.h"
#include <NeoPixelAnimator.h>

class SunRise : public EffectHandler
{

public:
	SunRise(){};

	bool InitVars()
	{
		addVar(new Variable<uint8_t>("brightness", 255)); // set default to 50, incase too much power
		addVar(new Variable<uint8_t>("sun_time", 1)); 
	}

	bool Start() override;
	bool Run() override; 

	bool Stop() override; 

	void Refresh() override;

	inline uint8_t brightness() { return getVar<uint8_t>("brightness"); }
	inline uint8_t time() { return getVar<uint8_t>("sun_time"); }

private:
	uint32_t _endtime{0};
	uint32_t _starttime{0}; 
	float _progress{0}; 
	uint32_t _timeout{100};
	uint32_t _tick{0};
	AnimUpdateCallback _animUpdate; 
};