#pragma once

#include <functional>
#include <NeoPixelBus.h>
#include <ArduinoJson.h>
#include "palette.h"

#define PRESETS_FILE "/Presets.json"

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

	EffectHandler* Current() const { return _currentHandle; };

	const uint16_t total() const { return _count;}
	const char* getName(uint8_t i);
	const char* getName();

	bool newSave(uint8_t ID); 
	bool newLoad(uint8_t ID); 

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


};


class EffectHandler
{

public:
	virtual bool Run() {return false; }
	virtual bool Start() {return false; }
	virtual bool Stop() {return false; }
	virtual bool Pause() {return false; }
	virtual void Refresh() {}
	virtual void SetTimeout(uint32_t) {}

	// experimental 
	virtual bool load(JsonObject& root, uint8_t nID) {};
	virtual bool save(JsonObject& root, uint8_t nID) {};


	// specific virtual functions for ALL effects... 
	// If they are not handlesd by subclass, they return false. 

	virtual bool setBrightness(uint8_t) { return false; }
	virtual bool getBrightness(uint8_t&) { return false;};

	virtual bool setColor(RgbColor ) { return false;}
	virtual bool getColor(RgbColor&) { return false;}

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
	GeneralEffect(EffectHandlerFunction Fn) : SwitchEffect(Fn) {};

	bool load(JsonObject& root, uint8_t nID) override;
	bool save(JsonObject& root, uint8_t nID) override;

	bool setBrightness(uint8_t bri) override { Serial.println("[sB]"); _brightness = bri; return true; }
	bool getBrightness(uint8_t& bri) override { Serial.println("[gB]"); bri = _brightness; return true; }

	bool setColor(RgbColor color) override  { Serial.println("[sC]"); _color = color; return true; }
	bool getColor(RgbColor& color) override { Serial.println("[gC]"); color = _color; return true; }
	
private:
	uint32_t _speed; 
	uint8_t _brightness;
	RgbColor _color; 
	
}; 


// class MarqueeEffect : public SwitchEffect
// {

// public:

// 	bool load() override {};
// 	bool save() override {};

// private:
// 	const char * MarqueeText; 
// 	uint32_t _speed; 
// 	uint32_t _timeout;
// 	uint8_t _brightness;

// }; 






