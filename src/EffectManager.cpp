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
{
};

bool EffectManager::Add(const char * name, EffectHandler* handle, bool defaulteffect)
{
        handle->index = _count++;

        if (defaulteffect && !_defaulteffecthandle) {
                _defaulteffecthandle = handle;
        }

        if (!_lastHandle) {
                _firstHandle = handle; //  set the first handle
                _firstHandle->name(name); // set its name in the handler... abstract it out so user doesn't have to
                _lastHandle = handle; // set this so we know first slot gone.
        } else {
                handle->previous(_lastHandle); //  sets the previous handle..
                _lastHandle->next(handle); // give the last handler address of next
                _lastHandle = handle; // move on..
                _lastHandle->name(name); // give it name...
        }

        DebugEffectManagerf("ADDED EFFECT %u: %s\n", _count, handle->name());

        return true;


}

EffectHandler* EffectManager::_findhandle(uint8_t index)
{
        EffectHandler* handler;
        bool found = false;
        uint8_t count = 0;
        for (handler = _firstHandle; handler; handler = handler->next()) {
                if (count == index) {
                        found = true;
                        break;
                }
                count++;
        }

        if (found) {
                return handler;
        } else {
                return nullptr;
        }
}

EffectHandler* EffectManager::_findhandle(const char * handle)
{
        EffectHandler* handler;
        bool found = false;
        for (handler = _firstHandle; handler; handler = handler->next()) {
                if ( strcasecmp( handler->name(), handle) == 0) {
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

//  starts an effect without all the preset and default checking... used at boot to load data from RTC.
bool EffectManager::Startblank(uint8_t index) {

        EffectHandler* handler = _findhandle(index);

        if (handler) {
								Stop();
                RTC::addr_counter = _rtcman.get().data; //
                _NextInLine = handler;
								_NextInLine->InitVars();
                return true;
        } else {
                return false;
        }

}


bool EffectManager::Start(EffectHandler* handler)
{
#ifdef DebugEffectManager
        DebugEffectManagerf("\n");
        DebugEffectManagerf("[Start] called for %s\n", handler->name());
        uint32_t heap;
#endif

        if (handler == _currentHandle) {
                return false;
        }

        Stop();

        RTC::addr_counter = _rtcman.get().data; // resets variable storing area to size of initial struc

        //Serial.printf( "Size of rtc_data_t = %u\n" , sizeof(rtc_data_t));

        if (handler) {
                _NextInLine = handler;

#ifdef DebugEffectManager
                heap = ESP.getFreeHeap();
#endif

                _NextInLine->InitVars();

                if (_NextInLine->preset() != 255) {
                        Load(_NextInLine->preset());
                } else {
                        // try to load defaut...

                        // find the file for specified preset, unless it is new preset.. ie  ID = 0;
                        if (_presetcountS && _presetS) {
                                for (uint8_t i = 0; i < _presetcountS; i++) {
                                        Presets_s * preset = &_presetS[i];
                                        if (preset->handle == _NextInLine) {

                                                if (!strcasecmp(preset->name, "Default") || !strcmp( preset->name, "default")) {

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

                }

                //  This sets the toggle... as long as it is not the default handle... ie... Off....
                if (_defaulteffecthandle) {
                        if (handler != _defaulteffecthandle) {
                                _toggleHandle = handler;
                        }
                }

								//  this stuff can go here has the bail options call start again if no handle...
                rtc_data_t & data = _rtcman.get();
                data.effect = _NextInLine->index;
                if (_NextInLine->preset() != 255) {
                        data.state = RTC_manager::PRESET;
                } else {
                        data.state = RTC_manager::NONE;
                }

                _rtcman.save();
								_NextInLine->SaveRTCdata(); 


#ifdef DebugEffectManager
                int used = heap - ESP.getFreeHeap();
                if (used < 0) { used = 0; }

                DebugEffectManagerf("[Start] Heap Used by %s (%u)\n", handler->name(), used);
#endif

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
                DebugEffectManagerf("[EffectManager::Stop] called for %s\n", _currentHandle->name());
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


void EffectManager::_process()
{
        bool waiting = false;

        if (_waitFn)  {
                waiting = _waitFn();
        }


        //  This flips over to next effect asyncstyle....
        if (!waiting && _NextInLine) {
                DebugEffectManagerf("[EffectManager::_process] _currentHandle NOW = %s\n", _NextInLine->name());
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
                                                        root->removeAt(index);
                                                        DebugEffectHandlerf("[EffectHandler::save] preset %u removed\n", ID);
                                                        break;
                                                }

                                        }

                                        index++;
                                        // this also works:
                                        //value = it->as<const char*>();

                                }



                                File f = SPIFFS.open( filename.c_str(), "w");

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
                fillPresetArray();
                return true;
        } else {
                return false;
        }



}


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
                                if (preset->handle) {
                                        presetjson["effect"] = preset->handle->name();
                                }
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
//  uint8_t File = decimalin.substring(   0 , decimalin.lastIndexOf(".")  ).toInt();
//  uint8_t ID = decimalin.substring(   decimalin.lastIndexOf(".") + 1, decimalin.length() ).toInt();

//  DebugEffectManagerf("[EffectManager::Load] String converted to File (%u) ID (%u)\n", File, ID);

//  if ( File == 0 || ID == 0) {
//   return false;
//  }

//  return Load(File, ID);
// }
bool EffectManager::Load(const char * name)
{
        // find the file for specified preset, unless it is new preset.. ie  ID = 0;
        DebugEffectManagerf("[EffectManager::Load char*] Called File: %s\n", name);

        if (name && _presetS) {

                for (uint8_t i = 0; i < _presetcountS; i++) {
                        Presets_s * preset = &_presetS[i];
                        if (!strcasecmp(preset->name, name) ) {
                                return Load(preset->file, preset->id);
                        }
                }
        }

        return false;
}

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

                                                        DebugEffectManagerf("[EffectManager::Load] Checking %u (%s)\n", tempID, name );

                                                        if (handle && preset.containsKey("effect")) {

                                                                if (strcmp(handle->name(), preset["effect"]) != 0) {
                                                                        modechange = true;
                                                                        handle = _findhandle(preset["effect"].asString());
                                                                        _NextInLine = handle;
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
                //  getPresets(handle, _numberofpresets, _presets, _preset_names); // goes here.. to go outofscope of the previous data[] and jsonbuffer... needs lot of heap...
                // }
                return true;
        } else {
                return false;
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

                                uint8_t FileID = fileName.substring(   strlen(PRESETS_FILE), fileName.lastIndexOf(".txt")  ).toInt();
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

                                uint8_t FileID = fileName.substring(   strlen(PRESETS_FILE), fileName.lastIndexOf(".txt")  ).toInt();
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

#ifdef DebugEffectManager
        DebugEffectManagerf("[EffectManager::fillPresetArray] presets found: %u, timetaken: %u\n", _presetcountS, millis() - starttime);
        dumpPresetArray();
        DebugEffectManagerf("[EffectManager::fillPresetArray] Next Free: %u\n", nextFreePresetID() );
#endif


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

#ifdef DebugEffectManager
void EffectManager::dumpPresetArray()
{
        DebugEffectManagerf("[dumpPresetArray] %u presets found \n", _presetcountS);

        for (uint8_t i = 0; i < _presetcountS; i++) {
                Presets_s * preset = &_presetS[i];
                if (preset && preset->handle) {
                        DebugEffectManagerf("  [%3u] File: %3u, ID: %3u, effect: %30s, name: %30s\n", i, preset->file, preset->id, preset->handle->name(), preset->name);
                }
        }
}
#endif

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
                bool isfile {false};
                uint32_t size {0};
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

                        uint8_t FileID = fileName.substring( strlen(PRESETS_FILE), fileName.lastIndexOf(".txt") ).toInt();
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
