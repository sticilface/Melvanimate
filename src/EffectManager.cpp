
//  need to add wait for init and end animations
//

#include "Arduino.h"
#include "EffectManager.h"
#include "FS.h"

//#define jsonprettyprint

const char * PRESETS_FILE = "/presets.txt";

/*---------------------------------------------

				Effect Manager

---------------------------------------------*/

EffectManager::EffectManager() : _count(0), _firstHandle(nullptr), _currentHandle(nullptr), _lastHandle(nullptr), _toggleHandle(nullptr),
	_NextInLine(nullptr), _defaulteffecthandle(nullptr)
{};

bool EffectManager::Add(const char * name, EffectHandler* handle, bool defaulteffect)
{
	_count++;

	if (defaulteffect) {
		_defaulteffecthandle = handle; 
	}

	if (!_lastHandle) {
		_firstHandle = handle; //  set the first handle
		_firstHandle->name(name); // set its name in the handler... abstract it out so user doesn't have to
		_lastHandle = handle;  // set this so we know first slot gone.

	} else {
		_lastHandle->next(handle); // give the last handler address of next
		_lastHandle = handle;  // move on..
		_lastHandle->name(name);  // give it name...
	}
//	Serial.printf("ADDED EFFECT %u: %s\n", _count, _lastHandle->name());
}

EffectHandler* EffectManager::Start()
{
	if (_toggleHandle) {
		return Start(_toggleHandle->name());
	}
	return nullptr;
}

// bool EffectManager::SetToggle(const char * name)
// {
// 	EffectHandler* handler = _findhandle(name);

// 	if (handler) {
// 		_toggleHandle = handler;
// 		return true;
// 	}
// 	return false;
// }




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

	Stop();

	EffectHandler* handler = _findhandle(name);

	if (handler) {
		_NextInLine = handler;
		_NextInLine->InitVars();

		if (_NextInLine->preset() != 255) {
			Load(_NextInLine->preset());
		}
		getPresets(_NextInLine, _numberofpresets, _presets, _preset_names);

		//  This sets the toggle... as long as it is not the default handle... ie... Off.... 
		if (_defaulteffecthandle) {
			if (handler != _defaulteffecthandle) {
				_toggleHandle = handler; 
			}
		}

		return _NextInLine;
	} else {

		if (_defaulteffecthandle)
		{
			return Start(_defaulteffecthandle->name()); 
		}

		return nullptr;
	}

};

// not sure about this implementation....
bool EffectManager::Next()
{
	_currentHandle = _currentHandle->next();
};


