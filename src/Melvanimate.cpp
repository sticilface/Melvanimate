#include "Melvanimate.h"

#include <MD5Builder.h>
#include <ArduinoJson.h>
#include <ESP8266mDNS.h>
#include "JsonPackage.h"

#define LAST_MODIFIED_DATE "Wed, 14 Sep 2016 17:00:00 GMT"

NeoPixelAnimator * animator = nullptr;
MyPixelBus * strip = nullptr;

const uint16_t UDP_BROADCAST_PORT = 8827;


Melvanimate::Melvanimate(AsyncWebServer & HTTP, uint16_t pixels, uint8_t pin) : _HTTP(HTTP), _pixels(pixels), _pin(pin)
    , _settings_changed(false)
{
    setWaitFn ( std::bind (&Melvanimate::returnWaiting, this)  );        //  this callback gives bool to Effectmanager... "am i waiting..."

}


bool Melvanimate::begin(const char * name)
{
    using namespace std::placeholders;
    //using namespace RTC_manager;
    _deviceName = strdup(name);

    DebugMelvanimatef("[Melvanimate::begin] called\n");

    _HTTP.on("/data.esp", HTTP_ANY, std::bind (&Melvanimate::_handleWebRequest, this, _1));
    _HTTP.on("/site.appcache", HTTP_ANY, std::bind (&Melvanimate::_handleManifest, this, _1));
    _HTTP.rewrite("/", "/index.htm");
    _HTTP.rewrite("/index.html", "/index.htm");
    _HTTP.serveStatic("/index.htm", SPIFFS, "/index.htm").setLastModified(LAST_MODIFIED_DATE).setCacheControl("max-age=86400");
    //.setCacheControl("max-age=86400")
    _HTTP.serveStatic("/images/ajax-loader.gif", SPIFFS, "/espman/ajax-loader.gif").setLastModified(LAST_MODIFIED_DATE).setCacheControl("max-age=86400");

    //_HTTP.rewrite("/", "/index.htm");

    //_HTTP.serveStatic("/", SPIFFS, "/").setCacheControl("max-age=86400");

    //_HTTP.serveStatic("/jqColorPicker.min.js", SPIFFS, "/jqColorPicker.min.js").setCacheControl("max-age=86400");

    _loadGeneral();
    _init_LEDs();
    fillPresetArray();

    _locator.begin(_deviceName, UDP_BROADCAST_PORT);

    if (_rtcman.load()) {

        rtc_data_t data = _rtcman.get();

        if (data.on) {

            switch ( (RTC_manager::rtc_ani_state_t)data.state) {

            case RTC_manager::UNKNOWN: {
                DebugMelvanimatef("[Melvanimate::begin] RTC state unknown\n");
                Start(_defaulteffecthandle);
                break;
            };
            case RTC_manager::NONE: {
                DebugMelvanimatef("[Melvanimate::begin] RTC state NONE: Start(%u)\n", data.effect);
                Startblank(data.effect);
                if (Current()) {
                    Current()->GetRTCdata();
                }
                break;
            };
            case RTC_manager::PRESET: {
                DebugMelvanimatef("[Melvanimate::begin] RTC state PRESET: Start(%u), Load(%u)\n", data.effect, data.preset);
                Start(data.effect);
                Load(data.preset);
                break;
            };

            }

        } else {

            _toggleHandle = _findhandle(data.effect);

            if (_toggleHandle) {
                DebugMelvanimatef("[Melvanimate::begin] Saved State = OFF: toggle = %u\n", data.effect );

                if (data.state == RTC_manager::PRESET) {
                    _toggleHandle->preset(data.preset);
                }

            } else {
                DebugMelvanimatef("[Melvanimate::begin] Unable to find handler: %u\n", data.effect );

            }

            Start(_defaulteffecthandle);

        }


    } else {
        DebugMelvanimatef("[Melvanimate::begin] CRC ERROR starting default\n" );
        Start(_defaulteffecthandle);
    }



}

void Melvanimate::addJQueryhandlers()
{


    // _HTTP.on("/jquery/jqm1.4.5.css", HTTP_GET, [](AsyncWebServerRequest *request){
    //         request->redirect("http://code.jquery.com/mobile/1.4.5/jquery.mobile-1.4.5.min.css");
    // }).setFilter(ON_STA_FILTER);
    //
    // _HTTP.on("/jquery/jq1.11.1.js", HTTP_GET, [](AsyncWebServerRequest *request){
    //         request->redirect("http://code.jquery.com/jquery-1.11.1.min.js");
    // }).setFilter(ON_STA_FILTER);
    //
    // _HTTP.on("/jquery/jqm1.4.5.js", HTTP_GET, [](AsyncWebServerRequest *request){
    //         request->redirect("http://code.jquery.com/mobile/1.4.5/jquery.mobile-1.4.5.min.js");
    // }).setFilter(ON_STA_FILTER);
    //
    // _HTTP.serveStatic("/jquery", SPIFFS, "/jquery/").setCacheControl("max-age:86400").setFilter(ON_AP_FILTER);





}

void Melvanimate::loop()
{
    _checkheap();
    _process();         //  this is function from EffectManager that has to be run.

    static uint32_t tick = 0;


    if ( millis() - tick > 30) {
        if (animator) {
            if ( animator->IsAnimating() ) {
                animator->UpdateAnimations();
            }
        }
        if (strip) {
            strip->Show();
        }
        tick = millis();

    }

    //yield();

    _timer.run();
    _saveGeneral();
    _locator.loop();

    if (_mqtt) {
        _mqtt->loop();
    }

    //yield();

    if (_reInitPixelsAsync) {
        strip->ClearTo(0);
        _init_LEDs();
        _reInitPixelsAsync = false;
    }

}


void Melvanimate::_init_LEDs()
{
    if (strip) {
        strip->ClearTo(0);
        strip->Show();
        delete strip;
        strip = nullptr;
    }

    if (animator) {
        delete animator;
        animator = nullptr;
    }

    if (!_pixels) {
        //strip = new NeoPixelBus(_pixels, DEFAULT_WS2812_PIN);
        _pixels = 1;
    }

    //strip = new MyPixelBus(_pixels, DEFAULT_WS2812_PIN);
    strip = new MyPixelBus(_pixels);

    if (strip) {
        strip->Begin();
        strip->Show();
    } else {
        //Serial.println("ERROR CREATING STRIP");
    }

}



// RgbColor  Melvanimate::dim(RgbColor input, const uint8_t brightness)
// {
//  if (brightness == 0) { return RgbColor(0); }
//  if (brightness == 255) { return input; }
//  if (input.R == 0 && input.G == 0 && input.B == 0 ) { return input; }
//  HslColor originalHSL = HslColor(input);
//  originalHSL.L =  originalHSL.L   * ( float(brightness) / 255.0 ) ;
//  return RgbColor( HslColor(originalHSL.H, originalHSL.S, originalHSL.L )  );
// }


