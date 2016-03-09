#pragma once

class Effect2: public EffectHandler
{
public:
	Effect2()
	{

	}

	bool InitVars() override
	{
		uint32_t heap = ESP.getFreeHeap();
		addVar(new Variable<int>("int"));
		addVar(new Variable<uint8_t>("brightness",255));
		addVar(new Variable<uint8_t>("speed",255));


		addVar(new Variable<MelvtrixMan>("matrix", new MelvtrixMan(1,2,3))); 
		addVar(new Variable<RgbColor>("color1"));
		addVar(new Variable<RgbColor>("color2"));
		addVar(new Variable<RgbColor>("color3"));
		addVar(new Variable<RgbColor>("color4"));
		addVar(new Variable<RgbColor>("color5"));
		addVar(new Variable<Palette*>("Palette"));
		// addVar(new Variable<Palette*>("palette2"));
		// addVar(new Variable<Palette*>("palette3"));
		// addVar(new Variable<Palette*>("palette4"));
		// addVar(new Variable<Palette*>("palette5"));
		// addVar(new Variable<Palette*>("palette6"));
		// addVar(new Variable<Palette*>("palette7"));
		// addVar(new Variable<Palette*>("palette8"));
		// addVar(new Variable<Palette*>("palette9"));
		// addVar(new Variable<Palette*>("palette10"));
		addVar(new Variable<int>("int2"));
		addVar(new Variable<int>("int3"));
		addVar(new Variable<int>("int4"));
		addVar(new Variable<int>("int5"));
		addVar(new Variable<int>("int6"));
		addVar(new Variable<int>("int7"));
		addVar(new Variable<int>("int8"));
		addVar(new Variable<int>("int9"));
		addVar(new Variable<int>("int10"));

	}

	bool Stop() override
	{
		Serial.println("[Effect2::Stop]");
	}

	void Refresh() override
	{
		Serial.println("[Effect2::Refresh]");
	}

private:
	uint32_t _timer = 0;
};