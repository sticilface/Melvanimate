#pragma once

#include "PropertyManager.h"
#include "ArduinoJson.h"
/* ------------------------------------------------------------------------
					Effect Handler
					Dummy implementation (required)
--------------------------------------------------------------------------*/

#define DebugEffectHandler

#ifdef DebugEffectHandler
#define DebugEffectHandlerf(...) Serial.printf(__VA_ARGS__)
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

	bool parseJson(JsonObject & root); // calls parseJsonEffect internally after calling propertymanager...
	virtual bool parseJsonEffect(JsonObject & root) { return false;} // allows JSON to be acted on within the effect

	// needs to NOT be virtual... call parsejson instead... but then call a virtual member..

	bool addJson(JsonObject& settings); // called first, iterates through 'installed properties' then calls addEffectJson
	virtual bool addEffectJson(JsonObject& settings) const { return false; };

	// save does NOT have to be overridden.  it calls addJson instead.
	virtual bool save(JsonArray& root, uint8_t ID, const char * name);

	uint8_t preset() const { return _preset; }
	void preset(uint8_t preset) { _preset = preset; }

	void animate(bool require) { _animator = require; }
	bool animate() const  { return _animator; }

//  Core Very important...
	EffectHandler* next() const { return _next; } //  ASK what is next
	void next (EffectHandler* next) { _next = next; } //  Set what is next
	EffectHandler* previous() const { return _previous; } //  ASK what is next
	void previous (EffectHandler* previous) { _previous = previous; } //  Set what is next


	void name (const char * name) { _name = name; }
	const char * name() const {return _name; };

private:
	EffectHandler* _next{nullptr};
	EffectHandler* _previous{nullptr};

	const char * _name;
	uint8_t _preset{255};  // no current preset
	bool _animator{false}; 

protected:


};