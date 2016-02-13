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

class EffectManager
{

public:
	EffectManager();
	~EffectManager() {};

	bool Add(const char * name, EffectHandler* Handler);
	void SetTimeout(uint32_t time);
	void SetTimeout(const char * name, uint32_t time);

	bool Start();
	bool Start(const char * name);
	bool Start(const String name) { Start(name.c_str()); };
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

	bool newSave(uint8_t ID);
	bool newLoad(uint8_t ID);
	bool removePreset(uint8_t ID);

	// fetches info from SPIFFS about which presest applies to current
	bool getPresets(EffectHandler* handle, uint8_t& numberofpresets, uint8_t *& presets, char **& preset_names );

	bool SetToggle(const char * name);

	static bool convertcolor(JsonObject & root, String colorstring); 


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

	virtual bool args(JsonObject & root) { return false;} // use json so it can be used with MQTT etc...

	// experimental
	virtual bool load(JsonObject& root, const char *& ID) { return false ; };
	virtual bool addJson(JsonObject& settings) { return false; };
	// save does NOT have to be overridden.  it calls addJson instead.
	virtual bool save(JsonObject& root, const char *& ID);
	virtual uint8_t getPreset() { return _preset; }

	virtual Palette * getPalette() { return nullptr; }

	EffectHandler* next() { return _next; } //  ASK what is next
	void next (EffectHandler* next) { _next = next; } //  Set what is next
	void name (const char * name) { _name = name; }
	const char * name() {return _name; };
private:
	EffectHandler* _next = nullptr;
	const char * _name;
protected:
	uint8_t _preset = 255;


};

/* ------------------------------------------------------------------------
					Effect Handler
					Dummy implementation (required)
--------------------------------------------------------------------------*/
typedef std::function<void(void)> EffectHandlerFunction;

class Effect : public EffectHandler
{

public:
	Effect(EffectHandlerFunction Fn) : _Fn(Fn) {};
	bool Run() override { _Fn();};
private:
	EffectHandlerFunction _Fn;
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

class GeneralEffect : public SwitchEffect
{

public:
	GeneralEffect(EffectHandlerFunction Fn) : SwitchEffect(Fn), _brightness(255), _color(0) {};

	//  These functions just need to add and retrieve preset values from the json.
	bool load(JsonObject& root, const char *& ID) override;
	bool addJson(JsonObject& settings) override;
	//bool args(ESP8266WebServer& HTTP) override;
	bool args(JsonObject& root) override;

	void setBrightness(uint8_t bri)   {   _brightness = bri; Refresh(); }
	uint8_t getBrightness()  { return _brightness; }

	void setColor(RgbColor color)   { _color = color; Refresh(); }
	RgbColor getColor()  {  return _color;  }

private:
	uint8_t _brightness;
	RgbColor _color;

};


class MarqueeEffect : public SwitchEffect
{

public:
	MarqueeEffect(EffectHandlerFunction Fn) : SwitchEffect(Fn), _brightness(255), _speed(5), _rotation(0)
	{
		_color = RgbColor(0, 0, 0);
		_marqueeText = strdup("MELVANIMATE");
	};
	~MarqueeEffect()
	{
		free(_marqueeText);
		Serial.println("[~MarqueeEffect] marquee txt freed");
	}

	//  These functions just need to add and retrieve preset values from the json.
	bool load(JsonObject& root, const char *& ID) override ;
	bool addJson(JsonObject& settings) override;
	bool args(JsonObject& root) override;

	//  Specific Variables
	void setBrightness(uint8_t bri)   {   _brightness = bri;  }
	uint8_t getBrightness()  {    return _brightness;  }

	void setColor(RgbColor color)   { _color = color;  }
	RgbColor getColor()  {  return  _color;}

	void setSpeed(uint8_t speed)   {   _speed = speed; }
	uint8_t getSpeed()  {   return _speed;  }

	void setRotation(uint8_t rotation)   {   _rotation = rotation;}
	uint8_t getRotation()  {   return _rotation; }

	void setText(const char * text)   {   free(_marqueeText); _marqueeText = strdup(text);  }
	const char * getText()  { return _marqueeText; }

	//void setPalette(palette_type palette)   {   _palette = palette;  }
	Palette * getPalette() override {   return &_palette;  }

private:
	char * _marqueeText;
	uint32_t _speed;
	uint8_t _brightness;
	//palette_type _palette;
	uint8_t _rotation;
	RgbColor _color;
	Palette _palette;
};

class AdalightEffect : public SwitchEffect
{

public:
	AdalightEffect(EffectHandlerFunction Fn);

	//  These functions just need to add and retrieve preset values from the json.
	bool load(JsonObject& root, const char *& ID) override;
	bool addJson(JsonObject& settings) override;
	bool args(JsonObject& root) override;

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
	bool addJson(JsonObject& settings) override;
	bool args(JsonObject& root) override;

private:
	Palette _palette;

};

