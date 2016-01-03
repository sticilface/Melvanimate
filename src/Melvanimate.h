#pragma once

#include <functional>
#include <NeoPixelBus.h>
#include "palette.h"


class EffectHandler;


class EffectManager
{

public:
	EffectManager();
	~EffectManager() {};

	bool Add(const char * name, EffectHandler* Handler);
	void SetTimeout(uint32_t time);
	void SetTimeout(const char * name, uint32_t time);

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


protected:

	EffectHandler*  _currentHandle;
	EffectHandler*  _firstHandle;
	EffectHandler*  _lastHandle;
	EffectHandler*  _NextInLine;
	uint16_t _count;
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


	//colours

	virtual void Color(RgbColor color) {} // not in use...
	virtual void Random() {}
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
								Unsed Handler
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
								Attempt at Template
--------------------------------------------------------------------------*/

// class EffectGroup {
// public:
// 	EffectGroup(uint16_t count) {
// 		_pixelset = new uint16_t(count);
// 	}
// 	~EffectGroup() {
// 		delete[] _pixelset;
// 	}


// private:
// 	uint16_t* _pixelset;
// };

/* ------------------------------------------------------------------------
								Attempt at Effect Object - crap.  issues with scope!
--------------------------------------------------------------------------*/

/* ------------------------------------------------------------------------
				My own animator class
--------------------------------------------------------------------------*/



// class NeoPixelBus;

// typedef std::function<void(float progress)> ObjUpdateCallback;

// class NeoPixelObject
// {
// public:
//     NeoPixelObject(NeoPixelBus* bus, uint16_t count);
//     ~NeoPixelObject();

//     bool IsAnimating() const
//     {
//         return _activeAnimations > 0;
//     }

//     bool IsAnimating(uint16_t n)
//     {
//         return (IsAnimating() && _animations[n].time != 0);
//     }

//     void StartAnimation(uint16_t n, uint16_t time, ObjUpdateCallback animUpdate);
//     void StopAnimation(uint16_t n);
//     void UpdateAnimations(uint32_t maxDeltaMs = 1000);

//     bool IsPaused()
//     {
//         return (!_isRunning);
//     }

//     void Pause()
//     {
//         _isRunning = false;
//     }

//     void Resume()
//     {
//         _isRunning = true;
//         _animationLastTick = millis();
//     }

//     void FadeTo(uint16_t time, RgbColor color);

// private:
//     NeoPixelBus* _bus;

//     struct AnimationContext
//     {
//         AnimationContext() :
//         	pixel(0),
//             time(0),
//             remaining(0),
//             fnUpdate(NULL)
//         {}
//         uint16_t pixel;
//         uint16_t time;
//         uint16_t remaining;

//         ObjUpdateCallback fnUpdate;
//     };

//     AnimationContext* _animations;
//     uint32_t _animationLastTick;
//     uint16_t _activeAnimations;
//     bool _isRunning;
// };







// class testclass1
// {
// public:
// 	void Show() {};
// };



// class testclass2
// {
// public:

// 	static testclass1* pABC;
// 	void set(testclass1* value ) { pABC = value; }
// 	void func() { pABC->Show(); };
// };











