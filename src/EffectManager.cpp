/*---------------------------------------------

				Effect Manager

---------------------------------------------*/


#include "EffectManager.h"
#include "FS.h"
#include "NeopixelBus.h"

#define MAXLEDANIMATIONS 300

extern NeoPixelBus * strip;
extern NeoPixelAnimator * animator;

//#define jsonprettyprint //  uses prettyPrintTo to send to SPIFFS... uses a lot more chars to pad the json.. a lot more..

//const char * PRESETS_FILE = "/presets_";

EffectManager::EffectManager() : _count(0), _firstHandle(nullptr), _currentHandle(nullptr), _lastHandle(nullptr), _toggleHandle(nullptr),
	_NextInLine(nullptr), _defaulteffecthandle(nullptr)
{};

bool EffectManager::Add(uint8_t file, const char * name, EffectHandler* handle, bool animations, bool defaulteffect)
{
	_count++;

	if (defaulteffect && !_defaulteffecthandle) {
		_defaulteffecthandle = handle;
	}

	if (!_lastHandle) {
		_firstHandle = handle; //  set the first handle
		_firstHandle->name(name); // set its name in the handler... abstract it out so user doesn't have to
		_lastHandle = handle;  // set this so we know first slot gone.
	} else {
		handle->previous(_lastHandle); //  sets the previous handle..
		_lastHandle->next(handle); // give the last handler address of next
		_lastHandle = handle;  // move on..
		_lastHandle->name(name);  // give it name...
	}

	handle->animate(animations);
	handle->saveFileID = file;
	DebugEffectManagerf("ADDED EFFECT %u: %s\n", _count, handle->name());
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

bool EffectManager::Start(EffectHandler* handler)
{

#ifdef DebugEffectManager
	uint32_t heap;
#endif
	Stop();


	if (handler) {
		_NextInLine = handler;

		_prepareAnimator();

#ifdef DebugEffectManager
		heap = ESP.getFreeHeap();
#endif
		_NextInLine->InitVars();

		bool result = getPresets(_NextInLine, _numberofpresets, _presets, _preset_names);

		if (result) {
			DebugEffectManagerf("[EffectManager::Start] Presets Fetched\n");
		} else {
			DebugEffectManagerf("[EffectManager::Start] Presets Fetched Failed\n");

		}

		if (_NextInLine->preset() != 255) {
			Load(_NextInLine->preset());
		} else {
			// try to load defaut...
			for (uint8_t i = 0; i < _numberofpresets; i++) {
				if (_preset_names[i]) {
					const char * name = (const char *)_preset_names[i];
					if (!strcmp(name, "Default") || !strcmp( name, "default")) {

						if (Load(_presets[i])) {
							DebugEffectManagerf("[Start] Default Loaded %u\n", _presets[i]);
						} else {
							DebugEffectManagerf("[Start] ERROR loading Default %u\n", _presets[i]);
						}
					}
				}
			}
		}

		//  This sets the toggle... as long as it is not the default handle... ie... Off....
		if (_defaulteffecthandle) {
			if (handler != _defaulteffecthandle) {
				_toggleHandle = handler;
			}
		}
#ifdef DebugEffectManager
		int used = heap - ESP.getFreeHeap();
		if (used < 0) { used = 0; }
#endif

		DebugEffectManagerf("[Start] Heap Used by %s (%u)\n", handler->name(), used);

		return true;
	} else {
		//  if no handle.. try to start default....
		if (_defaulteffecthandle) {
			Start(_defaulteffecthandle->name());
			return false; 
		}
		// if that fails.. bail...
		return false;
	}
}


EffectHandler* EffectManager::Current()
{
	if (_NextInLine) {
		return _NextInLine;
	} else {
		return _currentHandle;
	}
};


// not sure about this implementation....
bool EffectManager::Next()
{
	Start(_currentHandle->next()->name());
};
bool EffectManager::Previous()
{
	Start(_currentHandle->previous()->name());
};

bool EffectManager::Stop()
{
	if (_currentHandle) {
		_currentHandle->EndVars();
		return _currentHandle->Stop();
	}
};

bool EffectManager::Pause()
{
	if (_currentHandle) { return _currentHandle->Pause(); }
};

void EffectManager::Refresh()
{
	if (_currentHandle) { _currentHandle->Refresh(); }

}

void EffectManager::loop()
{

	_process();
};

// void EffectManager::SetTimeout(uint32_t time)
// {
// 	if (_currentHandle) { _currentHandle->SetTimeout(time); }
// }

// void EffectManager::SetTimeout(const char * name, uint32_t time)
// {

// 	EffectHandler* handler;
// 	for (handler = _firstHandle; handler; handler = handler->next()) {
// 		if ( strcmp( handler->name(), name) == 0) {
// 			break;
// 		}
// 	}

// 	if (handler) { handler->SetTimeout(time); }

// }

void EffectManager::_process()
{
	bool waiting = false;

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

	if (handler) {
		return handler->name();
	} else {
		return "";
	}
}




//  could try and package this up... maybe using a struct... ... maybe...
bool EffectManager::parsespiffs(char *& data,  DynamicJsonBuffer & jsonBuffer, JsonObject *& root, const char * file_name)
{
	uint32_t starttime = millis();
	uint32_t filesize;

	//DynamicJsonBuffer jsonBuffer;

	File f = SPIFFS.open(file_name, "r");
	bool success = false;

	if (!f) {
		DebugEffectManagerf("[parsespiffs] File open Failed\n");
	}

	if (f && f.size()) {
		filesize = f.size();
		//Serial.println("[parsespiffs] pre-malloc");

		data = new char[f.size()];
		// prevent nullptr exception if can't allocate
		if (data) {
			//DebugEffectManagerf("[parsespiffs] Buffer size %u\n", f.size());

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
			DebugEffectManagerf("[parsespiffs] malloc failed\n");
		}
	}

	f.close();

	if (success) {
		DebugEffectManagerf("[parsespiffs] heap: %u, FileName: %s, FileSize: %u, parsetime: %u, jsonBufferSize: %u\n", ESP.getFreeHeap(), file_name, filesize, millis() - starttime, jsonBuffer.size());
		return true;
	} else {
		return false;
	}
}

bool EffectManager::removePreset(String ID)
{

	uint8_t File = ID.substring(   0 , ID.lastIndexOf(".")  ).toInt();
	uint8_t IDout = ID.substring(   ID.lastIndexOf(".") + 1, ID.length() ).toInt();
	return removePreset(File, IDout); 
}


bool EffectManager::removePreset(uint8_t FileID, uint8_t ID)
{
	bool success = false;

	{
		DynamicJsonBuffer jsonBuffer;
		const char * cID = jsonBuffer.strdup(String(ID).c_str());
		JsonObject * root = nullptr;
		char * data = nullptr;

		String filename = String(PRESETS_FILE) + String(FileID) + ".txt";


		if (parsespiffs(data, jsonBuffer, root, filename.c_str() ) )  {

			if (root) {

				if (root->containsKey(cID)) {
					root->remove(cID);
					File f = SPIFFS.open( filename.c_str() , "w");

					if (f) {

#ifdef jsonprettyprint
						root->prettyPrintTo(f);
#else
						root->printTo(f);
#endif
						f.close();
						success = true;
						DebugEffectManagerf("[removePreset] [%s] Setting Removed\n", cID);
					} else {
						DebugEffectManagerf("[removePreset] FILE OPEN error:  NOT saved\n");
					}

				}

			}

		}

		if (data) {
			delete[] data;
		}
	}

	if (success && Current()) {
		getPresets(Current(), _numberofpresets, _presets, _preset_names); // goes here.. to go outofscope of the previous data[] and jsonbuffer... needs lot of heap...
		return true;
	} else {
		return false;
	}



}

bool EffectManager::getPresets(EffectHandler * handle, uint8_t& numberofpresets, uint8_t *& presets, char **& preset_names)
{
	char * data = nullptr;

	// delete any existing preset information.

	DebugEffectManagerf("[EffectManager::getPresets] Fetching for %s\n", handle->name());





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

		String filename = PRESETS_FILE + String(handle->saveFileID) + ".txt";

		// uint8_t FileID = filename.substring(   strlen(PRESETS_FILE) , filename.indexOf(".txt")  ).toInt();
		// DebugEffectManagerf("[EffectManager::getPresets] ** check from %s, FileID %u\n", filename.c_str(), FileID) ;

		if (parsespiffs(data, jsonBuffer, root, filename.c_str() )) {

			//delay(0);

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
							DebugEffectManagerf("[getPresets] Found preset for %s (%s)\n", handle->name(), key);
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
	DebugEffectManagerf("[EffectManager::addAllpresets] called\n");

	JsonObject & presets = root.createNestedObject("Presets");

	{
		{

			Dir dir = SPIFFS.openDir("/");
			while (dir.next()) {
				String fileName = dir.fileName();

				if (fileName.startsWith(PRESETS_FILE)) {

					uint8_t FileID = fileName.substring(   strlen(PRESETS_FILE) , fileName.lastIndexOf(".txt")  ).toInt();
					DebugEffectManagerf("[EffectManager::addAllpresets] Adding presets from %s, FileID %u\n", fileName.c_str(), FileID);

					DynamicJsonBuffer jsonBuffer2(2000);
					JsonObject * root2 = nullptr;
					char * data = nullptr;

					if (parsespiffs(data, jsonBuffer2, root2, fileName.c_str() )) {

						for (JsonObject::iterator it = root2->begin(); it != root2->end(); ++it) {
							// get id of preset
							const char * key = it->key;
							// extract the json object for each effect

							JsonObject& current = it->value.asObject();

							// compare to the name of current effect
							if (current.containsKey("effect") && current.containsKey("name")) {

								String fullkey = String(FileID) + "." + key; 

								const char * presetkey = jsonBuffer.strdup(fullkey.c_str());
								const char * presetname = jsonBuffer.strdup(current["name"].asString());
								const char * preseteffect = jsonBuffer.strdup(current["effect"].asString());

								JsonObject & effect = presets.createNestedObject(presetkey);
								//effect["FileID"] = FileID; 
								effect["name"] = presetname;
								effect["effect"] = preseteffect;

							}
						}

					}

					if (data) { delete[] data; }
				}
			}
		}
	}
	// Serial.println();
	// root.prettyPrintTo(Serial);
	// Serial.println(); 
}

uint8_t EffectManager::nextFreePreset(JsonObject & root)
{
	DebugEffectManagerf("[EffectManager::nextFreePreset] called\n");
	uint8_t i = 0;

	bool * array = new bool[255];
	//memset(array, 0, 255);
	if (array) {

		for (uint8_t j = 1; j < 255; j++) {
			array[j] = false;
		}


		for (JsonObject::iterator it = root.begin(); it != root.end(); ++it) {
			uint8_t ID = String(it->key).toInt();
			array[ID] = true;
		}


		for (i = 1; i < 255; i++) {
			if (array[i] == false) {
				break;
			}
		}


		delete[] array;

	} else {
		DebugEffectManagerf("[EffectManager::nextFreePreset] new array failed\n");
	}

	if (i == 254) {
		i = 0;
	}
	DebugEffectManagerf("[EffectManager::nextFreePreset] nextFree = %u \n", i);
	return i;

}

bool EffectManager::Save(String ID, const char * name, bool overwrite)
{
	uint8_t IDout = ID.substring( ID.lastIndexOf(".") + 1, ID.length() ).toInt();
	// save only worked on loaded effect.  so just have to stip out the file bit from the response. 
	return Save(IDout, name, overwrite);
}

bool EffectManager::Save(uint8_t ID, const char * name, bool overwrite)
{
	bool success = false;
	char * data = nullptr;

	EffectHandler* handle = Current();

	DebugEffectManagerf("[EffectManager::Save] Called for %s\n", handle->name());

	if (handle && strlen(name) > 0 ) {

		//uint8_t save_ID = handle->saveFileID;

		String filename = PRESETS_FILE + String(handle->saveFileID) + ".txt";
		DebugEffectManagerf("[EffectManager::Save] File name: %s\n", filename.c_str());

		File f = SPIFFS.open(filename, "r");

		if (f) {
			if ( f.size() > 2000) {
				DebugEffectManagerf("[EffectManager::Save] File Too Big Delete some effects: %u\n", f.size());
				return false;
			}
		}

		DynamicJsonBuffer jsonBuffer;
		//const char * cID = jsonBuffer.strdup(String(ID).c_str());
		JsonObject * root = nullptr;

		if (parsespiffs(data, jsonBuffer, root, filename.c_str() )) {
			DebugEffectManagerf("[EffectManager::Save] Existing Settings Loaded\n");
		} else {
			// if no file exists, or parse fails create new json object...
			DebugEffectManagerf("[EffectManager::Save] New jsonObject created\n");
			root = &jsonBuffer.createObject();
		}

		//  temp working
		if (ID == 0 && !overwrite) {
			ID = nextFreePreset(*root);
		}


		//String sID = String(handle->saveFileID) + "." + String(ID);
		const char * cID = jsonBuffer.strdup(String(ID).c_str());

		DebugEffectManagerf("[EffectManager::Save] ID chosen:%s\n", cID);

		//

		if (root) {
			if (handle->save(*root, cID, name)) {

				DebugEffectManagerf("[EffectManager::Save] New Settings Added to json\n");
				File f = SPIFFS.open(filename, "w");

				if (f) {

#ifdef jsonprettyprint
					root->prettyPrintTo(f);
#else
					root->printTo(f);
#endif
//				root->prettyPrintTo(Serial); Serial.println();

					//DebugEffectManagerf("[EffectManager::Save] DONE Heap %u\n", ESP.getFreeHeap() );
					f.close();
					handle->preset(ID);
					DebugEffectManagerf("[EffectManager::Save] %s SAVED\n", filename.c_str());
					success = true;

				} else {
					DebugEffectManagerf("[EffectManager::Save]ERROR FILE not OPEN: NOT saved\n");
				}

			} else {
				DebugEffectManagerf("[EffectManager::Save] save handler (addJson) returned false\n");
			}
		}
	}

	if (data) {
		delete[] data;
	}

	if (success && handle) {
		// puts the new saved effect into memory 
		getPresets(handle, _numberofpresets, _presets, _preset_names); // goes here.. to go outofscope of the previous data[] and jsonbuffer... needs lot of heap...
		return true;
	} else {
		return false;
	}

}

bool EffectManager::Load(String decimalin)
{
	uint8_t File = decimalin.substring(   0 , decimalin.lastIndexOf(".")  ).toInt();
	uint8_t ID = decimalin.substring(   decimalin.lastIndexOf(".") + 1, decimalin.length() ).toInt();

	DebugEffectManagerf("[EffectManager::Load] String converted to File (%u) ID (%u)\n", File, ID); 

	if ( File == 0 || ID == 0) 
	{
		return false; 
	}

	return Load(File, ID); 
}

bool EffectManager::Load(uint8_t ID)
{
	return Load( Current()->saveFileID, ID ); 
}

bool EffectManager::Load(uint8_t File, uint8_t ID)
{
	bool success = false;
	bool modechange = false;
	EffectHandler* handle = Current();

	DebugEffectManagerf("[EffectManager::Load] Called File: %u ID: %u\n", File, ID);

	if (handle) {

		DynamicJsonBuffer jsonBuffer(2000);

		const char * cID = jsonBuffer.strdup( String(ID).c_str());

		JsonObject * root = nullptr;
		char * data = nullptr;

		String filename = PRESETS_FILE + String(File) + ".txt";


		if (parsespiffs(data, jsonBuffer, root, filename.c_str() )) {

			//  Is current effect same effect as is wanted.
			JsonObject& preset = (*root)[cID];

			if (root) {

				if (root->containsKey(cID)) { // only process if the json contains a key with the right name

					if (handle && preset.containsKey("effect")) {
						if (strcmp(handle->name(), preset["effect"]) != 0) {
							modechange = true;
							handle = _findhandle(preset["effect"].asString());
							_NextInLine = handle;
							_prepareAnimator();
							_NextInLine->InitVars();

							if (_defaulteffecthandle) {
								if (handle != _defaulteffecthandle) {
									_toggleHandle = handle;
								}
							}

							//handle = Start(preset["effect"].asString());
							DebugEffectManagerf("[EffectManager::Load]  Effect changed to %s\n", handle->name());
						}

					}

					// now can load the effect using json
					if (handle) {
						if (handle->parseJson(preset)) {
							DebugEffectManagerf("[EffectManager::Load] Preset %s loaded for %s\n", cID, handle->name());
							handle->preset(ID);
							success = true;
						} else {
							DebugEffectManagerf("[EffectManager::Load] ERROR Preset %s for %s, parseJson returned false\n", cID, handle->name());
							//preset.prettyPrintTo(Serial);
							//Serial.println(); 
						}
					}
				} else {
					DebugEffectManagerf("[EffectManager::Load] ERROR Preset %s Not Found in File\n", cID);
				}
			}
		}

		if (data) { delete[] data; }
	}

	if (success) {
		if (modechange && handle) {
			getPresets(handle, _numberofpresets, _presets, _preset_names); // goes here.. to go outofscope of the previous data[] and jsonbuffer... needs lot of heap...
		}
		return true;
	} else {
		return false;
	}
}


bool EffectManager::convertcolor(JsonObject & root, const char * node)
{
	if (root.containsKey(node)) {

		String colorstring = root[node];

		DebugEffectManagerf("[EffectManager::convertcolor] bcolorstring = %s\n", colorstring.c_str());
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

void EffectManager::_prepareAnimator()
{
	if (_NextInLine->animate() && strip->PixelCount() <= MAXLEDANIMATIONS) {
		if (!animator) {
			DebugEffectManagerf("[EffectManager::Start] Animator Created\n");
			animator = new NeoPixelAnimator(strip);
		} else {
			DebugEffectManagerf("[EffectManager::Start] Animator Already in Place\n");
		}
	} else {
		if (animator) {
			DebugEffectManagerf("[EffectManager::Start] Animator Deleted\n");
			delete animator;
			animator = nullptr;
		} else {
			DebugEffectManagerf("[EffectManager::Start] Animator Already Deleted\n");
		}
	}
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
















