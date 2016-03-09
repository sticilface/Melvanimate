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


class SimpleEffect2 : public SwitchEffect
{

public:
	SimpleEffect2(EffectHandlerFunction Fn): SwitchEffect(Fn) {};

	bool InitVars()
	{
		addVar(new Variable<uint8_t>("brightness", 100));
		addVar(new Variable<RgbColor>("color1", RgbColor(0) ) );
		addVar(new Variable<uint8_t>("speed", 30 ) );
		addVar(new Variable<Palette*>("Palette")); 
	}

	inline  RgbColor color()  { return getVar<RgbColor>("color1"); }
	inline  uint8_t brightness()  { return getVar<uint8_t>("brightness"); }
	inline  uint8_t speed() { return getVar<uint8_t>("speed"); }
	inline  Palette & palette() { return *getVar<Palette*>("Palette"); }

};