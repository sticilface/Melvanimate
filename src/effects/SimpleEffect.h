#pragma once

#include "effects/SwitchEffect.h"

class SimpleEffect : public SwitchEffect
{

public:
	SimpleEffect(EffectHandlerFunction Fn): SwitchEffect(Fn) {};

	bool InitVars()
	{
		addVar(new Variable<uint8_t>("brightness"));
		addVar(new Variable<RgbColor>("color1"));
	}

	RgbColor color() { return getVar<RgbColor>("color1"); }
	uint8_t brightness() { return getVar<uint8_t>("brightness"); }

};