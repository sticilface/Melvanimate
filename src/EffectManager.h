#pragma once

#include <functional>
#include <memory>

#include <NeoPixelBus.h>
#include <ArduinoJson.h>
#include "palette.h"
#include <FS.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

//#include "VariablePropertiesHandler.h"


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

	bool newSave(uint8_t ID, const char * name, bool overwrite = false);
	bool newLoad(uint8_t ID);
	bool removePreset(uint8_t ID);
	uint8_t nextFree(JsonObject & root);

	// fetches info from SPIFFS valid presests for current effect
	bool getPresets(EffectHandler* handle, uint8_t& numberofpresets, uint8_t *& presets, char **& preset_names );

	bool SetToggle(const char * name);

	// maybe move this into a helper header file....
	static bool convertcolor(JsonObject & root, const char * colorstring);


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
	bool _parsespiffs(char *& data, DynamicJsonBuffer& jsonBuffer, JsonObject *& root, const char * file);
	EffectHandler* _findhandle(const char * handle);

};

/* ------------------------------------------------------------------------
					Effect Handler
					Dummy implementation (required)
--------------------------------------------------------------------------*/

class EffectHandler
{

public:
	virtual ~EffectHandler() {};
	virtual bool Run() {return false; }
	virtual bool Start() {return false; }
	virtual bool Stop() {return false; }
	virtual bool Pause() {return false; }
	virtual void Refresh() {}
	virtual void SetTimeout(uint32_t) {}

	bool parseJson(JsonObject & root);
	virtual bool parseJsonEffect(JsonObject & root) { return false;} // use json so it can be used with MQTT etc...

	// needs to NOT be virtual... call parsejson instead... but then call a virtual member..
	bool load(JsonObject& root, const char *& ID) { return false ; };

	bool addJson(JsonObject& settings); // called first, iterates through 'installed properties' then calls addEffectJson
	virtual bool addEffectJson(JsonObject& settings) { return false; };

	// save does NOT have to be overridden.  it calls addJson instead.
	virtual bool save(JsonObject& root, const char *& ID, const char * name);
	virtual uint8_t getPreset() { return _preset; }

//  Properties stuff

	bool installProperty(PropertyHandler* ptr);
	PropertyHandler * getPropertyPtr() { return _propertyPtr; }

//  Template property stuff

//	bool installVariableProperty(PropertyHandlerVariable * ptr);

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

	uint8_t _preset = 255;  // no current preset

private:
	EffectHandler* _next = nullptr;
	PropertyHandler * _propertyPtr = nullptr;
//	PropertyHandlerVariable * _propertyVariablePtr = nullptr;
	const char * _name;


	// bool _wait = false;
	// bool _autowait = false;
protected:


};

/* ------------------------------------------------------------------------
					Property Classes

--------------------------------------------------------------------------*/

class PropertyHandler
{
public:
	virtual bool addJsonProperty(JsonObject & root) { return false; }
	virtual bool parseJsonProperty(JsonObject & root) { return false; }
	virtual const char * name() = 0;

	PropertyHandler* next() { return _next; }
	void next (PropertyHandler* next) { _next = next; }
private:
	PropertyHandler* _next = nullptr;
};











/*

-----------------------------------------------------------------------


*/

class Color_property : public PropertyHandler
{

public:
	Color_property(EffectHandler* ptr)
	{
		if (ptr) {
			ptr->installProperty(this);
		}
	}
	const char * name() { return _name; }

	bool addJsonProperty(JsonObject & root) override
	{
		JsonObject& color = root.createNestedObject(_name);
		color["R"] = _color.R;
		color["G"] = _color.G;
		color["B"] = _color.B;
		return true;
	}


	bool parseJsonProperty(JsonObject & root) override
	{
		bool changed = false;
		if (root.containsKey(_name)) {


			if (root[_name].is<const char*>() ) {
				Serial.printf("[Color_property::parseJsonProperty] Color converted from String\n");
				EffectManager::convertcolor(root, _name);
			}

			uint8_t R = root[_name]["R"].as<long>();
			uint8_t G = root[_name]["G"].as<long>();
			uint8_t B = root[_name]["B"].as<long>();

			if (_color.R != R) {
				_color.R = R;
				changed = true;
			}
			if (_color.G != G) {
				_color.G = G;
				changed = true;
			}
			if (_color.B != B) {
				_color.B = B;
				changed = true;
			}

			Serial.printf("[Color_property::parseJsonProperty] color1 (%u,%u,%u)\n", _color.R, _color.G, _color.B);

		}

		return changed;
	}

	RgbColor _color;
private:
	const char * _name = "color1";
};