// void        Melvanimate::grid(const uint16_t x, const uint16_t y)
// {
//  if ( x * y > _pixels) { return; } // bail if grid is too big for pixels.. not sure its required
//  if (_grid_x == x && _grid_y == y) { return; } // return if unchanged
//  Start("Off");
//  _grid_x = x;
//  _grid_y = y;
//  DebugMelvanimatef("NEW grids (%u,%u)\n", _grid_x, _grid_y);
//  _settings_changed = true;
//  if (_matrix) { delete _matrix; _matrix = nullptr; }
//  _matrix = new Melvtrix(_grid_x, _grid_y, _matrixconfig);
// }

// void        Melvanimate::setmatrix(const uint8_t i)
// {
//  if (_matrixconfig == i && _matrix) { return; } //  allow for first initialisation with params = initialised state.
//  Start("Off");
//  _matrixconfig = i;
//  DebugMelvanimatef("NEW matrix Settings (%u)\n", _matrixconfig);
//  _settings_changed = true;
//  _init_matrix();
// }

void Melvanimate::setPixels(const int pixels)
{
    bool changed = false;
    JSONpackage json;

    if (!json.parseSPIFS(MELVANA_SETTINGS) || !SPIFFS.exists(MELVANA_SETTINGS)) {

        JsonObject & root = json.getRoot();

        if (root.containsKey("globals")) {
            JsonObject & globals = root["globals"];

            uint16_t saved =  globals["pixels"];
            if (saved != pixels) {
                globals["pixels"] = pixels;
                changed = true;
            }
        } else {
            JsonObject& globals = root.createNestedObject("globals");
            globals["pixels"] = pixels;
            changed = true;
        }

        if (changed) {
            File set;
            set = SPIFFS.open(MELVANA_SETTINGS, "w");
            if (set) {
                root.prettyPrintTo(set);
                set.close();
            }
            DebugMelvanimatef("NEW Pixels: %u\n", pixels);
            if (strip) {
                strip->ClearTo(0);
            }
            _pixels = pixels;
            _init_LEDs();
        }
    }
}

//  This is a callback that when set, checks to see if current animation has ended.
// set using setWaitFn ( std::bind (&Melvana::returnWaiting, this)  ); in initialisation
bool Melvanimate::returnWaiting()
{
    if (!_waiting) { return false; }

    if (animator && _waiting == 2) {

        if (!animator->IsAnimating()) {
            DebugMelvanimatef("[Melvanimate::returnWaiting] Autowait END\n");
            _waiting = false;
            return false;
        }

    }

    // if ur autowaiting but animator has been deleted...  creates a memory leak otherwise...
    if (!animator && _waiting == 2) {
        _waiting = false;
        return false;
    }

    // saftey, in case of faulty effect
    if (millis() - _waiting_timeout > EFFECT_WAIT_TIMEOUT) {
        DebugMelvanimatef("[Melvanimate::returnWaiting] Safety Timeout hit\n");
        _waiting = false;
        _waiting_timeout = 0;
        return false;
    }

    return true;

}

int Melvanimate::getTimeLeft()
{
    if (_timerState >= 0) {
        return _timer.getTimeLeft(_timerState);
    } else {
        return 0;
    }
}


void Melvanimate::autoWait()
{
    DebugMelvanimatef("[Melvanimate::autoWait] Auto wait set\n");
    _waiting_timeout = millis();
    _waiting = 2;
}

void Melvanimate::setWaiting(bool wait)
{
    if (wait) {
        DebugMelvanimatef("[Melvanimate::setWaiting] Set wait true\n");
        _waiting_timeout = millis();
        _waiting = 1;
    } else {
        DebugMelvanimatef("[Melvanimate::setWaiting] Set wait false\n");
        _waiting = 0;
        _waiting_timeout = 0;
    }
}



bool Melvanimate::_saveGeneral(bool override)
{
    //
    // JSONpackage json;
    //
    // if (!_settings_changed && !override) { return false; }
    //
    //
    // if (json.parseSPIFS(MELVANA_SETTINGS) == 0) {
    //
    //         JsonObject& root = json.getRoot();
    //
    //         File _settings;
    //
    //         DebugMelvanimatef("Saving Settings: ");
    //
    //         JsonObject& globals = root.createNestedObject("globals");
    //         {
    //                 globals["pixels"] = _pixels;
    //         }
    //
    //         _settings = SPIFFS.open(MELVANA_SETTINGS, "w+");
    //
    //         if (!_settings) { return false; }
    //
    //         if (_mqtt) {
    //                 _mqtt->addJson(globals);
    //         } else {
    //                 JsonObject& MQTTjson = root["MQTT"];
    //                 MQTTjson["enabled"] = false;
    //         }
    //
    //         _settings.seek(0, SeekSet);
    //         root.prettyPrintTo(_settings);
    //         _settings_changed = false;
    //         _settings.close();
    //         return true;
    //
    // }
}


bool Melvanimate::_loadGeneral()
{

    JSONpackage json;
    if (json.parseSPIFS(MELVANA_SETTINGS) == 0) {
        JsonObject& root = json.getRoot();

// global variables
        if (root.containsKey("globals")) {

            JsonObject& globals = root["globals"];

            if (globals.containsKey("pixels")) {
                uint16_t pixels = globals["pixels"].as<long>();
                if (pixels) {
                    _pixels = pixels;
                }
            }


            _initMQTT(globals);


        } else { DebugMelvanimatef("[Melvanimate::load] No Globals\n"); }
// current settings
        if (root.containsKey("current")) {

            JsonObject& current = root["current"];

        } else { DebugMelvanimatef("[Melvanimate::load] No Current\n"); }
// effect settings
        if (root.containsKey("effectsettings")) {
            JsonObject& effectsettings = root["effectsettings"];
        } else { DebugMelvanimatef("[Melvanimate::load] No effect settings\n"); }

        return true;

    } else {
        return false;
    }

};


