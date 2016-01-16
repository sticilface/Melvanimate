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

	EffectHandler* Current() const { 
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
	bool getPresets(EffectHandler* handle, uint8_t& numberofpresets, uint8_t *& presets);

	bool SetToggle(const char * name);



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
	uint8_t _numberofpresets = 0;
	uint8_t * _presets = nullptr;
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
	virtual bool args(ESP8266WebServer& HTTP) {return false;}

	// experimental
	virtual bool load(JsonObject& root, const char *& ID) { return false ; };
	virtual bool addJson(JsonObject& settings) { return false; };

	// save does NOT have to be overridden.  it calls addJson instead.
	virtual bool save(JsonObject& root, const char *& ID);

	// not being used... 
	virtual void* getp() { return nullptr; }

	// specific virtual functions for ALL effects...
	// If they are not handlesd by subclass, they return false.

	virtual bool setBrightness(uint8_t) { return false; }
	virtual bool getBrightness(uint8_t&) { return false;};

	virtual bool setColor(RgbColor ) { return false;}
	virtual bool getColor(RgbColor&) { return false;}

	virtual bool setColor2(RgbColor ) { return false;}
	virtual bool getColor2(RgbColor&) { return false;}

	virtual bool setSerialspeed(uint32_t speed) { return false;}
	virtual bool getSerialspeed(uint32_t& speed) { return false;}

	virtual bool setSpeed(uint8_t speed)   {   return false; }
	virtual bool getSpeed(uint8_t& speed)  {   return false; }

	virtual bool setRotation(uint8_t rotation)   {   return false; }
	virtual bool getRotation(uint8_t& rotation)  {   return false; }

	virtual bool setText(const char * text)   {   return false; }
	virtual bool getText( char *& text)   {   return false; }

	virtual bool setPalette(palette_type palette)   {   return false;  }
	virtual bool getPalette(palette_type& palette)  {   return false;  }

	//colours

	// virtual void Color(RgbColor color) {} // not in use...
	// virtual void Random() {}
	//virtual palette* Palette() {};

	EffectHandler* next() { return _next; } //  ASK what is next
	void next (EffectHandler* next) { _next = next; } //  Set what is next
	void name (const char * name) { _name = name; }
	const char * name() {return _name; };
private:
	EffectHandler* _next = nullptr;
	const char * _name;


};

/* ------------------------------------------------------------------------
					Effect Handler
					Dummy implementation (required)
--------------------------------------------------------------------------*/

class Effect : public EffectHandler
{

public:
	typedef std::function<void(void)> EffectHandlerFunction;
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
	typedef std::function<void(effectState&)> EffectHandlerFunction;
	SwitchEffect(EffectHandlerFunction Fn) : _Fn(Fn) {};
	bool Run() override
	{
		if (_state == PRE_EFFECT) {
			_Fn(_state);
			_state = RUN_EFFECT;
			return true;
		}

		if (_state == RUN_EFFECT) {
			if (millis() - _lastTick > _timeout) {
				_Fn(_state);
				_lastTick = millis();
			}
		}
	};
	bool Start() override { _state = PRE_EFFECT ; _Fn(_state) ; _state = RUN_EFFECT; }
	bool Stop() override { _state = POST_EFFECT; _Fn(_state); };
	void SetTimeout (uint32_t timeout) override { _timeout = timeout; };
	void Refresh()
	{
		_state = EFFECT_REFRESH;
		_Fn(_state) ;
		if ( _state == EFFECT_REFRESH ) _state = RUN_EFFECT;
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
	bool args(ESP8266WebServer& HTTP) override; 

	bool setBrightness(uint8_t bri) override  {   _brightness = bri; Refresh(); return true; }
	bool getBrightness(uint8_t& bri) override {   bri = _brightness; return true; }

	bool setColor(RgbColor color) override  { _color = color; Refresh(); return true; }
	bool getColor(RgbColor& color) override {  color = _color; return true; }

private:
	uint8_t _brightness;
	RgbColor _color;

};


class MarqueeEffect : public SwitchEffect
{

public:
	MarqueeEffect(EffectHandlerFunction Fn) : SwitchEffect(Fn), _brightness(255), _color(0), _speed(5), _palette(OFF), _rotation(0)
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
	bool addJson(JsonObject& settings) override  ;
	bool args(ESP8266WebServer& HTTP) override; 

	//  Specific Variables
	bool setBrightness(uint8_t bri) override  {   _brightness = bri; return true; }
	bool getBrightness(uint8_t& bri) override {   bri = _brightness; return true; }

	bool setColor(RgbColor color) override  { _color = color; return true; }
	bool getColor(RgbColor& color) override {  color = _color; return true; }

	bool setSpeed(uint8_t speed) override  {   _speed = speed; return true; }
	bool getSpeed(uint8_t& speed) override {   speed = _speed; return true; }

	bool setRotation(uint8_t rotation) override  {   _rotation = rotation; return true; }
	bool getRotation(uint8_t& rotation) override {   rotation = _rotation; return true; }

	bool setText(const char * text) override  {   free(_marqueeText); _marqueeText = strdup(text); return true; }
	bool getText(char *& text) override {   text = _marqueeText; return true; }

	bool setPalette(palette_type palette) override  {   _palette = palette; return true; }
	bool getPalette(palette_type& palette) override {   palette = _palette; return true; }

private:
	char * _marqueeText;
	uint32_t _speed;
	uint8_t _brightness;
	palette_type _palette;
	uint8_t _rotation;
	RgbColor _color;

};

class AdalightEffect : public SwitchEffect
{

public:
	AdalightEffect(EffectHandlerFunction Fn);

	//  These functions just need to add and retrieve preset values from the json.
	bool load(JsonObject& root, const char *& ID) override; 
	bool addJson(JsonObject& settings) override; 
	bool args(ESP8266WebServer & HTTP); 

	bool setSerialspeed(uint32_t speed) override {  _serialspeed = speed; Refresh(); return true; }
	bool getSerialspeed(uint32_t& speed) override {  speed = _serialspeed; return true; }

private:
	uint32_t _serialspeed;

};





