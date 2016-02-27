#pragma once

#include "effects/SwitchEffect.h"

class SimpleEffect : public SwitchEffect
{

public:
	SimpleEffect(EffectHandlerFunction Fn): SwitchEffect(Fn) {};

	bool InitVars()
	{
		addVar(new Variable<uint8_t>("brightness", 255));
		addVar(new Variable<RgbColor>("color1", RgbColor(0)));
	}

	inline RgbColor color() { return getVar<RgbColor>("color1"); }
	inline uint8_t brightness() { return getVar<uint8_t>("brightness"); }

};