bool Melvanimate::parse(JsonObject & root)
{

    bool code = false;

    if (root.containsKey("enable")) {
        String data = root["enable"];
        if ( data.equalsIgnoreCase("on")) {
            //if (_toggleHandle) {
            //  DebugMelvanimatef("[Melvanimate::load] StartBlank(_togglehandle)\n");
            //  Startblank(_toggleHandle);
            //} else {
            //DebugMelvanimatef("[Melvanimate::load] Start())\n");
            Start();
            //}

        } else if ( data.equalsIgnoreCase("off")) {
            if (_defaulteffecthandle) {
                Start(_defaulteffecthandle);
            } else {
                Start(_firstHandle);
            }
        }
        code = true;

    }


    //  depreciated.....
    if (root.containsKey("mode") ) {
        Start(root["mode"].asString());
        code = true;
    }

    if (root.containsKey("effect") ) {
        Start(root["effect"].asString());
        code = true;

    }

    rtc_data_t & data = _rtcman.get();

    if (root.containsKey("preset")) {

        //String data = root["preset"];

        const char * preset = root["preset"];
        bool isnumber = true;

        for (int i = 0; i < strlen(preset); i++) {
            if (!isdigit(preset[i])) {
                isnumber = false;
            }
        }

        if (isnumber) {
            DebugEffectManagerf("[EffectManager::parse] preset is number: %s\n", preset);
            long presetno = strtol( preset, nullptr, 10);
            if (presetno < 256 > -1) {
                code = Load( (uint8_t)presetno);
            }
        } else {
            DebugEffectManagerf("[EffectManager::parse] preset is string: %s\n", preset);
            code = Load(preset);

        }

    }

    if (Current()) {
        if (Current()->parseJson(root)) {
            code = true;
        }

    }

    if (_mqtt && *_mqtt) {

        if ( root.containsKey("effect") || root.containsKey("enable") || root.containsKey("preset") ) {
            DebugMelvanimatef("[parse] only changed false\n");
            _mqtt->sendJson(false);
        } else {
            DebugMelvanimatef("[parse] only changed true\n");
            _mqtt->sendJson(true);
        }
    }

//  save the data to RTC after user input has been parsed...

    if (Current() ) {

        if (Current() != _defaulteffecthandle) {
            DebugMelvanimatef("[Melvanimate::parse(JsonObject & root)] Saving RTC vars ON\n");
            data.effect = Current()->index;
            Current()->SaveRTCdata();
            data.on = true;

            if (Current()->preset() != 255) {
                data.state = RTC_manager::PRESET;
                data.preset = Current()->preset();
            } else {
                data.state = RTC_manager::NONE;
                data.preset = 255;
            }

        } else {
            DebugMelvanimatef("[Melvanimate::parse(JsonObject & root)] Saving RTC vars OFF\n");

            data.effect = (_toggleHandle) ? _toggleHandle->index : 0;
            data.on = false;
        }


        _rtcman.save();

    }

    if (code) {
        sendEvent("refresh", "refresh");
    }

    return code;
}



void Melvanimate::_initMQTT(JsonObject & root)
{
    //return;

    IPAddress addr;
    uint16_t port = 1883;
    DebugMelvanimatef("[Melvanimate::_initMQTT] called\n");

    // Serial.println();
    // root.prettyPrintTo(Serial);
    // Serial.println();

    if  (root.containsKey("MQTT")) {

        JsonObject& MQTTjson = root["MQTT"];

        if (MQTTjson["enabled"] == true ) {

            addr[0] = MQTTjson["ip"][0];
            addr[1] = MQTTjson["ip"][1];
            addr[2] = MQTTjson["ip"][2];
            addr[3] = MQTTjson["ip"][3];


            if (MQTTjson.containsKey("port")) {
                port = MQTTjson["port"];
            }

            if (_mqtt && (_mqtt->getIP() != addr || _mqtt->getPort() != port   )) {

                delete _mqtt;
                _mqtt = nullptr;
                _mqtt = new MelvanimateMQTT(this, addr, port);

                if (!_mqtt) {
                    DebugMelvanimatef("[Melvanimate::_initMQTT] FAILED\n");
                }

            } else if (_mqtt) {
                DebugMelvanimatef("[Melvanimate::_initMQTT] No changes Required\n");
                if (!*_mqtt) {
                    DebugMelvanimatef("[Melvanimate::_initMQTT] But it is NOT connected\n");
                }
            } else if (!_mqtt) {
                _mqtt = new MelvanimateMQTT(this, addr, port);
            }


            DebugMelvanimatef("[Melvanimate::_initMQTT] (%u,%u,%u,%u) : %u \n", addr[0], addr[1], addr[2], addr[3], port );


            uint32_t timeout = millis();

            //  waits for connection to allow subscribe to work.


            if (_mqtt) {
                //  std::bind(&ESPmanager::_HandleSketchUpdate, this, _1 ))


                _mqtt->onConnect( std::bind( &Melvanimate::_MQTTsubscribe, this) );

                _mqtt->connect();


            }



            //if (MQTTjson.containsKey("topics")) {

            //   JsonArray& topics = MQTTjson["topics"];
            //
            //     for (JsonArray::iterator it = topics.begin(); it != topics.end(); ++it) {
            //
            //       if (_mqtt) {
            //         const char * currenttopic = *it;
            //         DebugMelvanimatef("[Melvanimate::_initMQTT] subscribing to : %s\n", currenttopic);
            //           _mqtt->subscribe(currenttopic,2);
            //       }
            //
            //     }
            //
            //
            // }

            // if (_mqtt && *_mqtt) {
            //  DebugMelvanimatef("[Melvanimate::_initMQTT] Sending Full initial config\n");
            //  _mqtt->sendJson(false);
            // }


        } else {
            DebugMelvanimatef("[Melvanimate::_initMQTT] Disabling MQTT\n" );

            if (_mqtt) {
                delete _mqtt;
                _mqtt = nullptr;
            }
        }

        _settings_changed = true;

    }
    //}

}

void Melvanimate::_MQTTsubscribe()
{

    DebugMelvanimatef("[Melvanimate::_MQTTsubscribe] \n");

    JSONpackage json;

    int result = json.parseSPIFS(MELVANA_SETTINGS);

    if (!result) {

        JsonObject& root = json.getRoot();

        if (root.containsKey("globals") && root["globals"].asObject().containsKey("MQTT") && root["globals"]["MQTT"].asObject().containsKey("topics") ) {

            JsonArray & topics = root["globals"]["MQTT"]["topics"];

            for (JsonArray::iterator it = topics.begin(); it != topics.end(); ++it) {

                if (_mqtt) {
                    const char * currenttopic = *it;
                    DebugMelvanimatef("[Melvanimate::_MQTTsubscribe] subscribing to : %s\n", currenttopic);
                    _mqtt->subscribe( currenttopic, 2);
                }

            }

        }
    } else {
        DebugMelvanimatef("[Melvanimate::_MQTTsubscribe] JSON parse ERROR %i\n", result);
    }


}

