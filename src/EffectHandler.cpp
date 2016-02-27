#include "EffectHandler.h"


bool EffectHandler::save(JsonObject& root, const char *& ID, const char * name)
{
	Serial.printf("[save] ID = %s\n", ID);

	if (root.containsKey(ID)) {
//		Serial.printf("[save] [%s]previous setting identified\n", ID);
		root.remove(ID) ;
	}

	JsonObject& current = root.createNestedObject(ID);

	current["name"] = name;
	current["effect"] = _name;

	if (addJson(current)) {
		return true;
	} else {
		return false;
	}
};

bool EffectHandler::addJson(JsonObject& root)
{
	bool found = false;

	if (PropertyManager::addEffectJson(root)) {
		found = true;
	}

	if (addEffectJson(root)) { found = true; }
	return found;

};


bool EffectHandler::parseJson(JsonObject & root)
{
	bool found = false;

	if (PropertyManager::parseJsonEffect(root)) {
		found = true;
	}
	if (parseJsonEffect(root)) { found = true; }

	if (found) {
		Refresh(); 
		_preset = 255;
	}
	return found;
}