bool EffectManager::Stop()
{
	if (_currentHandle) {
		_currentHandle->EndVars();
		_currentHandle->Stop();
	}
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
//		Serial.println("[Loop] Next effect STARTED");
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




//  could try and package this up... maybe using a struct... ... maybe...
bool EffectManager::parsespiffs(char *& data,  DynamicJsonBuffer & jsonBuffer, JsonObject *& root, const char * file_name)
{
	uint32_t starttime = millis();

	File f = SPIFFS.open(file_name, "r");
	bool success = false;

//	if (!f) {
//		Serial.println("[parsespiffs] No File Found");
//	}

	if (f && f.size()) {

		//Serial.println("[parsespiffs] pre-malloc");

		data = new char[f.size()];
		// prevent nullptr exception if can't allocate
		if (data) {
			Serial.printf("[parsespiffs] Buffer size %u\n", f.size());

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
//			Serial.println("[parsespiffs] malloc failed");
		}
	}

	f.close();

//	Serial.printf("[parsespiffs] time: %u\n", millis() - starttime);
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

	if (parsespiffs(data, jsonBuffer, root, PRESETS_FILE )) {

		if (root) {

			if (root->containsKey(cID)) {
				root->remove(cID) ;
				File f = SPIFFS.open(PRESETS_FILE, "w");

				if (f) {

					root->prettyPrintTo(f);
					f.close();
					success = true;
//					Serial.printf("[removePreset] [%s] Setting Removed\n", cID);
				} else {
//					Serial.println("[removePreset] FILE OPEN error:  NOT saved");
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

		if (parsespiffs(data, jsonBuffer, root, PRESETS_FILE )) {

			delay(0);

			if (root) { // avoid nullptr ...

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

void EffectManager::addAllpresets(DynamicJsonBuffer& jsonBuffer, JsonObject & root)
{
	JsonObject & presets = root.createNestedObject("Presets");

	{

		DynamicJsonBuffer jsonBuffer2;
		JsonObject * root2 = nullptr;
		char * data = nullptr;


		if (parsespiffs(data, jsonBuffer2, root2, PRESETS_FILE )) {

			for (JsonObject::iterator it = root2->begin(); it != root2->end(); ++it) {
				// get id of preset
				const char * key = it->key;
				// extract the json object for each effect

				JsonObject& current = it->value.asObject();

				// compare to the name of current effect
				//	Serial.printf("[getPresets] Identified presets for %s (%s)\n", handle->name(), key);
				if (current.containsKey("effect") && current.containsKey("name")) {

					const char * presetkey = jsonBuffer.strdup(key); 
					const char * presetname = jsonBuffer.strdup(current["name"].asString());
					const char * preseteffect = jsonBuffer.strdup(current["effect"].asString());

					JsonObject & effect = presets.createNestedObject(presetkey);
					effect["name"] = presetname;
					effect["effect"] = preseteffect;


				}
			}

		}
		if (data) { delete[] data; }
	}

}


// todo....

uint8_t EffectManager::nextFree(JsonObject & root)
{

	for (JsonObject::iterator it = root.begin(); it != root.end(); ++it) {
		uint8_t ID = String(it->key).toInt();
//		Serial.printf("[EffectManager::nextFree] %u,", ID);
	}

	Serial.println();

}

bool EffectManager::Save(uint8_t ID, const char * name, bool overwrite)
{
	bool success = false;
	char * data = nullptr;

//	Serial.println("[newSave] Called");

	if (_currentHandle) {

		DynamicJsonBuffer jsonBuffer;
		const char * cID = jsonBuffer.strdup(String(ID).c_str());
		JsonObject * root = nullptr;

		if (parsespiffs(data, jsonBuffer, root, PRESETS_FILE )) {
//			Serial.println("[newSave] Existing Settings Loaded");
		} else {
			// if no file exists, or parse fails create new json object...
			root = &jsonBuffer.createObject();
		}

		//  temp working

		uint8_t xID = nextFree(*root);
		const char * xcID = jsonBuffer.strdup(String(xID).c_str());
//		Serial.printf("[newSave] Next Free %s\n", xcID);

		//

		if (root && _currentHandle->save(*root, cID, name)) {

//			Serial.println("[newSave] New Settings Added");
			File f = SPIFFS.open(PRESETS_FILE, "w");

			if (f) {

#ifdef jsonprettyprint
				root->prettyPrintTo(f);
#else
				root->printTo(f);
#endif
//				root->prettyPrintTo(Serial);

//				Serial.printf("[newSave] DONE Heap %u\n", ESP.getFreeHeap() );
				f.close();
				success = true;
			} else {
//				Serial.println("FILE OPEN error:  NOT saved");
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


bool EffectManager::Load(uint8_t ID)
{
	bool success = false;

	EffectHandler* handle = nullptr;

	if (_currentHandle && !_NextInLine) {
		handle = _currentHandle;
	} else if (_NextInLine) {
		handle = _NextInLine;
	}


	if (handle) {

		DynamicJsonBuffer jsonBuffer;
		const char * cID = jsonBuffer.strdup(String(ID).c_str()); //  this is a hack as i couldn't get it to work... can probably try without it now...

		JsonObject * root = nullptr;
		char * data = nullptr;
		bool success = false;

		if (parsespiffs(data, jsonBuffer, root, PRESETS_FILE )) {

			//  Is current effect same effect as is wanted.
			JsonObject& preset = (*root)[cID];
//			EffectHandler* effectp = nullptr;

			if (root) {

				if (root->containsKey(cID)) { // only process if the json contains a key with the right name

					if (handle && preset.containsKey("effect")) {
						if (strcmp(handle->name(), preset["effect"]) != 0) {
							handle = Start(preset["effect"].asString());
//						Serial.printf("[EffectManager::newLoad]  Effect changed to %s\n", effectp->name());
						}
						//else {
						//	effectp = _currentHandle;
						//}
					}

					// now can load the effect using json

					if (handle->parseJson(preset)) {
//						Serial.printf("[EffectManager::newLoad] Preset %s loaded\n", cID);
						handle->preset(ID);
						success = true;
					}

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
		_preset = 255;
	}
	return found;
}

bool EffectManager::convertcolor(JsonObject & root, const char * node)
{
	if (root.containsKey(node)) {

		String colorstring = root[node];

//		Serial.printf("[EffectManager::convertcolor] bcolorstring = %s\n", colorstring.c_str());
		JsonObject& colorroot = root.createNestedObject(node);

		colorroot["R"] = colorstring.substring(0, colorstring.indexOf(",")).toInt();
		colorstring = colorstring.substring( colorstring.indexOf(",") + 1, colorstring.length());
		colorroot["G"] = colorstring.substring(0, colorstring.indexOf(",")).toInt();
		colorroot["B"] = colorstring.substring( colorstring.indexOf(",") + 1, colorstring.length()).toInt();

//		Serial.println("[EffectManager::convertcolor] root" );
//		root.prettyPrintTo(Serial);
//		Serial.println();
		return true;

	}
	return false;
}



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
















