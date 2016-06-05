//   just gives a brightness to work with rgbw LEDS. 


#pragma once

#include "EffectHandler.h"

class White : public EffectHandler
{

public:
	White(){};

	bool InitVars()
	{
		addVar(new Variable<uint8_t>("brightness", 50)); // set default to 50, incase too much power
	}

	bool Start() override;

	void Refresh() override {
		Start(); 
	}

	inline uint8_t brightness() { return getVar<uint8_t>("brightness"); }
	
private:

};