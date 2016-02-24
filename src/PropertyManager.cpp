#include "PropertyManager.h"
#include "EffectManager.h"

AbstractPropertyHandler* PropertyManager::addVar(AbstractPropertyHandler* ptr)
{
	if (ptr) {

		if (!_firsthandle) {
			_firsthandle = ptr;
			ptr->next(nullptr);
		} else {

			AbstractPropertyHandler* handle = nullptr;
			AbstractPropertyHandler* lasthandle = nullptr;
			for (handle = _firsthandle; handle; handle = handle->next()) {
				lasthandle = handle;
			}

			lasthandle->next(ptr);
		}
		return ptr;
	}

	return nullptr;
}

void PropertyManager::EndVars() 
{


	//  set the first pointer in manager to null, but keeps track of pointer to first handle..

	AbstractPropertyHandler* handle = nullptr;
	AbstractPropertyHandler* previoushandle = nullptr;

	for ( handle = _firsthandle; handle; handle = handle->next()) {

		if (previoushandle) {
			delete previoushandle;
			previoushandle = nullptr;
		}

		previoushandle = handle;
	}

	_firsthandle = nullptr;

}

bool PropertyManager::parseJsonEffect(JsonObject & root)
{

//	Serial.printf("[PropertyManager::parseJsonEffect] called\n");
	bool success = false;

	AbstractPropertyHandler* handle = nullptr;

	for (handle = _firsthandle; handle; handle = handle->next()) {
		if (handle->parseJsonProperty(root)) {
			success = true;
		}
	}
	return success;
}

bool PropertyManager::addEffectJson(JsonObject & root)
{
//	Serial.printf("[PropertyManager::addEffectJson] called\n");
	bool success = false;

	AbstractPropertyHandler* handle = nullptr;

	for (handle = _firsthandle; handle; handle = handle->next()) {
		if (handle->addJsonProperty(root)) {
			success = true;
		}
	}
	return success;
}


bool Variable<RgbColor>::addJsonProperty(JsonObject & root)
{
	JsonObject& color = root.createNestedObject(_name);
	color["R"] = _var.R;
	color["G"] = _var.G;
	color["B"] = _var.B;
	return true;
}

bool Variable<RgbColor>::parseJsonProperty(JsonObject & root)
{
	bool changed = false;
	if (root.containsKey(_name)) {


		if (root[_name].is<const char*>() ) {
//			Serial.printf("[Variable<RgbColor>::parseJsonProperty] Color converted from String\n");
			EffectManager::convertcolor(root, _name);
		}

		uint8_t R = root[_name]["R"].as<long>();
		uint8_t G = root[_name]["G"].as<long>();
		uint8_t B = root[_name]["B"].as<long>();

		if (_var.R != R) {
			_var.R = R;
			changed = true;
		}
		if (_var.G != G) {
			_var.G = G;
			changed = true;
		}
		if (_var.B != B) {
			_var.B = B;
			changed = true;
		}

//		Serial.printf("[Variable<RgbColor>::parseJsonProperty] color1 (%u,%u,%u)\n", _var.R, _var.G, _var.B);

	}

	return changed;
}

bool Variable<const char *>::addJsonProperty(JsonObject & root)
{
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
				return true;
			}
		} else {
			_var = strdup(root[_name]);
			return true;
		}
	}
	return false;
}



