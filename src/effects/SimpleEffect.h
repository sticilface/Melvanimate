#pragma once

#include "Palette.h"
#include "SwitchEffect.h"

class SimpleEffect : public SwitchEffect
{

public:
	SimpleEffect(EffectHandlerFunction Fn): SwitchEffect(Fn) {};

	bool InitVars()
	{
		addVar(new Variable<uint8_t>("brightness", 50));
		addVar(new Variable<RgbColor>("color1", RgbColor(0)));
		addVar(new Variable<Palette*>("Palette", Palette::OFF));
		return true;
	}

	inline RgbColor color() { return getVar<RgbColor>("color1"); }
	inline uint8_t brightness() { return getVar<uint8_t>("brightness"); }
	inline Palette * palette() { return getVar<Palette*>("Palette"); }

};



class SimpleEffect2 : public SwitchEffect
{

public:
	SimpleEffect2(EffectHandlerFunction Fn): SwitchEffect(Fn) {};

	bool InitVars()
	{
		addVar(new Variable<uint8_t>("brightness", 50));
		addVar(new Variable<RgbColor>("color1", RgbColor(0) ) );
		addVar(new Variable<uint8_t>("speed", 30 ) );
		addVar(new Variable<Palette*>("Palette"));
	}

	inline  RgbColor color()  { return getVar<RgbColor>("color1"); }
	inline  uint8_t brightness()  { return getVar<uint8_t>("brightness"); }
	inline  uint8_t speed() { return getVar<uint8_t>("speed"); }
	inline  Palette * palette() { return getVar<Palette*>("Palette"); }

};
