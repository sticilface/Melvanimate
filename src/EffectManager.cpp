
//  need to add wait for init and end animations
//

#include "Arduino.h"
#include "EffectManager.h"
#include "FS.h"

const char * PRESETS_FILE = "/presets.txt";

/*---------------------------------------------

				Effect Manager

---------------------------------------------*/

EffectManager::EffectManager() : _count(0), _firstHandle(nullptr), _currentHandle(nullptr), _lastHandle(nullptr), _toggleHandle(nullptr),
	_NextInLine(nullptr)
{};

bool EffectManager::Add(const char * name, EffectHandler* handle)
{
	_count++;

	if (!_lastHandle) {
		_firstHandle = handle; //  set the first handle
		_firstHandle->name(name); // set its name in the handler... abstract it out so user doesn't have to
		_lastHandle = handle;  // set this so we know first slot gone.

	} else {
		_lastHandle->next(handle); // give the last handler address of next
		_lastHandle = handle;  // move on..
		_lastHandle->name(name);  // give it name...
	}
	Serial.printf("ADDED EFFECT %u: %s\n", _count, _lastHandle->name());
}

EffectHandler* EffectManager::Start()
{
	if (_toggleHandle) {
		return Start(_toggleHandle->name());
	}
	return nullptr;
}

bool EffectManager::SetToggle(const char * name)
{
	EffectHandler* handler = _findhandle(name);

	if (handler) {
		_toggleHandle = handler;
		//getPresets(_toggleHandle, _numberofpresets, _presets);
		return true;
	}
	return false;
}




EffectHandler* EffectManager::_findhandle(const char * handle)
{
	EffectHandler* handler;
	bool found = false;
	for (handler = _firstHandle; handler; handler = handler->next()) {
		if ( strcmp( handler->name(), handle) == 0) {
			found = true;
			break;
		}
	}

	if (found) {
		return handler;
	} else {
		return nullptr;
	}
}

EffectHandler* EffectManager::Start(const char * name)
{
	//  end current effect...
	//  need to store address of next... and handle changeover in the loop...
	// this function should signal to current to END & store

	// actually.. maybe use return values to signal... with timeouts to prevent getting stuck..
	// manager sends... stop()....  until that returns true.. it can't --- NOPE not going to work... Start and stop should only get called once...
	if (_currentHandle) { _currentHandle->Stop(); }

	EffectHandler* handler = _findhandle(name);


	if (handler) {
		_NextInLine = handler;
		getPresets(_NextInLine, _numberofpresets, _presets, _preset_names);

		Serial.printf("[Start] %u presets found for %s\n", _numberofpresets, _NextInLine->name());

		return _NextInLine;
	} else {
		return nullptr;
	}

};

bool EffectManager::Next()
{
	_currentHandle = _currentHandle->next();
};


bool EffectManager::Stop()
{
	if (_currentHandle) { _currentHandle->Stop(); }
};

bool EffectManager::Pause()
{
	if (_currentHandle) { _currentHandle->Pause(); }
};

void EffectManager::Refresh()
{
	if (_currentHandle) { _currentHandle->Refresh(); }

}

void EffectManager::Loop()
{

	bool waiting = false;

	// if (_currentHandle)  {
	// 	waiting = _currentHandle->wait();
	// }

	if (_waitFn)  {
		waiting = _waitFn();
	}


	//  This flips over to next effect asyncstyle....
	if (!waiting && _NextInLine) {
		Serial.println("[Loop] Next effect STARTED");
		_currentHandle = _NextInLine;
		_NextInLine = nullptr;
		_currentHandle->Start();
		return;
	}

	if (!waiting && _currentHandle) { _currentHandle->Run(); }
};

void EffectManager::SetTimeout(uint32_t time)
{
	if (_currentHandle) { _currentHandle->SetTimeout(time); }
}

