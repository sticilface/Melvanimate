#include "EffectHandler.h"


bool EffectHandler::save(JsonArray& array, uint8_t ID, const char * name)
{
	//Serial.printf("[EffectHandler::save] Effect = %s, ID = %s\n", _name, ID);

	int index = 0;

	for (JsonArray::iterator it = array.begin(); it != array.end(); ++it) {
		// *it contains the JsonVariant which can be casted as usuals
		//const char* value = *it;

		JsonObject& preset = *it;

		if (preset.containsKey("ID")) {

			if ( preset["ID"] == ID) {
				array.removeAt(index) ;
				DebugEffectHandlerf("[EffectHandler::save] preset %u removed\n", ID);
				break; 
			}

		}

		index++;
		// this also works:
		//value = it->as<const char*>();

	}

	JsonObject& current = array.createNestedObject();

	current["ID"] = ID;
	current["name"] = name;
	current["effect"] = _name;

	if (addJson(current)) {
		return true;
	} else {
		return false;
	}


};

bool EffectHandler::addJson(JsonObject & root, bool onlychanged)
{
	DebugEffectHandlerf("[EffectHandler::addJson] called\n"); 
	bool found = false;

	if (PropertyManager::addEffectJson(root, onlychanged)) {
		found = true;
	}
	// not sure i need this any more... 
	if (addEffectJson(root)) { found = true; }

	return found;

};


bool EffectHandler::parseJson(JsonObject & root, bool override)
{
	bool found = override;

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