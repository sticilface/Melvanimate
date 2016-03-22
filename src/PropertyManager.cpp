




#include "PropertyManager.h"
#include "EffectManager.h"
#include "helperfunc.h"

using namespace helperfunc; 

AbstractPropertyHandler* PropertyManager::addVar(AbstractPropertyHandler* ptr)
{
	if (ptr) {

		if (!_firsthandle) {
			PropertyManagerf("[PropertyManager::addVar] Added: %s\n", ptr->name());
			_firsthandle = ptr;
			ptr->next(nullptr);
		} else {

			AbstractPropertyHandler* handle = nullptr;
			AbstractPropertyHandler* lasthandle = nullptr;

			for (handle = _firsthandle; handle; handle = handle->next()) {
				lasthandle = handle;
			}
			PropertyManagerf("[PropertyManager::addVar] Added: %s\n", ptr->name());
			lasthandle->next(ptr);
		}
		return ptr;
	}

	return nullptr;
}

void PropertyManager::EndVars()
{
	PropertyManagerf("[EndVars] Called\n");

	AbstractPropertyHandler* handle = _firsthandle;
	AbstractPropertyHandler* previoushandle = nullptr;

#ifdef PropertyManager
	if (!handle) {
			PropertyManagerf("[EndVars] No handles Identified to delete for %s\n", static_cast<EffectHandler*>(this)->name());
	}
#endif

	while (handle) {
		previoushandle = handle;
		handle = handle->next();
		if (previoushandle) {
			PropertyManagerf("[EndVars] Deleting: %s\n", previoushandle->name());
			delete previoushandle;
		}
	}
	_firsthandle = nullptr;
}

bool PropertyManager::parseJsonEffect(JsonObject & root)
{

//	Serial.printf("[PropertyManager::parseJsonEffect] called\n");
	bool success = false;

	AbstractPropertyHandler* handle = nullptr;

	for (handle = _firsthandle; handle; handle = handle->next()) {
		handle->setChanged(false); 
		if (handle->parseJsonProperty(root)) {
			success = true;
		}
	}
	return success;
}

bool PropertyManager::addEffectJson(JsonObject & root, bool onlychanged)
{
//	Serial.printf("[PropertyManager::addEffectJson] called\n");
	PropertyManagerf("[PropertyManager::addEffectJson] called\n"); 

	bool success = false;

	AbstractPropertyHandler* handle = nullptr;

	for (handle = _firsthandle; handle; handle = handle->next()) {
		PropertyManagerf("[PropertyManager::addEffectJson] Variable : %s \n", handle->name()); 

		if (handle->addJsonProperty(root, onlychanged)) {
			success = true;
			if (onlychanged) { handle->setChanged(false); } //  once json has been added.  reset changed state... 
		} else {
			PropertyManagerf("[PropertyManager::addEffectJson] addJsonProperty returned false \n"); 
		}
	}
	return success;
}


bool Variable<RgbColor>::addJsonProperty(JsonObject & root, bool onlychanged)
{
	if (onlychanged && !_changed) { return false; }
	JsonArray& color = root.createNestedArray(_name);
	color.add(_var.R);
	color.add(_var.G);
	color.add(_var.B);
	return true;
}

bool Variable<RgbColor>::parseJsonProperty(JsonObject & root)
{
	if (root.containsKey(_name)) {

		if (root[_name].is<const char*>() ) {
//			Serial.printf("[Variable<RgbColor>::parseJsonProperty] Color converted from String\n");
			convertcolor(root, _name);
		}

		uint8_t R = root[_name][0];
		uint8_t G = root[_name][1];
		uint8_t B = root[_name][2];

		if (_var.R != R) {
			_var.R = R;
			_changed = true;
		}
		if (_var.G != G) {
			_var.G = G;
			_changed = true;
		}
		if (_var.B != B) {
			_var.B = B;
			_changed = true;
		}

//		Serial.printf("[Variable<RgbColor>::parseJsonProperty] color1 (%u,%u,%u)\n", _var.R, _var.G, _var.B);

	}

	return _changed;
}

bool Variable<const char *>::addJsonProperty(JsonObject & root, bool onlychanged)
{
	if (onlychanged && !_changed) { return false; }
	root[_name] = _var;
	return true;
}

bool Variable<const char *>::parseJsonProperty(JsonObject & root)
{
	if (root.containsKey(_name)) {
		if (_var) {
			if ( strcmp(root[_name], _var)) {
				free( (void*)_var);  //  not good... but i really want to free that const char *
				_var = strdup(root[_name]);
				_changed = true; 
				return true;
			}
		} else {
			_var = strdup(root[_name]);
			_changed = true; 
			return true;
		}
	}
	return false;
}


//template<>
// Variable<const char *>::~Variable()
// {
// 	Serial.printf("[class Variable<const char *>] Deconstructor called\n");
// 	if (_var) {
// 		free((void*)_var);
// 		Serial.printf("[class Variable<const char *>] _var freed\n");
// 	}
// }


