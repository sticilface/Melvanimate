#pragma once
#include <NeoPixelBus.h>

#include <functional>
#include <ArduinoJson.h>

#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "EffectHandler.h"
#include "helperfunc.h"
#include <NeoPixelAnimator.h>
#include "mybus.h"
#include "RTC_manager.h"



extern MyPixelBus * strip;
extern NeoPixelAnimator * animator;

#define PRESETS_FILE "/presets_"

//#define DebugEffectManager Serial

#if  defined(DebugEffectManager)
#define DebugEffectManagerf(...) DebugEffectManager.printf(__VA_ARGS__)
//#define DebugEffectManagerf(_1, ...) DebugEffectManager.printf_P( PSTR(_1), ##__VA_ARGS__) //  this saves around 5K RAM...

#else
#define DebugEffectManagerf(...) {}
#endif

using namespace helperfunc;
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

	bool Add(const char * name, EffectHandler* Handler, bool defaulteffect = false);

	bool Start(EffectHandler* handle);
	bool Startblank(EffectHandler* handler);
	bool Startblank(uint8_t index) { Startblank(_findhandle(index)) ; }
	inline bool Start()  { return Start(_toggleHandle); }
	inline bool Start(const char * name)  { return Start(_findhandle(name)); }
	inline bool Start(const String name)  { return Start(name.c_str()); }
	inline bool Start(uint8_t index) { return Start(_findhandle(index)); }

	void Refresh() ;
	bool Next() ;
	bool Previous();
	bool Stop() ;
	bool Pause() ;
	virtual void loop(); //  this can be overridden....  but should contain _process() function to operate...

	void setWaitFn(std::function<bool()> Fn ) { _waitFn = Fn; } //  allow effects to wait until triggering next...

	EffectHandler* Current();  // returns current.. or the next in line.

	const uint16_t total() const { return _count;}
	const char* getName(uint8_t i);

	// preset hanlding
	bool Save(uint8_t ID, const char * name, bool overwrite = false);

	//bool Load(String value);  //  loads effects using file number... 1.2 3.4 etc.... needed for presets page, or presets that change running effect
	bool Load(const char *);
	bool Load(uint8_t ID);    //  loads effect for the current running effect.. works from homepage...
	bool Load(uint8_t File, uint8_t ID); //  they all call this eventually....
	bool removePreset(uint8_t ID);

	// fetches info from SPIFFS valid presests for current effect
	void removeAllpresets();
	void addAllpresets(JsonObject & root);
	bool addCurrentPresets(JsonObject & root);

	// NEW....manage all presets...
	bool fillPresetArray();
#ifdef DebugEffectManager
	void dumpPresetArray();
#endif

	uint8_t nextFreePresetID();

protected:

	EffectHandler*  _currentHandle;
	EffectHandler*  _firstHandle;
	EffectHandler*  _lastHandle;
	EffectHandler*  _NextInLine;
	EffectHandler*  _toggleHandle;
	EffectHandler*  _defaulteffecthandle;
	EffectHandler*  _findhandle(const char * handle);
	EffectHandler*  _findhandle(uint8_t index);


	uint16_t _count;

	void _process();  //  this must be called by any derived classes in loop.
	RTC_manager _rtcman;

private:
	std::function<bool()>  _waitFn = nullptr;



	struct Presets_s {
		~Presets_s()
		{
			if (name) {
				free((void*)name);
			}
		}
		uint8_t file{0};
		uint8_t id{};
		EffectHandler * handle;
		const char * name;
		void setname(const char * namein)
		{
			name = strdup(namein);
		}
	};

	Presets_s * _presetS{nullptr};
	uint8_t _presetcountS{0};

	int _nextFreeFile();


};