bool Melvanimate::setTimer(int timeout, String command, String option)
{
    // delete current timer if set
    if (_timerState != -1) {
        _timer.deleteTimer(_timerState);
        _timerState = -1;
        DebugMelvanimatef("[Melvanimate::setTimer] Timer Cancelled\n");
    }

    timeout *= (1000 * 60);         // convert timout to milliseconds from minutes...

    // set new timer if there is an interval
    if (timeout) {

        _timerState = _timer.setTimeout(timeout, [command, option, this]() {
            DynamicJsonBuffer jsonBuffer;
            JsonObject& root = jsonBuffer.createObject();

            if (command.equalsIgnoreCase("off")) {
                Start("Off");
            } else if (command.equalsIgnoreCase("start")) {
                Start(option);
            } else if (command.equalsIgnoreCase("brightness")) {
                if (Current()) {
                    root["brightness"] = option.toInt();
                    Current()->parseJson(root);
                }
            } else if (command.equalsIgnoreCase("speed")) {
                if (Current()) {
                    root["speed"] = option.toInt();
                    Current()->parseJson(root);
                }
            } else if (command.equalsIgnoreCase("loadpreset")) {
                DebugMelvanimatef("[Melvanimate::setTimer] Load preset: %u\n", option.toInt());
                Load(option.toInt());
            }
            _timerState = -1;                         //  get ride of flag to timer!
        });

        if (_timerState > -1 ) { DebugMelvanimatef("[Melvanimate::setTimer] Started (%s,%s)\n", command.c_str(), option.c_str()); }

    } else {
        DebugMelvanimatef("[Melvanimate::setTimer] No Timeout, so timer cancelled\n");
    }

}

void Melvanimate::populateJson(JsonObject & root, bool onlychanged)
{
    JsonObject& settings = root.createNestedObject("settings");

    if (!onlychanged) {
        if (_deviceName) {
            root["device"] = _deviceName;
        }

        root["heap"] = _heap;
        root["power"] = String(getPower());
        root["founddevices"] = _locator.count();

        /*
              Home page
         */

        //if (page == "homepage" || page == "palette" || page == "all") {
        JsonArray& modes = root.createNestedArray("modes");
        //Serial.printf("Total effects: %u\n", total());
        for (uint8_t i = 0; i < total(); i++) {
            modes.add(getName(i));
        }


        // creates settings node for web page
        // adds minimum current effect name, if there if addJson returns false.
        if (Current()) {

            settings["currentpreset"] = Current()->preset();

            if (!Current()->addJson(settings)) {
                settings["effect"] = Current()->name();
            }

            if (!settings.containsKey("effect")) {
                settings["effect"] = Current()->name();
            }


            addCurrentPresets(root);


            //  }

            //  this is needed as the matrix settings is simple x, y, uin8_t... not all the required settings for the gui...
            if (expandMatrixConfigToJson(settings)) {
                //DebugMelvanimatef("[Melvanimate::_sendData] matrix json expanded!\n");
            }
        }
    } else {

        DebugMelvanimatef("[Melvanimate::populateJson] Adding ONLY changed variables\n");

        Current()->addJson(settings, onlychanged);
        expandMatrixConfigToJson(settings);

    }


}

void Melvanimate::_sendData(String page, int8_t code, AsyncWebServerRequest *request)
{
    //JsonObject test;
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();

    root["code"] = code;
    JSONpackage json; // = *jsonpackage;

    //JSONpackage * jsonpackage = nullptr;
    /*

       (page == "homepage") might be a breaking change for the GUI...

     */
    if (page == "homepage" || page == "all" ) {
        populateJson(root);
    }



    DebugMelvanimatef("[Melvanimate::_sendData] page = %s\n", page.c_str());

    if (page == "configpage" || page == "all") {

        root["pixels"] = getPixels();
        // jsonpackage = new JSONpackage;
        // if (!jsonpackage) return;
        bool mqtt_settings_ok = false;
        JsonObject * settings = nullptr;

        if (!json.parseSPIFS(MELVANA_SETTINGS)) {
            settings =  &json.getRoot().asObject();
            if (settings && settings->containsKey("globals") && (*settings)["globals"].asObject().containsKey("MQTT")) {
                mqtt_settings_ok = true;
            } else {

                if (!settings) {
                    DebugMelvanimatef("settings = null\n");
                } else {
                    DebugMelvanimatef("settings = ");
#ifdef DebugMelvanimate
                    settings->prettyPrintTo(Serial);
#endif
                    DebugMelvanimatef("\n");
                }
                //if (!settings->containsKey("MQTT")) { Serial.println("settings->containsKey(\"MQTT\") = null"); }
            }
        }

        if (mqtt_settings_ok) {
            //if (_mqtt && mqtt_settings_ok)
            //_mqtt->addJson(root);
            //JsonObject & settings = json.getRoot();
            JsonObject & destmqtt = root.createNestedObject("MQTT");
            JsonObject & mqtt = (*settings)["globals"]["MQTT"].asObject();
            JSONpackage::mergejson(destmqtt, mqtt);
        } else {
            JsonObject & mqtt = root.createNestedObject("MQTT");
            mqtt["enabled"] = false;
        }

    }

    if (page == "devicelist" || page == "all" || page == "homepage") {
        root["founddevices"] = _locator.count();
        JsonArray & devicelist = root.createNestedArray("devices");

        _locator.addJson(devicelist);
    }

    if (page == "timer" || page == "all" || page == "homepage") {

        JsonObject& timerobj = root.createNestedObject("timer");
        timerobj["running"] = (getTimeLeft() > 0) ? true : false;
        if (getTimeLeft() > 0) {
            JsonArray& remaining = timerobj.createNestedArray("remaining");
            int minutes = getTimeLeft() / ( 1000 * 60);
            int seconds = getTimeLeft() / 1000;
            seconds %= 60;
            remaining.add(minutes);
            remaining.add(seconds);
        }
        // only add them all for the actual timer page...
        if (page == "timer") {
            addAllpresets(root);
        }
    }

    if (page == "presetspage" || page == "all" || page == "allpresetspage" ) {

        JsonObject& settings = root.createNestedObject("settings");
        // adds minimum current effect name, if there if addJson returns false.
        if (Current()) {

            settings["currentpreset"] = Current()->preset();
            settings["currentpresetname"] = Current()->name();
            addCurrentPresets(root);
        }

        addAllpresets(root);
    }

    if (page == "palette") {

        populateJson(root);
    }

    if (page == "eqpage" || page == "equdpsendpage") {

        populateJson(root);

    }

#ifdef DebugMelvanimate
    Serial.println("JSON REPLY");
    root.prettyPrintTo(Serial);
    Serial.println();
#endif 

    _sendJsontoHTTP(root, request);

    // if (jsonpackage) {
    //         delete jsonpackage;
    // }
    DebugMelvanimatef("[Melvanimate::_sendData] jsonBuffer Size = %u, heap = %u\n", jsonBuffer.size(), ESP.getFreeHeap() );

}