void EffectManager::SetTimeout(const char * name, uint32_t time)
{

	EffectHandler* handler;
	for (handler = _firstHandle; handler; handler = handler->next()) {
		if ( strcmp( handler->name(), name) == 0) {
			break;
		}
	}

	if (handler) { handler->SetTimeout(time); }

}



const char * EffectManager::getName(uint8_t i)
{
	if (i > _count) { return ""; }

	EffectHandler* handler;
	uint8_t count = 0;
	for (handler = _firstHandle; handler; handler = handler->next()) {
		if ( i == count ) { break; }
		count++;
	}
	return handler->name();
}



bool EffectManager::_parsespiffs(char *& data,  DynamicJsonBuffer & jsonBuffer, JsonObject *& root, const char * file_name)
{
	uint32_t starttime = millis();

	File f = SPIFFS.open(file_name, "r");
	bool success = false;

	if (!f) {
		Serial.println("[_parsespiffs] No File Found");
	}

	if (f && f.size()) {

		//Serial.println("[_parsespiffs] pre-malloc");

		data = new char[f.size()];
		// prevent nullptr exception if can't allocate
		if (data) {
			Serial.printf("[_parsespiffs] buffer size %u\n", f.size());

			//  This method give a massive improvement in file reading speed for SPIFFS files..

			int bytesleft = f.size();
			int position = 0;
			while ((f.available() > -1) && (bytesleft > 0)) {

				// get available data size
				int sizeAvailable = f.available();
				if (sizeAvailable) {
					int readBytes = sizeAvailable;

					// read only the asked bytes
					if (readBytes > bytesleft) {
						readBytes = bytesleft ;
					}

					// get new position in buffer
					char * buf = &data[position];
					// read data
					int bytesread = f.readBytes(buf, readBytes);
					bytesleft -= bytesread;
					position += bytesread;

				}
				// time for network streams
				delay(0);
			}


			root = &jsonBuffer.parseObject(data);

			if (root->success()) {
				success = true;
			}
		} else {
			Serial.println("[_parsespiffs] malloc failed");
		}
	}

	f.close();

	Serial.printf("[_parsespiffs] time: %u\n", millis() - starttime);
	if (success) {
		return true;
	} else {
		return false;
	}
}

bool EffectManager::removePreset(uint8_t ID)
{

	DynamicJsonBuffer jsonBuffer;
	const char * cID = jsonBuffer.strdup(String(ID).c_str());
	JsonObject * root = nullptr;
	char * data = nullptr;
	bool success = false;

	if (_parsespiffs(data, jsonBuffer, root, PRESETS_FILE )) {

		if (root) {

			if (root->containsKey(cID)) {
				root->remove(cID) ;
				File f = SPIFFS.open(PRESETS_FILE, "w");

				if (f) {

					root->prettyPrintTo(f);
					f.close();
					success = true;
					Serial.printf("[removePreset] [%s] Setting Removed\n", cID);
				} else {
					Serial.println("[removePreset] FILE OPEN error:  NOT saved");
				}

			}

		}

	}

	if (data) {
		delete[] data;
	}

	if (success) {
		return true;
	} else {
		return false;
	}



}