class Brightness_property : public PropertyHandler
{

public:
	Brightness_property(EffectHandler* ptr)
	{
		if (ptr) {
			ptr->installProperty(this);
		}
	}
	const char * name() { return _name; }
	bool addJsonProperty(JsonObject & root) override
	{
		root[_name] = _brightness;
		return true;
	}
	bool parseJsonProperty(JsonObject & root) override
	{
		if (root.containsKey(_name)) {
			if (_brightness != root[_name] ) {
				_brightness = root[_name];
				return true;
			}
		}
		return false;
	}

	uint8_t _brightness;
private:
	const char * _name = "brightness";
};

class Palette_property : public PropertyHandler
{

public:
	Palette_property(EffectHandler* ptr)
	{
		if (ptr) {
			ptr->installProperty(this);
		}
	}
	const char * name() { return _name; }
	bool addJsonProperty(JsonObject & root) override
	{
		return _palette.addJson(root);
	}
	bool parseJsonProperty(JsonObject & root) override
	{
		if (root.containsKey(_name)) {
			return _palette.parseJson(root);
		} else {
			return false;
		}
	}

	Palette _palette;
private:
	const char * _name  = "palette";
};


class Speed_property : public PropertyHandler
{

public:
	Speed_property(EffectHandler* ptr)
	{
		if (ptr) {
			ptr->installProperty(this);
		}
	}
	const char * name() { return _name; }
	bool addJsonProperty(JsonObject & root) override
	{
		root[_name] = _speed;
		return true;
	}
	bool parseJsonProperty(JsonObject & root) override
	{
		if (root.containsKey(_name)) {
			if (_speed != root[_name] ) {
				_speed = root[_name];
				return true;
			}
		}
		return false;
	}

	uint32_t _speed;
private:
	const char * _name = "speed";
};






// typedef std::function<void(void)> EffectHandlerFunction;

// class Effect : public EffectHandler
// {

// public:
// 	Effect(EffectHandlerFunction Fn) : _Fn(Fn) {};
// 	bool Run() override { _Fn();};
// private:
// 	EffectHandlerFunction _Fn;
// };

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
								Unused Empty Handler
--------------------------------------------------------------------------*/

class ComplexEffect : public EffectHandler
{

public:
	typedef std::function<void(char *, char*, double)> EffectHandlerFunction;
	ComplexEffect(EffectHandlerFunction Fn): _Fn(Fn) {};
	bool Run() override {};
	bool Start() override { Serial.println("Starting Complex effect"); };
	bool Stop() override {};
	bool Pause() override {};

private:
	const char * _name;
	EffectHandlerFunction _Fn;
};

/* ------------------------------------------------------------------------
								Attempt at SUB Template for settings...
--------------------------------------------------------------------------*/

class SimpleEffect : public SwitchEffect, public Color_property, public Brightness_property
{

public:
	SimpleEffect(EffectHandlerFunction Fn) : SwitchEffect(Fn), Color_property(this), Brightness_property(this) {};

	//  These functions just need to add and retrieve preset values from the json.
//	bool load(JsonObject& root, const char *& ID) override;
	//bool addEffectJson(JsonObject& settings) override;
	//bool args(ESP8266WebServer& HTTP) override;
	//bool parseJsonEffect(JsonObject& root) override;

	// void setBrightness(uint8_t bri)   {   _brightness = bri; Refresh(); }
	// uint8_t getBrightness()  { return _brightness; }

	// void setColor(RgbColor color)   { _color = color; Refresh(); }
	// RgbColor getColor()  {  return _color;  }


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

class Effect2: public PropertyManager, public EffectHandler
{
public:
	Effect2()
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
		heapv = heapv - ESP.getFreeHeap(); 
		Serial.printf("**[Effect2:init] heap used %u", heapv);

	}

	//  required as inherited functions do not override 
	bool addEffectJson(JsonObject & root) override  { return PropertyManager::addEffectJson(root); }
	bool parseJsonEffect(JsonObject & root) override { return PropertyManager::parseJsonEffect(root); }

  bool Run() override
  {
    if ( millis() - _timer > 10000) {
      RgbColor color = getVar<RgbColor>("color1");
      uint8_t brightness = getVar<uint8_t>("brightness");
      uint8_t speed = getVar<uint8_t>("speed");

      Serial.printf("[testclass::Run] bri = %u, col = (%u,%u,%u)\n", brightness, color.R, color.G, color.B);

      _timer = millis();
    }
  };
  bool Start() override
  {
    Serial.println("[Effect2::Start]");
  };
  bool Stop() override
  {
    Serial.println("[Effect2::Stop]");
  };
  void Refresh() override
  {
    Serial.println("[Effect2::Refresh]");
  };
private:
	uint32_t _timer = 0; 
};


