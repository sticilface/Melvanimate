/*---------------------------------------------

				Effect Manager

---------------------------------------------*/


#include "EffectManager.h"
#include <FS.h>


#define MAXLEDANIMATIONS 300 // number of pixels before animtor is not created
#define MAX_PRESET_FILE_SIZE 1000 // max size of permitted settings files... 
#define MAX_NUMBER_PRESET_FILES 10



//#define jsonprettyprint //  uses prettyPrintTo to send to SPIFFS... uses a lot more chars to pad the json.. a lot more..

//const char * PRESETS_FILE = "/presets_";

EffectManager::EffectManager() : _count(0), _firstHandle(nullptr), _currentHandle(nullptr), _lastHandle(nullptr), _toggleHandle(nullptr),
	_NextInLine(nullptr), _defaulteffecthandle(nullptr)
{};

bool EffectManager::Add(const char * name, EffectHandler* handle, bool animations, bool defaulteffect)
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

//		bool result = getPresets(_NextInLine, _numberofpresets, _presets, _preset_names);

		// if (result) {
		// 	DebugEffectManagerf("[EffectManager::Start] Presets Fetched\n");
		// } else {
		// 	DebugEffectManagerf("[EffectManager::Start] Presets Fetched Failed\n");

		// }

		if (_NextInLine->preset() != 255) {
			Load(_NextInLine->preset());
		} else {
			// try to load defaut...

			// find the file for specified preset, unless it is new preset.. ie  ID = 0;
			if (_presetcountS && _presetS) {
				for (uint8_t i = 0; i < _presetcountS; i++) {
					Presets_s * preset = &_presetS[i];
					if (preset->handle == _NextInLine) {

						if (!strcmp(preset->name, "Default") || !strcmp( preset->name, "default")) {

							if (Load(preset->file, preset->id  )) {
								DebugEffectManagerf("[Start] Default Loaded %u\n", preset->id);
								break;
							} else {
								DebugEffectManagerf("[Start] ERROR loading Default %u\n", preset->id);
							}


						}

					}
				}
			}






			//for (uint8_t i = 0; i < _presetcountS; i++) {




			// if (_preset_names[i]) {
			// 	const char * name = (const char *)_preset_names[i];
			// 	if (!strcmp(name, "Default") || !strcmp( name, "default")) {

			// 		if (Load(_presets[i])) {
			// 			DebugEffectManagerf("[Start] Default Loaded %u\n", _presets[i]);
			// 		} else {
			// 			DebugEffectManagerf("[Start] ERROR loading Default %u\n", _presets[i]);
			// 		}
			// 	}
			// }
			//}
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







bool EffectManager::removePreset(uint8_t ID)
{
	DebugEffectManagerf("[EffectManager::removePreset] id = %u\n ", ID);

	bool success = false;
	int FileID = -1;

	// find the file for specified preset, unless it is new preset.. ie  ID = 0;
	if (ID && _presetS) {
		for (uint8_t i = 0; i < _presetcountS; i++) {
			Presets_s * preset = &_presetS[i];
			if (preset->id == ID) {
				FileID = preset->file;
				break;
			}
		}
	}

	if (FileID == -1) {
		DebugEffectManagerf("[EffectManager::removePreset] FileID = -1\n ");
		return false;
	}

	{
		DynamicJsonBuffer jsonBuffer;
		const char * cID = jsonBuffer.strdup(String(ID).c_str());
		JsonArray * root = nullptr;
		char * data = nullptr;

		String filename = String(PRESETS_FILE) + String(FileID) + ".txt";


		if (parsespiffs(data, jsonBuffer, root, filename.c_str() ) )  {

			if (root) {


				int index = 0;

				for (JsonArray::iterator it = root->begin(); it != root->end(); ++it) {
					// *it contains the JsonVariant which can be casted as usuals
					//const char* value = *it;

					JsonObject& preset = *it;

					if (preset.containsKey("ID")) {

						if ( preset["ID"] == ID) {
							root->removeAt(index) ;
							DebugEffectHandlerf("[EffectHandler::save] preset %u removed\n", ID);
							break;
						}

					}

					index++;
					// this also works:
					//value = it->as<const char*>();

				}



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

		if (data) {
			delete[] data;
		}
	}

	if (success && Current()) {
		//getPresets(Current(), _numberofpresets, _presets, _preset_names); // goes here.. to go outofscope of the previous data[] and jsonbuffer... needs lot of heap...
		fillPresetArray();
		return true;
	} else {
		return false;
	}



}
/*
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

		String filename = " "; //PRESETS_FILE + String(handle->saveFileID) + ".txt";

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
*/

void EffectManager::addAllpresets(JsonObject & root)
{
	DebugEffectManagerf("[EffectManager::addAllpresets] called\n");

	JsonArray & array = root.createNestedArray("Presets");

	if (_presetS) {

		for (uint8_t i = 0; i < _presetcountS; i++) {

			Presets_s * preset = &_presetS[i];
			if (preset) {
				JsonObject & presetjson = array.createNestedObject();
				presetjson["ID"] = preset->id;
				presetjson["name"] = preset->name;
				presetjson["effect"] = preset->handle->name();
			}

		}
	}
	// Serial.println();
	// root.prettyPrintTo(Serial);
	// Serial.println();
}

bool EffectManager::addCurrentPresets(JsonObject & root)
{
	bool success = false;

	if (_presetcountS) {

		JsonArray & array = root.createNestedArray("currentpresets");

		if (_presetS) {

			for (uint8_t i = 0; i < _presetcountS; i++) {

				Presets_s * preset = &_presetS[i];

				if (preset) {

					if (preset->handle == Current()) {
						JsonObject & presetjson = array.createNestedObject();

						presetjson["ID"] = preset->id;
						presetjson["name"] = preset->name;
						//presetjson["effect"] = preset->handle->name();
						success = true;

					}
				}
			}
		}
	}

	return success;
}

// uint8_t EffectManager::nextFreePreset(JsonObject & root)
// {
// 	DebugEffectManagerf("[EffectManager::nextFreePreset] called\n");
// 	uint8_t i = 0;

// 	bool * array = new bool[255];
// 	//memset(array, 0, 255);
// 	if (array) {

// 		for (uint8_t j = 1; j < 255; j++) {
// 			array[j] = false;
// 		}


// 		for (JsonObject::iterator it = root.begin(); it != root.end(); ++it) {
// 			uint8_t ID = String(it->key).toInt();
// 			array[ID] = true;
// 		}


// 		for (i = 1; i < 255; i++) {
// 			if (array[i] == false) {
// 				break;
// 			}
// 		}


// 		delete[] array;

// 	} else {
// 		DebugEffectManagerf("[EffectManager::nextFreePreset] new array failed\n");
// 	}

// 	if (i == 254) {
// 		i = 0;
// 	}
// 	DebugEffectManagerf("[EffectManager::nextFreePreset] nextFree = %u \n", i);
// 	return i;

// }

// bool EffectManager::Save(String ID, const char * name, bool overwrite)
// {
// 	uint8_t IDout = ID.substring( ID.lastIndexOf(".") + 1, ID.length() ).toInt();
// 	// save only worked on loaded effect.  so just have to stip out the file bit from the response.
// 	return Save(IDout, name, overwrite);
// }

bool EffectManager::Save(uint8_t ID, const char * name, bool overwrite)
{
	bool success = false;
	char * data = nullptr;

	EffectHandler* handle = Current();

	DebugEffectManagerf("[EffectManager::Save] Called for %s\n", handle->name());

	if (handle && strlen(name) > 0 ) {

		int filePostfix = -1;

		// find the file for specified preset, unless it is new preset.. ie  ID = 0;
		if (ID && _presetS) {
			for (uint8_t i = 0; i < _presetcountS; i++) {
				Presets_s * preset = &_presetS[i];
				if (preset->id == ID) {
					filePostfix = preset->file;
					break;
				}
			}
		}

		// if no file is found use file 0 as default...
		if (filePostfix == -1) {
			filePostfix = 0;
		}

		String filename = PRESETS_FILE + String(filePostfix) + ".txt";


		File f = SPIFFS.open(filename, "r");

		if (f) {
			DebugEffectManagerf("[EffectManager::Save] Exists File name: %s, size:%u\n", filename.c_str(), f.size());
		} else {
			DebugEffectManagerf("[EffectManager::Save] New File name: %s\n", filename.c_str());
		}

		if (f) {
			if ( f.size() > MAX_PRESET_FILE_SIZE && !overwrite) {
				DebugEffectManagerf("[EffectManager::Save] %s too big (%u)creating new file\n", filename.c_str(),  f.size());
				f.close();
				filePostfix = _nextFreeFile();
				DebugEffectManagerf("[EffectManager::Save] _nextfreeFile = %s\n", String(filePostfix).c_str());
				if (filePostfix != -1) {
					filename = PRESETS_FILE + String(filePostfix) + ".txt";
				} else {
					DebugEffectManagerf("[EffectManager::Save] Unable to create new file\n");
					return false;
				}
			}
		}

		DynamicJsonBuffer jsonBuffer;
		//const char * cID = jsonBuffer.strdup(String(ID).c_str());
		JsonArray * root = nullptr;

		if (parsespiffs(data, jsonBuffer, root, filename.c_str() )) {
			DebugEffectManagerf("[EffectManager::Save] Existing Settings Loaded\n");
		} else {
			// if no file exists, or parse fails create new json object...
			DebugEffectManagerf("[EffectManager::Save] New jsonObject created\n");
			root = &jsonBuffer.createArray();
		}

		//
		if (ID == 0 && !overwrite) {
			//ID = nextFreePreset(*root);
			ID = nextFreePresetID();
		}


		//String sID = String(handle->saveFileID) + "." + String(ID);
//		const char * cID = jsonBuffer.strdup(String(ID).c_str());

		DebugEffectManagerf("[EffectManager::Save] ID chosen:%u\n", ID);

		//

		if (root) {

			if (handle->save(*root, ID, name)) {

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
		//getPresets(handle, _numberofpresets, _presets, _preset_names); // goes here.. to go outofscope of the previous data[] and jsonbuffer... needs lot of heap...
		fillPresetArray();
		return true;
	} else {
		return false;
	}

}

// bool EffectManager::Load(String decimalin)
// {
// 	uint8_t File = decimalin.substring(   0 , decimalin.lastIndexOf(".")  ).toInt();
// 	uint8_t ID = decimalin.substring(   decimalin.lastIndexOf(".") + 1, decimalin.length() ).toInt();

// 	DebugEffectManagerf("[EffectManager::Load] String converted to File (%u) ID (%u)\n", File, ID);

// 	if ( File == 0 || ID == 0) {
// 		return false;
// 	}

// 	return Load(File, ID);
// }

bool EffectManager::Load(uint8_t ID)
{
	// find the file for specified preset, unless it is new preset.. ie  ID = 0;
	if (ID && _presetS) {
		for (uint8_t i = 0; i < _presetcountS; i++) {
			Presets_s * preset = &_presetS[i];
			if (preset->id == ID) {
				return Load(preset->file, ID);

			}
		}
	}

	return false;
}

bool EffectManager::Load(uint8_t File, uint8_t ID)
{
	bool success = false;
	bool modechange = false;
	EffectHandler* handle = Current();

	DebugEffectManagerf("[EffectManager::Load] Called File: %u ID: %u\n", File, ID);

	if (handle) {

		DynamicJsonBuffer jsonBuffer(2000);

		//const char * cID = jsonBuffer.strdup( String(ID).c_str());

		JsonArray * root = nullptr;
		char * data = nullptr;

		String filename = PRESETS_FILE + String(File) + ".txt";


		if (parsespiffs(data, jsonBuffer, root, filename.c_str() )) {

			if (root) {

				for (JsonArray::iterator it = root->begin(); it != root->end(); ++it) {

					JsonObject& preset = *it;

					if (preset.containsKey("ID")) {

						if ( preset["ID"] == ID) {

							uint8_t tempID = preset["ID"];
							const char * name = preset["name"];

							DebugEffectManagerf("[EffectManager::Load] Checking %u (%s)\n", tempID , name );

							if (handle && preset.containsKey("effect")) {

								if (strcmp(handle->name(), preset["effect"]) != 0) {
									modechange = true;
									handle = _findhandle(preset["effect"].asString());
									_NextInLine = handle;
									_prepareAnimator();
									handle->InitVars();

									if (_defaulteffecthandle) {
										if (handle != _defaulteffecthandle) {
											_toggleHandle = handle;
										}
									}

									//handle = Start(preset["effect"].asString());
									DebugEffectManagerf("[EffectManager::Load]  Effect changed to %s\n", handle->name());
								}



								// now can load the effect using json
								if (handle) {
									if (handle->parseJson(preset, true)) {
										DebugEffectManagerf("[EffectManager::Load] Preset %u loaded for %s\n", ID, handle->name());
										handle->preset(ID);
										success = true;
										break;
									} else {
										DebugEffectManagerf("[EffectManager::Load] ERROR Preset %u for %s, parseJson returned false\n", ID, handle->name());
										//preset.prettyPrintTo(Serial);
										//Serial.println();
									}
								}
							}

						}

					}

				}

			} else {
				DebugEffectManagerf("[EffectManager::Load] Root is nullptr\n");
			}
		} else {
			DebugEffectManagerf("[EffectManager::Load] Parsespiffs Failed\n");
		}

		if (data) { delete[] data; }
	}

	if (success) {
		// if (modechange && handle) {
		// 	getPresets(handle, _numberofpresets, _presets, _preset_names); // goes here.. to go outofscope of the previous data[] and jsonbuffer... needs lot of heap...
		// }
		return true;
	} else {
		return false;
	}
}


// Possible values from 1 to 32768, and there some helpful constants defined as...
// NEO_MILLISECONDS        1    // ~65 seconds max duration, ms updates
// NEO_CENTISECONDS       10    // ~10.9 minutes max duration, centisecond updates
// NEO_DECISECONDS       100    // ~1.8 hours max duration, decisecond updates
// NEO_SECONDS          1000    // ~18.2 hours max duration, second updates
// NEO_DECASECONDS     10000    // ~7.5 days, 10 second updates

void EffectManager::_prepareAnimator()
{
	if (_NextInLine->animate() && strip->PixelCount() <= MAXLEDANIMATIONS) {
		if (!animator) {
			DebugEffectManagerf("[EffectManager::Start] Animator Created\n");

			animator = new NeoPixelAnimator(strip->PixelCount(),NEO_MILLISECONDS);
		
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

bool EffectManager::fillPresetArray()
{
#ifdef DebugEffectManagerf
	uint32_t starttime = millis();
#endif


	uint8_t numberofpresets = 0;

	if (_presetS) {
		delete[] _presetS;
		_presetS = nullptr;
	}


	//  iterate through all files and count the presets
	{
		Dir dir = SPIFFS.openDir("/");
		while (dir.next()) {
			String fileName = dir.fileName();

			if (fileName.startsWith(PRESETS_FILE) && fileName.endsWith(".txt") && !fileName.endsWith(".corrupt.txt")) {

				uint8_t FileID = fileName.substring(   strlen(PRESETS_FILE) , fileName.lastIndexOf(".txt")  ).toInt();
				//	DebugEffectManagerf("[EffectManager::fillPresetArray] Adding presets from %s, FileID %u\n", fileName.c_str(), FileID);

				DynamicJsonBuffer jsonBuffer2(2000);
				JsonArray * root2 = nullptr;
				char * data = nullptr;

				if (parsespiffs(data, jsonBuffer2, root2, fileName.c_str() )) {

					for (JsonArray::iterator it = root2->begin(); it != root2->end(); ++it) {
						// get id of preset
						JsonObject & preset = *it;

						// compare to the name of current effect
						if (preset.containsKey("effect") && preset.containsKey("name")) {

							numberofpresets++;

						}
					}

				} else {
					String newFileName = fileName.substring(0, fileName.length() - 3) + "corrupt.txt";
					SPIFFS.rename(fileName, newFileName);
					DebugEffectManagerf("[EffectManager::fillPresetArray] %s parse failed. File renamed to %s\n", fileName.c_str(), newFileName.c_str());
					continue;
				}

				if (data) { delete[] data; }
			}
		}
	}

	//  now create the presetS structure the correct size

	if (numberofpresets) {
		_presetS = new Presets_s[numberofpresets];
		_presetcountS = numberofpresets;
	}

	uint8_t arrayposition = 0;

	// now iterate through again.. and assign variables to presetS

	if (_presetS)

	{

		Dir dir = SPIFFS.openDir("/");
		while (dir.next()) {
			String fileName = dir.fileName();

			if (fileName.startsWith(PRESETS_FILE) && fileName.endsWith(".txt") && !fileName.endsWith(".corrupt.txt")) {

				uint8_t FileID = fileName.substring(   strlen(PRESETS_FILE) , fileName.lastIndexOf(".txt")  ).toInt();
				//DebugEffectManagerf("[EffectManager::addAllpresets] Adding presets from %s, FileID %u\n", fileName.c_str(), FileID);

				DynamicJsonBuffer jsonBuffer2(2000);
				JsonArray * root2 = nullptr;
				char * data = nullptr;

				if (parsespiffs(data, jsonBuffer2, root2, fileName.c_str() )) {

					for (JsonArray::iterator it = root2->begin(); it != root2->end(); ++it) {
						// get id of preset
						// extract the json object for each effect

						JsonObject& presetJSON = *it;

						// compare to the name of current effect
						if (presetJSON.containsKey("effect") && presetJSON.containsKey("name")) {

							Presets_s * preset = &_presetS[arrayposition++];

							if (preset) {
								preset->file = FileID;
								preset->id = presetJSON["ID"];
								preset->handle = _findhandle(presetJSON["effect"].asString());
								preset->setname(presetJSON["name"]);
							}

						}
					}

				}

				if (data) { delete[] data; }
			}
		}
	}

	DebugEffectManagerf("[EffectManager::fillPresetArray] presets found: %u, timetaken: %u\n", _presetcountS, millis() - starttime);

	dumpPresetArray();

	DebugEffectManagerf("[EffectManager::fillPresetArray] Next Free: %u\n", nextFreePresetID() );



}

void EffectManager::removeAllpresets()

{
	Dir dir = SPIFFS.openDir("/");
	while (dir.next()) {
		String fileName = dir.fileName();

		if (fileName.startsWith(PRESETS_FILE) && fileName.endsWith(".txt") && !fileName.endsWith(".corrupt.txt")) {
			SPIFFS.remove(fileName);
		}

	}

	fillPresetArray(); 

}

void EffectManager::dumpPresetArray()
{
	DebugEffectManagerf("[dumpPresetArray] %u presets found \n", _presetcountS);

	for (uint8_t i = 0; i < _presetcountS; i++) {
		Presets_s * preset = &_presetS[i];
		if (preset) {
			DebugEffectManagerf("  [%3u] File: %3u, ID: %3u, effect: %30s, name: %30s\n", i, preset->file, preset->id, preset->handle->name(), preset->name);
		}
	}
}

uint8_t EffectManager::nextFreePresetID()
{
	DebugEffectManagerf("[EffectManager::nextFreePresetID] called\n");
	uint8_t i = 0;

	if (_presetS) {

		bool * array = new bool[255];
		//memset(array, 0, 255);
		if (array) {

			for (uint8_t j = 0; j < 255; j++) {
				array[j] = false;
			}

			for (uint8_t j = 0; j < _presetcountS; j++) {
				Presets_s * preset = &_presetS[j];
				array[preset->id] = true;
			}

			for (i = 1; i < 255; i++) {
				if (array[i] == false) {
					break;
				}
			}

			delete[] array;

		} else {
			DebugEffectManagerf("[EffectManager::nextFreePresetArray] new array failed\n");
		}

		if (i == 254) {
			i = 0;
		}
		DebugEffectManagerf("[EffectManager::nextFreePresetArray] nextFree = %u \n", i);
	} else {
		//  if there are no presets return 1 as the first.
		i = 1;
	}
	return i;
}


int EffectManager::_nextFreeFile()
{
	struct presetfile_s {
		bool isfile{false};
		uint32_t size{0};
	};

	DebugEffectManagerf("[_nextFreeFile] called\n");

	presetfile_s * presetfile = new presetfile_s[MAX_NUMBER_PRESET_FILES];

	if (!presetfile) {
		return 0;
	}

	Dir dir = SPIFFS.openDir("/");
	while (dir.next()) {

		String fileName = dir.fileName();

		if (fileName.startsWith(PRESETS_FILE)) {

			uint8_t FileID = fileName.substring( strlen(PRESETS_FILE) , fileName.lastIndexOf(".txt") ).toInt();
			presetfile_s * current = &presetfile[FileID];
			current->isfile = true;
			File f = SPIFFS.open(fileName, "r");
			current->size = f.size();
			f.close();
		}

	}

#ifdef DebugEffectManager

	DebugEffectManagerf("[_nextFreeFile] Listing all:\n");

	for (uint8_t i = 0; i < MAX_NUMBER_PRESET_FILES; i++) {
		presetfile_s * current = &presetfile[i];
		DebugEffectManagerf("  [%u] %s (%u)\n", i, (current->isfile) ? "true" : "false", current->size );
	}

#endif

	for (uint8_t i = 0; i < MAX_NUMBER_PRESET_FILES; i++) {
		presetfile_s * current = &presetfile[i];
		if (current->isfile == false || current->size < MAX_PRESET_FILE_SIZE) {
			return i;
		}
	}

	return -1;

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
