bool EffectManager::getPresets(EffectHandler * handle, uint8_t& numberofpresets, uint8_t *& presets, char **& preset_names)
{
	char * data = nullptr;

	// delete any existing preset information.

	for (uint8_t i = 0; i < numberofpresets; i++) {
		char * p = preset_names[i];
		if (p) { free(p); }
	}

	if (presets) {
		delete[] presets;
		presets = nullptr;
	}

	if (preset_names) {
		delete[] preset_names;
		preset_names = nullptr;
	}

	numberofpresets = 0;

	if (handle) {
		DynamicJsonBuffer jsonBuffer;
		JsonObject * root = nullptr;
		uint8_t count = 0;

		if (_parsespiffs(data, jsonBuffer, root, PRESETS_FILE )) {

			delay(0);

			if (root) { // avoid nullptr errors...

				for (JsonObject::iterator it = root->begin(); it != root->end(); ++it) {
					// get id of preset
					const char * key = it->key;
					// extract the json object for each effect
					JsonObject& current = it->value.asObject();

					// compare to the name of current effect
					//	Serial.printf("[getPresets] Identified presets for %s (%s)\n", handle->name(), key);
					if (current.containsKey("effect")) {
						if ( strcmp( current["effect"], handle->name() ) == 0) {
							// if matched then this preset is a valid effect for the current one.
							// Serial.printf("[getPresets] found preset for %s (%s)\n", handle->name(), key);
							count++;
						}
					}
				}

				// once number of presets identified, create array and fill it..
				numberofpresets = count;

				if (numberofpresets) {

					presets = new uint8_t[numberofpresets];
					preset_names = new char*[numberofpresets];

					count = 0; // reset the counter...

					if (presets && preset_names) {

						memset(presets, 0, numberofpresets);
						memset(preset_names, 0, numberofpresets);

						for (JsonObject::iterator it = root->begin(); it != root->end(); ++it) {
							// get id of preset
							const char * key = it->key;
							// extract the json object for each effect
							JsonObject& current = it->value;
							// compare to the name of current effect

							if (current.containsKey("effect") && handle->name()) {
								if ( strcmp(current["effect"], handle->name()) == 0) {
									// if matched then this preset is a valid effect for the current one.
									presets[count] = String(key).toInt();

									if (current.containsKey("name")) {
										preset_names[count] = strdup(current["name"]);
									} else {
										preset_names[count] = nullptr;
									}


									count++;
								}
							}

						}
					}

				}
			}

		}

	}

	if (data) { delete[] data; }

	if (numberofpresets) {
		return true;
	} else {
		return false;
	}




}

uint8_t EffectManager::nextFree(JsonObject & root)
{

	for (JsonObject::iterator it = root.begin(); it != root.end(); ++it) {
		uint8_t ID = String(it->key).toInt();
		Serial.printf("[EffectManager::nextFree] %u,", ID);
	}

	Serial.println();

}

bool EffectManager::newSave(uint8_t ID, const char * name, bool overwrite)
{
	bool success = false;
	char * data = nullptr;

	Serial.println("[newSave] Called");

	if (_currentHandle) {

		DynamicJsonBuffer jsonBuffer;
		const char * cID = jsonBuffer.strdup(String(ID).c_str());
		JsonObject * root = nullptr;

		if (_parsespiffs(data, jsonBuffer, root, PRESETS_FILE )) {
			Serial.println("[newSave] Existing Settings Loaded");
		} else {
			// if no file exists, or parse fails create new json object...
			root = &jsonBuffer.createObject();
		}

		//  temp working

		uint8_t xID = nextFree(*root);
		const char * xcID = jsonBuffer.strdup(String(xID).c_str());
		Serial.printf("[newSave] Next Free %s\n", xcID);

		//

		if (root && _currentHandle->save(*root, cID, name)) {

			Serial.println("[newSave] New Settings Added");
			File f = SPIFFS.open(PRESETS_FILE, "w");

			if (f) {

				root->prettyPrintTo(f);
//				root->prettyPrintTo(Serial);

				Serial.printf("[newSave] DONE Heap %u\n", ESP.getFreeHeap() );
				f.close();
				success = true;
			} else {
				Serial.println("FILE OPEN error:  NOT saved");
			}

		}

	}

	if (data) {
		delete[] data;
	}

	if (success) {
		return true;
	} else {
		return false;
	}

}


