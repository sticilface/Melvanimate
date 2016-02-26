#pragma once

#include "PropertyManager.h"
#include "ArduinoJson.h"
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

private:
	EffectHandler* _next = nullptr;
	const char * _name;
	uint8_t _preset = 255;  // no current preset

protected:


};