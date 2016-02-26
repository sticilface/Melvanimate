#pragma once

#include "EffectHandler.h"
#include "e131/_E131.h"

// extern const IPAddress multicast_ip_addr; // Multicast broadcast address
// extern const uint16_t UDPlightPort;
// extern E131* e131;


class DMXEffect : public EffectHandler
{

public:
	DMXEffect() {};

	bool InitVars()
	{
		addVar(new Variable<uint8_t>("universe"));
		addVar(new Variable<uint8_t>("ppu"));
		addVar(new Variable<uint8_t>("channel_start"));
		addVar(new Variable<const char *>("marqueetext"));
		//addVar(new Variable<Palette*>("Palette"));

	}

	RgbColor color() { return getVar<RgbColor>("color1"); }
	uint8_t brightness() { return getVar<uint8_t>("brightness"); }

private:

//	const IPAddress multicast_ip_addr(224, 0, 0, 0); // Multicast broadcast address
	const uint16_t UDPlightPort = 8888;
	E131* e131 = nullptr;


};