bool EffectManager::newLoad(uint8_t ID)
{
	bool success = false;
	if (_currentHandle) {
		DynamicJsonBuffer jsonBuffer;
		const char * cID = jsonBuffer.strdup(String(ID).c_str());
		JsonObject * root = nullptr;
		char * data = nullptr;
		bool success = false;

		if (_parsespiffs(data, jsonBuffer, root, PRESETS_FILE )) {

			//  Is current effect same effect as it wanted.
			JsonObject& preset = (*root)[cID];
			EffectHandler* effectp = nullptr;


			if (root->containsKey(cID)) { // only process if the json contains a key with the right name

				if (_currentHandle && preset.containsKey("effect")) {
					if (strcmp(_currentHandle->name(), preset["effect"]) != 0) {
						effectp = Start(preset["effect"].asString());
						Serial.printf("[EffectManager::newLoad]  Effect changed to %s\n", effectp->name());
					} else {
						effectp = _currentHandle;
					}
				}

				// now can load the effect using json

				if (effectp->parseJson(preset)) {
						Serial.printf("[EffectManager::newLoad] Preset %s loaded\n", cID);
						effectp->_preset = ID;
						success = true; 
				}

			}
		}

		if (data) { delete[] data; }
	}
	return success;
}


bool EffectHandler::save(JsonObject& root, const char *& ID, const char * name)
{
	Serial.printf("[save] ID = %s\n", ID);

	if (root.containsKey(ID)) {
		Serial.printf("[save] [%s]previous setting identified\n", ID);
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


bool EffectHandler::installProperty(PropertyHandler* ptr)
{
	if (ptr) {

		if (!_propertyPtr) {
			_propertyPtr = ptr;
			ptr->next(nullptr);
			Serial.printf("[EffectHandler::installProperty] installed property 1: %s\n", ptr->name());
		} else {

			PropertyHandler* handler = nullptr;
			PropertyHandler* lasthandler = nullptr;
			uint8_t count = 1;
			for (handler = _propertyPtr; handler; handler = handler->next()) {
				lasthandler = handler;
				count++;
			}

			lasthandler->next(ptr);

			Serial.printf("[EffectHandler::installProperty] %s property %u: %s\n", name(), count, ptr->name());

		}




	}
}

bool EffectHandler::addJson(JsonObject& settings)
{
	bool found = false;
	PropertyHandler* handler = nullptr;
	for (handler = getPropertyPtr(); handler; handler = handler->next()) {
		if (handler->addJsonProperty(settings)) {
			Serial.printf("[EffectHandler::addJson] Settings added for %s\n", handler->name());
			found = true;
		}

	}

	if (addEffectJson(settings)) { found = true; }
	return found;

};


bool EffectHandler::parseJson(JsonObject & root)
{
	bool found = false;
	PropertyHandler* handler = nullptr;
	for (handler = getPropertyPtr(); handler; handler = handler->next()) {
		if (handler->parseJsonProperty(root)) {
			Serial.printf("[EffectHandler::parseJson] Settings parsed for %s\n", handler->name());
			found = true;
		}

	}

	if (parseJsonEffect(root)) { found = true; }
	if (found) 
	{
		_preset = 255; 
	}
	return found;
}



/*---------------------------------------------

				Generic Class

---------------------------------------------*/



// Moved objec stuff to newer class..



// bool EffectObject::StartBasicAnimations(uint16_t time) {

// 	Serial.println("Start Andimations");

// 	if (!_bus || !_animator) return false;

// 	for (uint16_t i = 0; i < _total; i++) {

// 		Details_s * current = &_details[i];

// 		current->original = _bus->GetPixelColor(current->pixel);

// 		Serial.printf(" I=> %u->%u original(%u,%u,%u) target(%u,%u,%u)\n", i, current->pixel,
// 		              current->original.R, current->original.G, current->original.B,
// 		              current->target.R, current->target.G, current->target.B );

// 		AnimUpdateCallback  animUpdate = [this, i, &current](float progress)
// 		{
// 			RgbColor updatedColor = RgbColor::LinearBlend(current->original, current->target, progress );
// 			_bus->SetPixelColor(current->pixel, updatedColor);
// 		};

// 		_animator->StartAnimation(current->pixel, time, animUpdate);
// 	}
// 	return true;
// }



/* ------------------------------------------------------------------------
				My own animator class
--------------------------------------------------------------------------*/


// NeoPixelObject::NeoPixelObject(NeoPixelBus* bus, uint16_t count) :
//     _bus(bus),
//     _animationLastTick(0),
//     _activeAnimations(0),
//     _isRunning(true)
// {
//     _animations = new AnimationContext[count];
// }

// NeoPixelObject::~NeoPixelObject()
// {
//     _bus = NULL;
//     if (_animations)
//     {
//         delete[] _animations;
//         _animations = NULL;
//     }
// }

// void NeoPixelObject::StartAnimation(uint16_t n, uint16_t time, ObjUpdateCallback animUpdate)
// {
//     if (n >= _bus->PixelCount())
//     {
//         return;
//     }

//     if (_activeAnimations == 0)
//     {
//         _animationLastTick = millis();
//     }

//     StopAnimation(n);

//     if (time == 0)
//     {
//         time = 1;
//     }

//     _animations[n].time = time;
//     _animations[n].remaining = time;
//     _animations[n].fnUpdate = animUpdate;

//     _activeAnimations++;
// }

// void NeoPixelObject::StopAnimation(uint16_t n)
// {
//     if (IsAnimating(n))
//     {
//         _activeAnimations--;
//         _animations[n].time = 0;
//         _animations[n].remaining = 0;
//         _animations[n].fnUpdate = NULL;
//     }
// }

// void NeoPixelObject::FadeTo(uint16_t time, RgbColor color)
// {
//     for (uint16_t n = 0; n < _bus->PixelCount(); n++)
//     {
//         RgbColor original = _bus->GetPixelColor(n);
//         ObjUpdateCallback animUpdate = [=](float progress)
//         {
//             RgbColor updatedColor = RgbColor::LinearBlend(original, color, progress);
//             _bus->SetPixelColor(n, updatedColor);
//         };
//         StartAnimation(n, time, animUpdate);
//     }
// }

// void NeoPixelObject::UpdateAnimations(uint32_t maxDeltaMs)
// {
//     if (_isRunning)
//     {
//         uint32_t currentTick = millis();
//         uint32_t delta = currentTick - _animationLastTick;

//         if (delta > maxDeltaMs)
//         {
//             delta = maxDeltaMs;
//         }

//         if (delta > 0)
//         {
//             uint16_t countAnimations = _activeAnimations;

//             AnimationContext* pAnim;

//             for (uint16_t iAnim = 0; iAnim < _bus->PixelCount() && countAnimations > 0; iAnim++)
//             {
//                 pAnim = &_animations[iAnim];

//                 if (pAnim->remaining > delta)
//                 {
//                     pAnim->remaining -= delta;

//                     float progress = (float)(pAnim->time - pAnim->remaining) / (float)pAnim->time;

//                     pAnim->fnUpdate(progress);
//                     countAnimations--;
//                 }
//                 else if (pAnim->remaining > 0)
//                 {
//                     pAnim->fnUpdate(1.0f);
//                     StopAnimation(iAnim);
//                     countAnimations--;
//                 }
//             }

//             _animationLastTick = currentTick;
//         }
//     }
// }


/*

		General Effect... TEST for SAVING presets...

*/



// GeneralEffect::GeneralEffect() {


// }




// bool GeneralEffect::load(JsonObject & root, const char *& ID)
// {
// 	if (!root.containsKey(ID)) {
// 		return false;
// 	}

// 	JsonObject& current = root[ID];
// 	const char * effect = current["effect"];

// 	if (effect) {
// 		if ( strcmp( effect , name() ) != 0) { return false; }
// 	}

// 	_brightness = current["brightness"];
// 	JsonObject& jscolor1 = current["color1"];

// 	_color.R = jscolor1["R"];
// 	_color.G = jscolor1["G"];
// 	_color.B = jscolor1["B"];

// 	Serial.printf("RGB (%u,%u,%u)\n", _color.R, _color.G, _color.B);

// 	_preset = String(ID).toInt();
// 	Serial.printf("[GeneralEffect::load] _preset = %u\n", _preset);
// 	//current["name"] = "TO BE IMPLEMENTED";
// 	return true;

// }



// bool SimpleEffect::addEffectJson(JsonObject & settings)
// {

// 	// settings["effect"] = name();
// 	// settings["brightness"] = _brightness ;
// 	// JsonObject& jscolor1 = settings.createNestedObject("color1");
// 	// jscolor1["R"] = _color.R;
// 	// jscolor1["G"] = _color.G;
// 	// jscolor1["B"] = _color.B;

// 	// //include return true to override the default no handler...
// 	// return true;
// }


// bool GeneralEffect::args(ESP8266WebServer& HTTP)
// {

// 	bool found = false;

// 	// need to add color... but change the JS to send normal POST not JSON...

// 	if (HTTP.hasArg("brightness")) {
// 		setBrightness(HTTP.arg("brightness").toInt());
// 		found = true;
// 	}

// 	return found;

// }

bool EffectManager::convertcolor(JsonObject & root, const char * node)
{
	if (root.containsKey(node)) {

		String colorstring = root[node];

		Serial.printf("[EffectManager::convertcolor] bcolorstring = %s\n", colorstring.c_str());
		JsonObject& colorroot = root.createNestedObject(node);

		colorroot["R"] = colorstring.substring(0, colorstring.indexOf(",")).toInt();
		colorstring = colorstring.substring( colorstring.indexOf(",") + 1, colorstring.length());
		colorroot["G"] = colorstring.substring(0, colorstring.indexOf(",")).toInt();
		colorroot["B"] = colorstring.substring( colorstring.indexOf(",") + 1, colorstring.length()).toInt();

		Serial.println("[EffectManager::convertcolor] root" );
		root.prettyPrintTo(Serial);
		Serial.println();
		return true;

	}
	return false;
}

// bool SimpleEffect::parseJsonEffect(JsonObject & root)
// {
// 	// bool found = false;

// 	// if (root.containsKey("color1")) {
// 	// 	if (EffectManager::convertcolor(root, "color1")) {
// 	// 		RgbColor color;
// 	// 		color.R = root["color1"]["R"].as<long>();
// 	// 		color.G = root["color1"]["G"].as<long>();
// 	// 		color.B = root["color1"]["B"].as<long>();
// 	// 		Serial.printf("[GeneralEffect::args] color1 (%u,%u,%u)\n", color.R, color.G, color.B);
// 	// 		setColor(color);
// 	// 		found = true;

// 	// 	}
// 	// }

// 	// if (root.containsKey("brightness")) {
// 	// 	setBrightness( root["brightness"] );
// 	// 	found = true;
// 	// }

// 	// if (found) { _preset = 255; }
// 	// return found;
// }

/*


	Marquee effect
	const char * _marqueeText;
	uint32_t _speed;
	uint8_t _brightness;
	Palette* _palette;
	uint8_t _rotation;
	RgbColor _color;
*/




/*

bool MarqueeEffect::load(JsonObject& root, const char *& ID)
{
	if (!root.containsKey(ID)) {
		return false;
	}

	JsonObject& current = root[ID];


	const char * effect = current["effect"];

	if (effect) {
		if ( strcmp( effect , EffectHandler::name() ) != 0) { return false; }
	}

	//  load palette settings...
	// if (current.containsKey("Palette")) {
	// 	Serial.println("[MarqueeEffect::load] Palette key foung in settings");
	// 	// Parse the whole object to palette to retrieve settings
	// 	if (_palette.parseJson(current)) {
	// 		Serial.println("[MarqueeEffect::load] Palette Settings Applied!");
	// 	}

	// }


// 	_brightness = current["brightness"];
// 	_speed = current["speed"];

// //	const char * palette_temp = current["palette"];

// 	JsonObject& jscolor1 = current["color1"];
// 	_color.R = jscolor1["R"];
// 	_color.G = jscolor1["G"];
// 	_color.B = jscolor1["B"];

	if (current.containsKey("rotation")) {
		_rotation = current["rotation"];
	}


	if (current.containsKey("marqueetext")) {

		if (_marqueeText) {
			free (_marqueeText);
			_marqueeText = nullptr;
		}


		const char * temp_text = current["marqueetext"];
		_marqueeText = strdup(temp_text);
	}

	_preset = String(ID).toInt();
	return true;

}

bool MarqueeEffect::addEffectJson(JsonObject& settings)
{
	settings["effect"] = EffectHandler::name();
	// settings["brightness"] = _brightness ;
	// settings["speed"] = _speed;
	//settings["palette"] = String(Palette::enumToString(_palette));

	// JsonObject& jscolor1 = settings.createNestedObject("color1");
	// jscolor1["R"] = _color.R;
	// jscolor1["G"] = _color.G;
	// jscolor1["B"] = _color.B;

	settings["rotation"] = _rotation;
	settings["marqueetext"] = _marqueeText;

	//include return true to override the default no handler...
	return true;
} ;

bool MarqueeEffect::parseJsonEffect(JsonObject& root)
{
	bool found = false;

	// need to add color... but change the JS to send normal POST not JSON...

	// if (root.containsKey("color")) {
	// 	JsonObject& color = root["color"];
	// 	RgbColor input;
	// 	input.R = color["R"];
	// 	input.G = color["G"];
	// 	input.B = color["B"];
	// 	setColor(input);
	// 	found = true;

	// }

	// if (root.containsKey("brightness")) {
	// 	setBrightness(root["brightness"]);
	// 	found = true;
	// }

	// if (root.containsKey("speed")) {
	// 	setSpeed(root["speed"]);
	// 	found = true;
	// }

	if (root.containsKey("rotation")) {
		uint8_t rotation = root["rotation"];
		if (rotation > 3) { rotation = 0; }
		setRotation( rotation );
		Refresh();
		found = true;
	}

	if (root.containsKey("marqueetext")) {
		setText(root["marqueetext"]) ;
		Refresh();
		found = true;
	}

	// if (root.containsKey("Palette")) {
	// 	Serial.println("[MarqueeEffect::args] palette parsed to Effect function");
	// 	if (_palette.parseJson(root)) {
	// 		Refresh();
	// 		found = true;
	// 	}
	// }



	if (found) { _preset = 255; }

	return found;
}

*/


/*

	Adalight class


*/

/*
AdalightEffect::AdalightEffect(EffectHandlerFunction Fn) : SwitchEffect(Fn), _serialspeed(115200) {};

//  These functions just need to add and retrieve preset values from the json.
bool AdalightEffect::load(JsonObject& root, const char *& ID)
{
	if (!root.containsKey(ID)) {
		return false;
	}

	JsonObject& current = root[ID];
	const char * effect = current["effect"];

	if (effect) {
		if ( strcmp( effect , name() ) != 0) { return false; }
	}

	_serialspeed = current["serialspeed"];
	_preset = String(ID).toInt();

	return true;
}

bool AdalightEffect::addEffectJson(JsonObject& settings)
{
	settings["effect"] = name();
	settings["serialspeed"] = _serialspeed;
	return true;
}

bool AdalightEffect::parseJsonEffect(JsonObject& root)
{
	bool found = false;

	if (root.containsKey("serialspeed")) {
		setSerialspeed(root["serialspeed"]);
		found = true;
	}

	if (found) { _preset = 255; }
	return found;
}

*/

/*

Dummy Class


*/


/*
DummyEffect::DummyEffect(EffectHandlerFunction Fn) : SwitchEffect(Fn) {};


//  These functions just need to add and retrieve preset values from the json.
bool DummyEffect::load(JsonObject& root, const char *& ID)
{
	if (!root.containsKey(ID)) {
		return false;
	}

	JsonObject& current = root[ID];
	const char * effect = current["effect"];

	if (effect) {
		if ( strcmp( effect , name() ) != 0) { return false; }
	}

	Serial.println("[DummyEffect::load] JSON Loaded");
	current.prettyPrintTo(Serial);
	Serial.println();

	//  load palette settings...
	if (current.containsKey("Palette")) {
		Serial.println("[DummyEffect::load] Palette key foung in settings");
		// Parse the whole object to palette to retrieve settings
		if (_palette.parseJson(current)) {
			Serial.println("[DummyEffect::load] Palette Settings Applied!");
		}

	}

//	_brightness = current["brightness"];
//	_speed = current["speed"];

//	const char * palette_temp = current["palette"];

	JsonObject& jscolor1 = current["color1"];
//	_color.R = jscolor1["R"];
//	_color.G = jscolor1["G"];
//	_color.B = jscolor1["B"];

	if (current.containsKey("rotation")) {
//		_rotation = current["rotation"];
	}

	if (current.containsKey("marqueetext")) {
//	if (_marqueeText) {
//		free (_marqueeText);
//		_marqueeText = nullptr;
//	}



//		const char * temp_text = current["marqueetext"];
//		_marqueeText = strdup(temp_text);
	}

//	_preset = String(ID).toInt();
	return true;
}

bool DummyEffect::addEffectJson(JsonObject& settings)
{
	settings["effect"] = name();

	if (_palette.addJson(settings)) {
		Serial.println("[DummyEffect::addJson] Palette Settings Added");
	}

//	settings["brightness"] = _brightness ;
//	settings["speed"] = _speed;
	//settings["palette"] = String(Palette::enumToString(_palette));

//	JsonObject& jscolor1 = settings.createNestedObject("color1");
//	jscolor1["R"] = _color.R;
//	jscolor1["G"] = _color.G;
//	jscolor1["B"] = _color.B;

//	settings["rotation"] = _rotation;
//	settings["marqueetext"] = _marqueeText;

	//include return true to override the default no handler...
	//return true;

	Serial.println("[DummyEffect::addJson] JSON Added");
	settings.prettyPrintTo(Serial);
	Serial.println();

}

bool DummyEffect::parseJsonEffect(JsonObject& root)
{
	bool found = false;

	Serial.println("[DummyEffect::args] JSON parsed:");
	root.prettyPrintTo(Serial);
	Serial.println();

	if (root.containsKey("color1")) {
		if (EffectManager::convertcolor(root, "color1")) {
			RgbColor color;
			color.R = root["color1"]["R"].as<long>();
			color.G = root["color1"]["G"].as<long>();
			color.B = root["color1"]["B"].as<long>();
			Serial.printf("[DummyEffect::args] color1 (%u,%u,%u)\n", color.R, color.G, color.B);
			found = true;

		}
	}

	if (root.containsKey("color2")) {
		if (EffectManager::convertcolor(root, "color2")) {
			RgbColor color;
			color.R = root["color2"]["R"].as<long>();
			color.G = root["color2"]["G"].as<long>();
			color.B = root["color2"]["B"].as<long>();
			Serial.printf("[DummyEffect::args] color2 (%u,%u,%u)\n", color.R, color.G, color.B);
			found = true;

		}
	}

	if (root.containsKey("brightness")) {
//		setBrightness(root["brightness"]);
		found = true;
	}

	if (root.containsKey("speed")) {
//		setSpeed(root["speed"]);
		found = true;
	}

	if (root.containsKey("rotation")) {
		// uint8_t rotation = root["rotation"];
		// if (rotation > 3) { rotation = 0; }
		// setRotation( rotation );
		Refresh();
		found = true;
	}

	if (root.containsKey("marqueetext")) {
//		setText(root["marqueetext"]) ;
		Refresh();
		found = true;
	}

// dummyFn
	if (root.containsKey("Palette")) {
		Serial.println("[MarqueeEffect::args] palette parsed to Effect function");
		if (_palette.parseJson(root)) {
			Refresh();
			found = true;
		}
	}


	if (found) { _preset = 255; }

	return found;
}


*/
















