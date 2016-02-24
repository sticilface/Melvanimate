#pragma once

#include <functional>
#include <memory>

#include <NeoPixelBus.h>
#include <ArduinoJson.h>
#include "palette.h"
#include <FS.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

extern const char * PRESETS_FILE;
/* ------------------------------------------------------------------------
	Effect Mangager
	This is the base class for managing effects
--------------------------------------------------------------------------*/

class EffectHandler;
class PropertyHandler;

#include "PropertyManager.h"


class EffectManager
{

public:
	EffectManager();
	~EffectManager() {};

	bool Add(const char * name, EffectHandler* Handler);
	void SetTimeout(uint32_t time);
	void SetTimeout(const char * name, uint32_t time);

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
	uint8_t nextFree(JsonObject & root);

	// fetches info from SPIFFS valid presests for current effect
	bool getPresets(EffectHandler* handle, uint8_t& numberofpresets, uint8_t *& presets, char **& preset_names );

	bool SetToggle(const char * name);

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
	uint16_t _count;
	//const char * PresetsFile = PRESETS_FILE;
private:

	std::function<bool()>  _waitFn = nullptr;

	// hold a 'new' array of elegible presets for _currenthandler
	EffectHandler* _findhandle(const char * handle);

};

/* ------------------------------------------------------------------------
					Effect Handler
					Dummy implementation (required)
--------------------------------------------------------------------------*/

class EffectHandler: public PropertyManager
{

public:
	virtual ~EffectHandler() {};
	virtual bool Run() {return false; }
	virtual bool Start() {return false; }
	virtual bool Stop() {return false; }
	virtual bool Pause() {return false; }
	virtual void Refresh() {}
	virtual void SetTimeout(uint32_t) {}

	bool parseJson(JsonObject & root); // calls parseJsonEffect internally after calling propertymanager...
	virtual bool parseJsonEffect(JsonObject & root) { return false;} // allows JSON to be acted on within the effect

	// needs to NOT be virtual... call parsejson instead... but then call a virtual member..

	bool addJson(JsonObject& settings); // called first, iterates through 'installed properties' then calls addEffectJson
	virtual bool addEffectJson(JsonObject& settings) { return false; };

	// save does NOT have to be overridden.  it calls addJson instead.
	virtual bool save(JsonObject& root, const char *& ID, const char * name);

	uint8_t preset() { return _preset; }
	void preset(uint8_t preset) { _preset = preset; }

//  Core Very important...
	EffectHandler* next() { return _next; } //  ASK what is next
	void next (EffectHandler* next) { _next = next; } //  Set what is next
	void name (const char * name) { _name = name; }
	const char * name() {return _name; };

//  New waits handled in handler...
	//   NOT in use.. as it makes it hard for isAnimating to be used.  Callback is defo better.... it is not possible to
	//    know in the effect when the effect is about to finish.  Doing it this way makes it compatible with things like FadeTo...
	//		Alternatively i just make the effects handler dependent on neopixelbus which is currently is not.

	// void wait(bool wait) { _wait = wait; }
	// bool wait() { return _wait; }
	// void autoWait() {
	// 	_wait = true;
	// 	_autowait = true;
	// }


private:
	EffectHandler* _next = nullptr;
	const char * _name;
	uint8_t _preset = 255;  // no current preset

protected:


};


/* ------------------------------------------------------------------------
					Effect Handler SWITCH - MAIN effect handler....
					Handles most of my effects... (required)
--------------------------------------------------------------------------*/
enum effectState { PRE_EFFECT = 0, RUN_EFFECT, POST_EFFECT, EFFECT_PAUSED, EFFECT_REFRESH };