template <class T> void Melvanimate::_sendJsontoHTTP( const T & root, AsyncWebServerRequest *request)
{


    AsyncResponseStream *response = request->beginResponseStream("text/json");
    response->addHeader("Access-Control-Allow-Origin", "*");
    response->addHeader("Cache-Control", "no-store");
    root.printTo(*response);
    request->send(response);


}


void Melvanimate::_handleManifest(AsyncWebServerRequest *request)
{
#ifdef DISABLE_MANIFEST
    request->send(404);
    return;
#endif

    AsyncResponseStream *response = request->beginResponseStream(F("text/cache-manifest")); //Sends 404 File Not Found
    response->addHeader(F("Cache-Control"), F( "must-revalidate"));
    response->print(F("CACHE MANIFEST\n"));
    response->printf( "# %s\n", __DATE__ " " __TIME__ );
#ifdef RANDOM_MANIFEST
    response->printf("# %u\n", random(10000));
#endif
    response->print(F("CACHE:\n"));
    // response->print(F("jquery/jqm1.4.5.css\n"));
    // response->print(F("jquery/jq1.11.1.js\n"));
    // response->print(F("jquery/jqm1.4.5.js\n"));
    // response->print(F("jqColorPicker.min.js\n"));
    response->print(F("images/ajax-loader.gif\n"));
    response->print(F("index.htm\n"));
    response->print(F("NETWORK:\n"));
    response->print("*\n");
    request->send(response);
}

