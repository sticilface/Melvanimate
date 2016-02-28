#pragma once

#include <functional>
#include <ArduinoJson.h>
#include "EffectHandler.h"

extern const char * PRESETS_FILE;

/* ------------------------------------------------------------------------
	Effect Mangager
	This is the base class for managing effects
--------------------------------------------------------------------------*/
class EffectHandler; 

class EffectManager
{

public:
	EffectManager();
	~EffectManager() {};

	bool Add(const char * name, EffectHandler* Handler, bool animations, bool defaulteffect = false);
//	void SetTimeout(uint32_t time);
//	void SetTimeout(const char * name, uint32_t time);

	EffectHandler* Start();
	EffectHandler* Start(const char * name);
	EffectHandler* Start(const String name) { Start(name.c_str()); };
	void Refresh() ;
	bool Next() ;
	bool Previous() {}; //  need to think about how to imlement this... store previous like next maybe...

	bool Stop() ;
	bool Pause() ;
	void Loop();

	void setWaitFn(std::function<bool()> Fn ) { _waitFn = Fn; } //  allow effects to wait until triggering next...

	EffectHandler* Current() const
	{
		if (_NextInLine) {
			return _NextInLine;
		} else {
			return _currentHandle;
		}
	};

	const uint16_t total() const { return _count;}
	const char* getName(uint8_t i);
	//const char* getName();

	bool Save(uint8_t ID, const char * name, bool overwrite = false);

	bool Load(uint8_t ID);
	//  Load needs to be moved to the effecthandler... so that the manager can just set the flag..
	//  load this preset, then the effect can do it when it is ready... so async...


	bool removePreset(uint8_t ID);
	uint8_t nextFreePreset(JsonObject & root);

	// fetches info from SPIFFS valid presests for current effect
	bool getPresets(EffectHandler* handle, uint8_t& numberofpresets, uint8_t *& presets, char **& preset_names );
	void addAllpresets(DynamicJsonBuffer& jsonBuffer, JsonObject & root); 

//	bool SetToggle(const char * name);

	// maybe move this into a helper header file....
	static bool convertcolor(JsonObject & root, const char * colorstring);
	static bool parsespiffs(char *& data, DynamicJsonBuffer& jsonBuffer, JsonObject *& root, const char * file);


	uint8_t _numberofpresets = 0;
	uint8_t * _presets = nullptr;
	char ** _preset_names = nullptr;

protected:

	EffectHandler*  _currentHandle;
	EffectHandler*  _firstHandle;
	EffectHandler*  _lastHandle;
	EffectHandler*  _NextInLine;
	EffectHandler*  _toggleHandle;
	EffectHandler*  _defaulteffecthandle; 

	uint16_t _count;
private:
	std::function<bool()>  _waitFn = nullptr;

	// hold a 'new' array of elegible presets for _currenthandler
	EffectHandler* _findhandle(const char * handle);

};




/* ------------------------------------------------------------------------
								Attempt at SUB Template for settings...
--------------------------------------------------------------------------*/







/*

class MarqueeEffect : public SwitchEffect, public Color_property, public Brightness_property, public Palette_property, public Speed_property
{

public:
	MarqueeEffect(EffectHandlerFunction Fn) : SwitchEffect(Fn), _rotation(0), Color_property(this), Brightness_property(this), Palette_property(this), Speed_property(this)
	{
		_marqueeText = strdup("MELVANIMATE");
	};
	~MarqueeEffect()
	{
		free(_marqueeText);
		Serial.println("[~MarqueeEffect] marquee txt freed");
	}

	//  These functions just need to add and retrieve preset values from the json.
	bool load(JsonObject& root, const char *& ID) override ;

	bool addEffectJson(JsonObject& settings) override;
	bool parseJsonEffect(JsonObject& root) override;

	//  Specific Variables
//	void setBrightness(uint8_t bri)   {   _brightness = bri;  }
//	uint8_t getBrightness()  {    return _brightness;  }

//	void setColor(RgbColor color)   { _color = color;  }
//	RgbColor getColor()  {  return  _color;}

//	void setSpeed(uint8_t speed)   {   _speed = speed; }
//	uint8_t getSpeed()  {   return _speed;  }

	void setRotation(uint8_t rotation)   {   _rotation = rotation;}
	uint8_t getRotation()  {   return _rotation; }

	void setText(const char * text)   {   free(_marqueeText); _marqueeText = strdup(text);  }
	const char * getText()  { return _marqueeText; }

	//void setPalette(palette_type palette)   {   _palette = palette;  }
	Palette * getPalette()  {   return &_palette;  }
	//uint32_t _speed;

private:
	char * _marqueeText;
	//uint8_t _brightness;
	//palette_type _palette;
	uint8_t _rotation;
//	RgbColor _color;
	//Palette _palette;
};





class DummyEffect : public SwitchEffect
{

public:
	DummyEffect(EffectHandlerFunction Fn);

	//  These functions just need to add and retrieve preset values from the json.
	bool load(JsonObject& root, const char *& ID) override;
	bool addEffectJson(JsonObject& settings) override;
	bool parseJsonEffect(JsonObject& root) override;

private:
	Palette _palette;

};


class CascadeEffect : public SwitchEffect, public Color_property, public Brightness_property, public Palette_property
{

public:
	CascadeEffect(EffectHandlerFunction Fn): SwitchEffect(Fn), Color_property(this), Brightness_property(this),
		Palette_property(this)
	{};

	// bool addEffectJson(JsonObject& settings) override
	// {
	// 	Serial.printf("[CascadeEffect::addJson] Called\n");
	// 	Serial.println("[CascadeEffect::addJson] root");
	// 	settings.prettyPrintTo(Serial);
	// 	Serial.println();
	// }

//	bool parseJsonEffect(JsonObject& root) override;

	// bool testFn()
	// {
	// 	_color = RgbColor(0, 0, 0);
	// 	_brightness = 255;
	// }

private:
//	Palette _palette;

};


*/

//virtual bool parseJsonEffect(JsonObject & root) { return false;} // use json so it can be used with MQTT etc...
//virtual bool addEffectJson(JsonObject& settings) { return false; };

//	bool parseJsonEffectM(JsonObject & root) ;  // use json so it can be used with MQTT etc...
//	bool addEffectJsonM(JsonObject& root) ;


// class Effect2: public EffectHandler
// {
// public:
// 	Effect2()
// 	{
// 		_manager.addVar(new Variable<int>("int"));
// 		_manager.addVar(new Variable<RgbColor>("color1"));
// 		_manager.addVar(new Variable<RgbColor>("color2"));
// 		_manager.addVar(new Variable<RgbColor>("color3"));
// 		_manager.addVar(new Variable<RgbColor>("color4"));
// 		_manager.addVar(new Variable<RgbColor>("color5"));
// 	}

// 	//  required as inherited functions do not override
// 	bool addEffectJson(JsonObject & root) override  { return _manager.addEffectJsonM(root); }
// 	bool parseJsonEffect(JsonObject & root) override { return _manager.parseJsonEffectM(root); }

// 	void run()
// 	{
// 		int a = _manager.getVar<int>("int");
// 		RgbColor b = _manager.getVar<RgbColor>("color1");
// 	}
// private:
// 	PropertyManager _manager;
// };