class SwitchEffect : public EffectHandler
{

public:
	typedef std::function<void(effectState&, EffectHandler* )> EffectHandlerFunction;
	SwitchEffect(EffectHandlerFunction Fn) : _Fn(Fn) {};
	bool Run() override
	{
		if (_state == PRE_EFFECT) {
			_Fn(_state, this);
			_state = RUN_EFFECT;
			return true;
		}

		if (_state == RUN_EFFECT) {
			if (millis() - _lastTick > _timeout) {
				_Fn(_state, this);
				_lastTick = millis();
			}
		}
	};
	virtual bool Start() override { _state = PRE_EFFECT ; _Fn(_state, this) ; _state = RUN_EFFECT; }
	virtual bool Stop() override { _state = POST_EFFECT; _Fn(_state, this); };
	virtual void SetTimeout (uint32_t timeout) override { _timeout = timeout; };
	virtual void Refresh()
	{
		_state = EFFECT_REFRESH;
		_Fn(_state, this) ;
		if ( _state == EFFECT_REFRESH ) { _state = RUN_EFFECT; }
	}


private:
	EffectHandlerFunction _Fn;
	effectState _state;
	uint32_t _lastTick = 0;
	uint32_t _timeout = 0;
};



/* ------------------------------------------------------------------------
								Attempt at SUB Template for settings...
--------------------------------------------------------------------------*/

class SimpleEffect : public SwitchEffect
{

public:
	SimpleEffect(EffectHandlerFunction Fn): SwitchEffect(Fn)
	{

		//addVar(new Variable<Palette*>("Palette"));
	};

	bool InitVars()
	{
		addVar(new Variable<uint8_t>("brightness"));
		addVar(new Variable<RgbColor>("color1"));
	}

	RgbColor color() { return getVar<RgbColor>("color1"); }
	uint8_t brightness() { return getVar<uint8_t>("brightness"); }
	//Palette palette() { return *(getVar<Palette*>("Palette")); }

};


class DMXEffect : public EffectHandler
{

public:
	DMXEffect()
	{

	};

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
	//Palette palette() { return *(getVar<Palette*>("Palette")); }

};


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

class AdalightEffect : public SwitchEffect
{

public:
	AdalightEffect(EffectHandlerFunction Fn);

	//  These functions just need to add and retrieve preset values from the json.
	bool load(JsonObject& root, const char *& ID) override;
	bool addEffectJson(JsonObject& settings) override;
	bool parseJsonEffect(JsonObject& root) override;

	bool setSerialspeed(uint32_t speed)  {  _serialspeed = speed; Refresh(); return true; }
	bool getSerialspeed(uint32_t& speed)  {  speed = _serialspeed; return true; }

private:
	uint32_t _serialspeed;

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

class Effect2: public EffectHandler
{
public:
	Effect2()
	{

	}

	bool InitVars() override
	{
		uint32_t heapv = ESP.getFreeHeap();
		addVar(new Variable<int>("int"));
		addVar(new Variable<uint8_t>("brightness"));
		addVar(new Variable<uint8_t>("speed"));
		addVar(new Variable<RgbColor>("color1"));
		addVar(new Variable<RgbColor>("color2"));
		addVar(new Variable<RgbColor>("color3"));
		addVar(new Variable<RgbColor>("color4"));
		addVar(new Variable<RgbColor>("color5"));
		addVar(new Variable<Palette*>("Palette"));
		addVar(new Variable<Palette*>("palette2"));
		addVar(new Variable<Palette*>("palette3"));
		addVar(new Variable<Palette*>("palette4"));
		addVar(new Variable<Palette*>("palette5"));
		addVar(new Variable<Palette*>("palette6"));
		addVar(new Variable<Palette*>("palette7"));
		addVar(new Variable<Palette*>("palette8"));
		addVar(new Variable<Palette*>("palette9"));
		addVar(new Variable<Palette*>("palette10"));
		addVar(new Variable<int>("int2"));
		addVar(new Variable<int>("int3"));
		addVar(new Variable<int>("int4"));
		addVar(new Variable<int>("int5"));
		addVar(new Variable<int>("int6"));
		addVar(new Variable<int>("int7"));
		addVar(new Variable<int>("int8"));
		addVar(new Variable<int>("int9"));
		addVar(new Variable<int>("int10"));

		heapv = heapv - ESP.getFreeHeap();
		Serial.printf("[Effect2:init] heap used %u\n", heapv);
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