void Melvanimate::_handleWebRequest(AsyncWebServerRequest *request)
{
    uint32_t start_time = millis();
    String page = "homepage";
    int8_t code = -1;

    //  this fires back an OK, but ignores the request if all the args are the same.  uses MD5.
//  if (_check_duplicate_req()) { _HTTP.setContentLength(0); _HTTP.send(200); return; }

    DebugMelvanimatef("[Melvanimate::_handleWebRequest] \n");

#ifdef DebugMelvanimate
//List all collected headers
    int params = request->params();
    int i;
    for (i = 0; i < params; i++) {
        AsyncWebParameter* h = request->getParam(i);
        DebugMelvanimatef("[Melvanimate::_handleWebRequest] [%s]: %s\n", h->name().c_str(), h->value().c_str());
    }
#endif

    DebugMelvanimatef("[Melvanimate::_handleWebRequest] Heap = [%u]\n", ESP.getFreeHeap());

    //   set the page to the requested page
    if (request->hasParam("data", true) ) {
        page = request->getParam("data", true)->value();
        DebugMelvanimatef("[Melvanimate::_handleWebRequest] Page Requested = %s\n", page.c_str());

    }

    if (request->hasParam("rtc")) {
        //Serial.println("Getting RTC Data");
        if (_currentHandle) {
            _currentHandle->GetRTCdata();
        }
    }

    // puts all the args into json...
    // might be better to send pallette by json instead..

    DynamicJsonBuffer jsonBuffer;
    JsonObject * p_root;         // = & jsonBuffer.createObject();


    //  This serialises all the request... except when it is a json...
    bool jsonparsed = false;

    if (request->params() == 1) {

        DebugMelvanimatef("[Melvanimate::_handleWebRequest] Parsing Json Request: %s\n", request->getParam(0)->value().c_str() );
        p_root = &jsonBuffer.parseObject(request->getParam(0)->value());
        if (p_root) {
            JsonObject & root = *p_root;
            if (root.success()) {
                DebugMelvanimatef("[Melvanimate::_handleWebRequest] Parse success\n");
                jsonparsed = true;
            } else {
                DebugMelvanimatef("[Melvanimate::_handleWebRequest] Parse FAIL\n");
            }
        }
    }

    if (!jsonparsed) {

        p_root = &jsonBuffer.createObject();
        JsonObject & root = *p_root;

        for (uint8_t i = 0; i < request->params(); i++) {
            AsyncWebParameter* h = request->getParam(i);

            root[ h->name() ] = h->value();
        }
    }

    // if (!request->hasParam("json")) {

    //  p_root = & jsonBuffer.createObject();
    //  JsonObject & root = *p_root;

    //  for (uint8_t i = 0; i < request->getParams(); i++) {
    //   root[request->getParamName(i)] = request->getParam(i);
    //  }

    // } else {
    //  p_root = & jsonBuffer.parseObject(request->getParam("json"));
    // }



    if (request->hasParam("nopixels", true) && (request->getParam("nopixels", true)->value()).length() != 0) {

        int num = (request->getParam("nopixels", true )->value()).toInt();

        //setTimeout(long d, timer_callback f);

        //if (num != _pixels) {
        //DebugMelvanimatef("[Melvanimate::_handleWebRequest] Pixels Count Changed... Changing Pixels\n");

//  pixelstochange
        _timer.setTimeout(1000, [num, this]() {
            setPixels(num);
        });
        //}

        page = "layout";


        //  also submits mqtt data
        /*
           [ARG:0] nopixels = 50
           [ARG:1] enablemqtt = off
           [ARG:2] mqtt_ip = 1.2.3.4
           [ARG:3] mqtt_port = 123

         */
    }






    if (request->hasParam("enablemqtt", true) || request->hasParam("add_topic", true) || request->hasParam("remove_topic", true) ) {

        DebugMelvanimatef("[Melvanimate::_handleWebRequest] MQTT\n");

        bool mqtt_settings_changed = false;
        page = "configpage";

        JSONpackage * json = new JSONpackage;

        if (json) {

            if (!json->parseSPIFS(MELVANA_SETTINGS) || !SPIFFS.exists(MELVANA_SETTINGS)) {

                DebugMelvanimatef("[Melvanimate::_handleWebRequest] Settings loaded\n");

                JsonObject & rootsettings = json->getRoot();

                bool mqttenabled = false;
                IPAddress mqttip;
                uint16_t mqttport;
                JsonObject * globals = nullptr;
                JsonObject * MQTT = nullptr;
                JsonArray * topics = nullptr;


                if (rootsettings.containsKey("globals") && rootsettings["globals"].asObject().containsKey("MQTT")) {
                    DebugMelvanimatef("[Melvanimate::_handleWebRequest] loaded settings contain globals & MQTT\n");
                    globals = &rootsettings["globals"].asObject();
                    MQTT = &rootsettings["globals"]["MQTT"].asObject();

                    mqttenabled = rootsettings["globals"]["MQTT"]["enabled"];
                    JsonArray & ip = rootsettings["globals"]["MQTT"]["ip"].asArray();
                    mqttip = IPAddress( ip[0], ip[1], ip[2], ip[3]);
                    if (rootsettings["globals"]["MQTT"].asObject().containsKey("topics")) {
                        topics = &rootsettings["globals"]["MQTT"]["topics"].asArray();
                    }


                }

                if (!rootsettings.containsKey("globals")) {
                    DebugMelvanimatef("[Melvanimate::_handleWebRequest] Creating Globals json object\n");
                    globals = &rootsettings.createNestedObject("globals");
                    mqtt_settings_changed = true;
                } else {
                    globals = &rootsettings["globals"].asObject();
                }

                if (globals && !globals->containsKey("MQTT")) {
                    DebugMelvanimatef("[Melvanimate::_handleWebRequest] Creating MQTT json object\n");
                    MQTT = &globals->createNestedObject("MQTT");
                    mqtt_settings_changed = true;
                }

                // JsonObject & settings = rootsettings.createNestedObject("globals");
                // JsonObject & mqttjson = settings.createNestedObject("MQTT");
//  enabled
                if (request->hasParam("enablemqtt", true )) {
                    bool value = (  request->getParam("enablemqtt", true )->value() == "on") ? true : false;
                    DebugMelvanimatef("[Melvanimate::_handleWebRequest] Enable MQTT: %s \n", (value) ? "true" : "false");
                    if (MQTT && value != mqttenabled) {
                        (*MQTT)["enabled"] = value;
                        mqtt_settings_changed = true;
                    } else {
                        DebugMelvanimatef("    unchanged\n");
                    }

                }
// MQTT IP

                if (request->hasParam("mqtt_ip", true )) {
                    IPAddress ip;
                    if (ip.fromString( request->getParam("mqtt_ip", true )->value())) {
                        DebugMelvanimatef("[Melvanimate::_handleWebRequest] MQTT IP: %u.%u.%u.%u \n", ip[0], ip[1], ip[2], ip[3] );

                        if (MQTT && ip != mqttip) {
                            JsonArray & iparray = MQTT->createNestedArray("ip");
                            iparray.add(ip[0]);
                            iparray.add(ip[1]);
                            iparray.add(ip[2]);
                            iparray.add(ip[3]);
                            mqtt_settings_changed = true;

                        } else {
                            DebugMelvanimatef("    unchanged\n");
                        }

                    }
                }

// port
                if (request->hasParam("mqtt_port", true)) {
                    uint16_t port = request->getParam("mqtt_port", true)->value().toInt();
                    DebugMelvanimatef("[Melvanimate::_handleWebRequest] MQTT port: %u \n", port );

                    if (MQTT && port != mqttport) {
                        (*MQTT)["port"] = port;
                        mqtt_settings_changed = true;
                    } else {
                        DebugMelvanimatef("    unchanged\n");
                    }

                }


//  topics
//  need to check if it is a topic already...

                if (request->hasParam("add_topic", true)) {

                    String topic = request->getParam("add_topic", true)->value();

                    if (topic.length() > 0) {

                        bool exists = false;

                        if (topics) {
                            for (JsonArray::iterator it = topics->begin(); it != topics->end(); ++it) {
                                String current_topic = *it;
                                if (current_topic == topic) {
                                    exists = true;
                                    break;
                                }
                            }
                        }

                        DebugMelvanimatef("[Melvanimate::_handleWebRequest] add_topic %s \n", topic.c_str() );

                        if (!exists && MQTT && !topics) {
                            topics = &MQTT->createNestedArray("topics");
                        }

                        if (topics) {
                            topics->add(topic);
                            mqtt_settings_changed = true;
                        }


                    }
                }


                if (topics && request->hasParam("remove_topic", true)) {

                    String topic_to_remove = request->getParam("remove_topic", true)->value().c_str();

                    if (topics) {
                        uint16_t index = 0;
                        for (JsonArray::iterator it = topics->begin(); it != topics->end(); ++it) {
                            String current_topic = *it;
                            if (current_topic == topic_to_remove) {
                                mqtt_settings_changed = true;
                                topics->removeAt(index);
                                DebugMelvanimatef("[Melvanimate::_handleWebRequest] remove_topic %s at index % u \n", current_topic.c_str(), index );
                            }
                            index++;
                        }



                    }


                }

// save changes

                if (mqtt_settings_changed) {

                    File _settings;

                    DebugMelvanimatef("Saving Settings: ");

                    _settings = SPIFFS.open(MELVANA_SETTINGS, "w");

                    if (_settings) {

                        // if (_mqtt) {
                        //         _mqtt->addJson(globals);
                        // } else {
                        //         JsonObject& MQTTjson = root["MQTT"];
                        //         MQTTjson["enabled"] = false;
                        // }

                        //_settings.seek(0, SeekSet);
                        rootsettings.prettyPrintTo(_settings);
                        //_settings_changed = false;
                        _settings.close();
                        DebugMelvanimatef("Done\n");

                        // rootsettings.prettyPrintTo(Serial);
                        // Serial.println();
                        //return true;



                    }

                }
            }

            delete json;
        }



//      if ( request->getParam("enablemqtt") == "on" ) {

        //DebugMelvanimatef("[Melvanimate::_handleWebRequest] Enable MQTT..\n");







        // if (request->hasParam("mqtt_port", true)) {
        //         mqttjson["port"] = request->getParam("mqtt_port", true)->value();
        // }

// #ifdef DebugMelvanimate
//                         // Serial.println();
//                         // mqttjson.prettyPrintTo(Serial);
//                         // Serial.println();
// #endif

        //_initMQTT(settings);

    }

    if (p_root) {

        JsonObject & root = *p_root;


        if (request->hasParam("palette", true)) {
            //palette().mode(request->getParam("palette").c_str());
            page = "palette";                         //  this line might not be needed... palette details are now handled entirely by the effect for which they belong

            /*
               [ARG:0] palette = complementary
               [ARG:1] palette-random = timebased
               [ARG:2] palette-spread =
               [ARG:3] palette-delay =

               palette["mode"] = (uint8_t)_mode;
               palette["total"] = _total;
               palette["available"] = _available;
               palette["randmode"] = (uint8_t)_random;
               palette["range"] = _range;
               palette["delay"] = _delay;
             */


            //  this is a bit of a bodge...  Capital P for object with all parameters...
            JsonObject & palettenode = root.createNestedObject("Palette");

            palettenode["mode"] = (uint8_t)(request->getParam("palette", true)->value().toInt());


            if (request->hasParam("palette-random", true)) {
                palettenode["randmode"] = (uint8_t)Palette::randommodeStringtoEnum(request->getParam("palette-random", true)->value().c_str());
            }

            if (request->hasParam("palette-spread", true)) {
                palettenode["range"] = request->getParam("palette-spread", true)->value();

            }

            if (request->hasParam("palette-delay", true )) {
                palettenode["delay"] = request->getParam("palette-delay", true)->value();

            }
            // Serial.println("[handle_data] JSON dump");
            // root.prettyPrintTo(Serial);
            // Serial.println();

        }


        if (request->hasParam("eqmode", true) || request->hasParam("eq_send_udp", true)) {

            DebugMelvanimatef("[Melvanimate::_handleWebRequest] has enableeq\n");

            JsonObject& EQjson = root.createNestedObject("EQ");

            if (request->hasParam("eqmode", true)) {
                EQjson["eqmode"] = request->getParam("eqmode", true)->value().toInt();
            }
            //EQjson["resetpin"] = _resetPin;
            //EQjson["strobepin"] = _strobePin;
            if (request->hasParam("peakfactor", true)) {
                EQjson["peakfactor"] =  request->getParam("peakfactor", true)->value().toFloat();
            }
            if (request->hasParam("beatskiptime", true)) {
                EQjson["beatskiptime"] = request->getParam("beatskiptime", true)->value().toInt();
            }
            if (request->hasParam("samples", true)) {
                EQjson["samples"] = request->getParam("samples", true)->value().toInt();
            }
            if (request->hasParam("sampletime", true)) {
                EQjson["sampletime"] = request->getParam("sampletime", true)->value().toInt();
            }
            if (request->hasParam("eq_send_udp", true)) {
                EQjson["eq_send_udp"] = (request->getParam("eq_send_udp", true)->value() == "on") ? true : false;
            }
            if (request->hasParam("eq_addr", true)) {
                EQjson["eq_addr"] = request->getParam("eq_addr", true)->value();
            }
            if (request->hasParam("eq_port", true)) {
                EQjson["eq_port"] = request->getParam("eq_port", true)->value().toInt();
            }

        }


// matrixmode stuff
// #define NEO_MATRIX_TOP         0x00 // Pixel 0 is at top of matrix
// #define NEO_MATRIX_BOTTOM      0x01 // Pixel 0 is at bottom of matrix
// #define NEO_MATRIX_LEFT        0x00 // Pixel 0 is at left of matrix
// #define NEO_MATRIX_RIGHT       0x02 // Pixel 0 is at right of matrix
// #define NEO_MATRIX_CORNER      0x03 // Bitmask for pixel 0 matrix corner
// #define NEO_MATRIX_ROWS        0x00 // Matrix is row major (horizontal)
// #define NEO_MATRIX_COLUMNS     0x04 // Matrix is column major (vertical)
// #define NEO_MATRIX_AXIS        0x04 // Bitmask for row/column layout
// #define NEO_MATRIX_PROGRESSIVE 0x00 // Same pixel order across each line
// #define NEO_MATRIX_ZIGZAG      0x08 // Pixel order reverses between lines
// #define NEO_MATRIX_SEQUENCE    0x08 // Bitmask for pixel line order

// #define NEO_TILE_TOP           0x00 // First tile is at top of matrix
// #define NEO_TILE_BOTTOM        0x10 // First tile is at bottom of matrix
// #define NEO_TILE_LEFT          0x00 // First tile is at left of matrix
// #define NEO_TILE_RIGHT         0x20 // First tile is at right of matrix
// #define NEO_TILE_CORNER        0x30 // Bitmask for first tile corner
// #define NEO_TILE_ROWS          0x00 // Tiles ordered in rows
// #define NEO_TILE_COLUMNS       0x40 // Tiles ordered in columns
// #define NEO_TILE_AXIS          0x40 // Bitmask for tile H/V orientation
// #define NEO_TILE_PROGRESSIVE   0x00 // Same tile order across each line
// #define NEO_TILE_ZIGZAG        0x80 // Tile order reverses between lines
// #define NEO_TILE_SEQUENCE      0x80 // Bitmask for tile line order

        if (request->hasParam("matrixmode", true)) {

            page = "layout";

            JsonObject & matrixnode = root.createNestedObject("Matrix");


            if (request->hasParam("grid_x", true) && request->hasParam("grid_y", true)) {

                matrixnode["x"] = request->getParam("grid_x", true)->value().toInt();
                matrixnode["y"] = request->getParam("grid_y", true)->value().toInt();

                if (request->hasParam("matrixmode", true)) {
                    uint8_t matrixvar = 0;


                    matrixnode["multiple"] = (request->getParam("matrixmode", true)->value() == "singlematrix") ? false : true;

                    if (request->getParam("firstpixel", true)->value() == "topleft") { matrixvar += NEO_MATRIX_TOP + NEO_MATRIX_LEFT; }
                    if (request->getParam("firstpixel", true)->value() == "topright") { matrixvar += NEO_MATRIX_TOP + NEO_MATRIX_RIGHT; }
                    if (request->getParam("firstpixel", true)->value() == "bottomleft") { matrixvar += NEO_MATRIX_BOTTOM + NEO_MATRIX_LEFT; }
                    if (request->getParam("firstpixel", true)->value() == "bottomright") { matrixvar += NEO_MATRIX_BOTTOM + NEO_MATRIX_RIGHT; }

                    if (request->getParam("axis", true)->value() == "rowmajor") { matrixvar += NEO_MATRIX_ROWS; }
                    if (request->getParam("axis", true)->value() == "columnmajor") { matrixvar += NEO_MATRIX_COLUMNS; }

                    if (request->getParam("sequence", true)->value() == "progressive") { matrixvar += NEO_MATRIX_PROGRESSIVE; }
                    if (request->getParam("sequence", true)->value() == "zigzag") { matrixvar += NEO_MATRIX_ZIGZAG; }

                    if (request->getParam("matrixmode", true)->value() == "multiplematrix") {
                        if (request->getParam("multimatrixtile", true)->value() == "topleft") { matrixvar += NEO_TILE_TOP + NEO_TILE_LEFT; }
                        if (request->getParam("multimatrixtile", true)->value() == "topright") { matrixvar += NEO_TILE_TOP + NEO_TILE_RIGHT; }
                        if (request->getParam("multimatrixtile", true)->value() == "bottomleft") { matrixvar += NEO_TILE_BOTTOM + NEO_TILE_LEFT; }
                        if (request->getParam("multimatrixtile", true)->value() == "bottomright") { matrixvar += NEO_TILE_BOTTOM + NEO_TILE_RIGHT; }
                        if (request->getParam("multimatrixaxis", true)->value() == "rowmajor") { matrixvar += NEO_TILE_ROWS; }
                        if (request->getParam("multimatrixaxis", true)->value() == "columnmajor") { matrixvar += NEO_TILE_COLUMNS; }
                        if (request->getParam("multimatrixseq", true)->value() == "progressive") { matrixvar += NEO_TILE_PROGRESSIVE; }
                        if (request->getParam("multimatrixseq", true)->value() == "zigzag") { matrixvar += NEO_TILE_ZIGZAG; }
                    }

                    matrixnode["config"] = matrixvar;

                    //Serial.println("[Melvanimate::_handleWebRequest] matrixnode dump");
                    //matrixnode.prettyPrintTo(Serial);
                    //Serial.println();
                }
            }

        }

        //root.prettyPrintTo(Serial);

        code = parse(root);


        if (request->hasParam("flashfirst", true)) {

            _timer.setTimeout(30, [this]() {
                Start("Off");
                Stop();
                strip->ClearTo(0);
                strip->SetPixelColor(0, RgbColor(255, 0, 0));

            });

            page = "layout";

            // AnimUpdateCallback animUpdate = [] (float progress) {
            //  strip->SetPixelColor(0, Palette::wheel( (uint8_t)(progress * 255) ));
            //  if (progress == 1.0) { strip->SetPixelColor(0, 0); }
            // };

//   StartAnimation(0, 5000 , animUpdate);



        }

        if (request->hasParam("revealorder", true)) {

            _timer.setTimeout(30, [this]() {
                Start("Off");
                Stop();
                strip->ClearTo(0);


                for (uint16_t pixel = 0; pixel < strip->PixelCount(); pixel++) {

                    strip->SetPixelColor(pixel, RgbColor(255, 0, 0));

                    if (pixel) {
                        strip->SetPixelColor(pixel - 1, RgbColor(0, 0, 0));
                    }
                }

            });
            page = "layout";


        }


        if (request->hasParam("data", true)) {
            _sendData(request->getParam("data", true)->value(), 0, request);                         // sends JSON data for whatever page is currently being viewed
            return;
        }

        if (request->hasParam("enabletimer", true)) {
            page = "timer";
            if (request->getParam("enabletimer", true)->value() == "on") {

                if (request->hasParam("timer", true) && request->hasParam("timercommand", true)) {

                    String effect =  (request->hasParam("timeroption", true)) ? request->getParam("timeroption", true)->value() : String();

                    if (setTimer(request->getParam("timer", true)->value().toInt(), request->getParam("timercommand", true)->value(), effect )) {
                        DebugMelvanimatef("[handle] Timer command accepted\n");
                    }
                }
            } else if (request->getParam("enabletimer", true)->value() == "off") {
                setTimer(0, "off");
            }

        }


        if (request->hasParam("presetcommand", true)) {

            //String in = request->getParam("selectedeffect").toInt()
            //uint8_t File = in.substring(0, in.indexOf(".")).toInt();
            //uint8_t ID = in.substring(in.indexOf(".") + 1, in.length()).toInt();


            if (request->getParam("presetcommand", true)->value() == "load") {
                code = Load(request->getParam("selectedeffect", true)->value().toInt());
            } else if (request->getParam("presetcommand", true)->value() == "new" ) {
                code = Save(0, request->getParam("presetsavename", true)->value().c_str());
            } else if (request->getParam("presetcommand", true)->value() == "overwrite" ) {
                code = Save(request->getParam("selectedeffect", true)->value().toInt(), request->getParam("presetsavename", true)->value().c_str(), true);
            } else if (request->getParam("presetcommand", true)->value() == "delete" ) {
                code = removePreset(request->getParam("selectedeffect", true)->value().toInt());
            } else if (request->getParam("presetcommand", true)->value() == "deleteall" ) {
                removeAllpresets();
            }

        }
    }         // end of if(p_root)

    sendEvent("refresh", "command");


    _sendData(page, code, request);

    DebugMelvanimatef("[handle] time %u: [Heap] %u\n", millis() - start_time, ESP.getFreeHeap());
    return;

}


