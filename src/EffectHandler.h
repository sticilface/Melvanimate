#pragma once

#include "PropertyManager.h"
#include "ArduinoJson.h"
/* ------------------------------------------------------------------------
					Effect Handler
					Dummy implementation (required)
--------------------------------------------------------------------------*/

//#define DebugEffectHandler Serial

#if defined(DebugEffectHandler)
//#define DebugEffectHandlerf(...) DebugEffectHandler.printf(__VA_ARGS__)
#define DebugEffectHandlerf(_1, ...) DebugEffectHandler.printf_P( PSTR(_1), ##__VA_ARGS__) //  this saves around 5K RAM...

#else
#define DebugEffectHandlerf(...) {}
#endif


class EffectHandler: public PropertyManager
{

public:
	virtual ~EffectHandler() {};
	virtual bool Run() {return false; }
	virtual bool Start() {return false; }
	virtual bool Stop() {return false; }
	virtual bool Pause() {return false; }
	virtual void Refresh() {}

	virtual bool jsonSize(uint8_t &obj, uint8_t &array ) { return false; }
//	virtual void SetTimeout(uint32_t) {}

	bool parseJson(JsonObject & root, bool override = false); // calls parseJsonEffect internally after calling propertymanager...
	virtual bool parseJsonEffect(JsonObject & root) { return false;} // allows JSON to be acted on within the effect

	// needs to NOT be virtual... call parsejson instead... but then call a virtual member..

	bool addJson(JsonObject& settings, bool onlychanged = false); // called first, iterates through 'installed properties' then calls addEffectJson
	virtual bool addEffectJson(JsonObject& settings) const { return false; };

	// save does NOT have to be overridden.  it calls addJson instead.
	virtual bool save(JsonArray& root, uint8_t ID, const char * name);

	uint8_t preset() const { return _preset; }
	void preset(uint8_t preset) { _preset = preset; }

//  Core Very important...
	EffectHandler* next() const { return _next; } //  ASK what is next
	void next (EffectHandler* next) { _next = next; } //  Set what is next
	EffectHandler* previous() const { return _previous; } //  ASK what is next
	void previous (EffectHandler* previous) { _previous = previous; } //  Set what is next


	void name (const char * name) { _name = name; }
	const char * name() const {return _name; };
	uint8_t index{0};

private:
	EffectHandler* _next{nullptr};
	EffectHandler* _previous{nullptr};

	const char * _name;
	uint8_t _preset{255};  // no current preset

protected:


};