// fixed...
//  this is required as some
// bool Melvanimate::_check_duplicate_req()
// {
//  static uint32_t last_time = 0;
//  static char last_request[16] = {0};
//  if (request->hasParam("data")) { return false; }

//  MD5Builder md5;
//  md5.begin();

//  for (uint8_t args = 0; args < request->getParams(); args++) {
//   String req = request->getParamName(args) + request->getParam(args);
//   md5.add(req);
//  }

//  md5.calculate();
//  bool match = false;
//  //Serial.printf("[MD5] %s\n", md5.toString().c_str());
//  char this_request[16] = {0};
//  md5.getChars(this_request);

//  if (memcmp(last_request, this_request, 16) == 0) {
//   match = true;
//   DebugMelvanimatef("Request ignored: duplicate");
//  }

//  memcpy(last_request, this_request, 16);

//  bool time_elapsed = (millis() - last_time > 10000) ? true : false;
//  last_time = millis();

//  return match & !time_elapsed;

// }

uint32_t Melvanimate::getPower()
{
    uint32_t total = 0;
    int brightnesstally = 0;


    if (millis() - _powertick < 500) {
        return _power;
    }

    if (strip) {


        for (int i = 0; i < strip->PixelCount(); i++) {
            RgbColor colour = myPixelColor(strip->GetPixelColor(i));
            int brightness = colour.CalculateBrightness();
            brightness = map(brightness, 0, 255, 0, 60);
            brightnesstally = brightnesstally + brightness;
        }







        // for (uint16_t pixel = 0; pixel < strip->PixelCount(); pixel++)
        // {
        //  RgbColor color =  strip->GetPixelColor(pixel) ;

        //  total += (color.R + color.G + color.B) / 765;

        //  //total +=  strip->GetPixelColor(pixel).CalculateBrightness();

        // }

        //total = total / strip->PixelCount();

    }

    _power = brightnesstally;
    _powertick = millis();

    return brightnesstally;

}

bool Melvanimate::createAnimator()
{
    if (strip) {
        return createAnimator(strip->PixelCount());
    }

    return false;
}


bool Melvanimate::createAnimator(uint16_t count)
{

    if (animator) {

        if (animator->IsAnimating()) {
            //  Serial.printf("[Melvanimate::createAnimator] animator->IsAnimating() = true \n");
        }
        delete animator;
        animator = nullptr;
    }


    if (count < MAX_NUMBER_OF_ANIMATIONS ) {
        animator = new NeoPixelAnimator(strip->PixelCount());
    }

    if (animator) {
        return true;
    } else {
        return false;
    }

}

void Melvanimate::deleteAnimator()
{
    if (animator) {
        delete animator;
        animator = nullptr;
    }
}

void Melvanimate::_checkheap()
{

    uint32_t newTest = ESP.getFreeHeap();
    if (newTest  != _heap) {
        _heap = newTest;
    }

}

void Melvanimate::sendEvent(const char * msg, const char * topic)
{
    if (_events) {
        _events->send(msg, topic);
    }
}

void Melvanimate::_saveState()
{


}

void Melvanimate::_getState()
{

}
