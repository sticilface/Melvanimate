#include "Melvanimate.h"

#include <MD5Builder.h>
#include <ArduinoJson.h>
#include <ESP8266mDNS.h>

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

        _deviceName = name;

        DebugMelvanimatef("Begin Melvana called\n");

        _HTTP.on("/data.esp", HTTP_ANY, std::bind (&Melvanimate::_handleWebRequest, this, _1));

        _HTTP.serveStatic("", SPIFFS, "/", "max-age=86400");

        _loadGeneral();
        _init_LEDs();
        fillPresetArray();

        MDNS.addService("melvanimate", "tcp", 8270);

        _locator.begin(_deviceName, 8827);

}

void Melvanimate::addJQueryhandlers() {
        _HTTP.serveStatic("/jquery/", SPIFFS, "/jquery/", "max-age=86400");
}

void Melvanimate::loop()
{

        _process();         //  this is function from EffectManager that has to be run.
        _timer.run();
        _saveGeneral();

        yield();

        _locator.loop();

        if (_mqtt) {
                _mqtt->loop();
        }

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

        yield();

        if (_reInitPixelsAsync) {
                strip->ClearTo(0);
                _init_LEDs();
                _reInitPixelsAsync = false;
        }


// //#ifdef DebugMelvanimate
//         {
//                 static uint32_t mdns_search_timeout = 0;
//
//                 if (millis() - mdns_search_timeout > 10000) {
//                         mdns_search_timeout = millis();
//
//                         int number = MDNS.queryService("melvanimate", "tcp");
//
//                         if (number > 0) {
//
//                                 for (uint8_t result = 0; result < number; result++) {
//                                         int port = MDNS.port(result);
//                                         String host = MDNS.hostname(result);
//                                         IPAddress IP = MDNS.IP(result);
//                                         DebugMelvanimatef("Service Found [%u] %s (%u.%u.%u.%u) port = %u\n", result, host.c_str(), IP[0], IP[1], IP[2], IP[3], port);
//                                 }
//                         }
//                 }
//         }
// //#endif
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

        if (_pixels) {
                //strip = new NeoPixelBus(_pixels, DEFAULT_WS2812_PIN);
                strip = new MyPixelBus(_pixels, DEFAULT_WS2812_PIN);

        }


        if (strip) {
                strip->Begin();
                strip->Show();
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
        if (pixels == _pixels) { return; }
        DebugMelvanimatef("NEW Pixels: %u\n", _pixels);
        strip->ClearTo(0);
        _pixels = pixels;
        _settings_changed = true;
        _init_LEDs();
        DebugMelvanimatef("HEAP: %u\n", ESP.getFreeHeap());
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
        File _settings;

        if (!_settings_changed && !override) { return false; }

        DebugMelvanimatef("Saving Settings: ");
        DynamicJsonBuffer jsonBuffer;
        JsonObject& root = jsonBuffer.createObject();

        JsonObject& globals = root.createNestedObject("globals");
        {
                globals["pixels"] = _pixels;
        }

        _settings = SPIFFS.open(MELVANA_SETTINGS, "w+");

        if (!_settings) { return false; }

        if (_mqtt) {
                _mqtt->addJson(globals);
        } else {
                JsonObject& MQTTjson = root["MQTT"];
                MQTTjson["enabled"] = false;
        }

        _settings.seek(0, SeekSet);
        root.prettyPrintTo(_settings);
        _settings_changed = false;
        _settings.close();
        return true;
}


bool Melvanimate::_loadGeneral()
{
        File _settings;

        DynamicJsonBuffer jsonBuffer;
        if (!_settings) {
                DebugMelvanimatef("[Melvanimate::load] ERROR File NOT open!\n");
                _settings = SPIFFS.open(MELVANA_SETTINGS, "r");
                if (!_settings) {
                        DebugMelvanimatef("[Melvanimate::load] No Settings File Found\n");
                        return false;
                }
        }

        char * data = nullptr;

        if (_settings.size()) {

                data = new char[_settings.size()];

        } else {
                DebugMelvanimatef("[Melvanimate::load] Fail: buffer size 0\n");
                return false;
        }

        // prevent nullptr exception if can't allocate
        if (data) {
                DebugMelvanimatef("[Melvanimate::load] buffer size %u\n", _settings.size());

                //  This method give a massive improvement in file reading speed for SPIFFS files..

                int bytesleft = _settings.size();
                int position = 0;
                while ((_settings.available() > -1) && (bytesleft > 0)) {

                        // get available data size
                        int sizeAvailable = _settings.available();
                        if (sizeAvailable) {
                                int readBytes = sizeAvailable;

                                // read only the asked bytes
                                if (readBytes > bytesleft) {
                                        readBytes = bytesleft;
                                }

                                // get new position in buffer
                                char * buf = &data[position];
                                // read data
                                int bytesread = _settings.readBytes(buf, readBytes);
                                bytesleft -= bytesread;
                                position += bytesread;

                        }
                        // time for network streams
                        delay(0);
                }


                JsonObject& root = jsonBuffer.parseObject(data);

                if (!root.success()) {
                        DebugMelvanimatef("[Melvanimate::load] Parsing settings file Failed!\n");
                        return false;
                } else { DebugMelvanimatef("[Melvanimate::load] Parse successfull\n"); }
// global variables
                if (root.containsKey("globals")) {

                        JsonObject& globals = root["globals"];

                        _pixels = globals["pixels"].as<long>();

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

                _settings.close();

                return true;

        } else {
                DebugMelvanimatef("[Melvanimate::load] Unable to Malloc for settings\n");
        }

};


bool Melvanimate::parse(JsonObject & root)
{

        bool code = false;

        if (root.containsKey("enable")) {
                String data = root["enable"];
                if ( data.equalsIgnoreCase("on")) {
                        Start();
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

        return code;
}



void Melvanimate::_initMQTT(JsonObject & root)
{



        IPAddress addr;
        uint16_t port = 0;
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

                        if (_mqtt) {
                                delete _mqtt;
                        }

                        if (MQTTjson["port"]) {

                                port = MQTTjson["port"];
                                _mqtt = new MelvanimateMQTT(this, addr, port);

                        } else {
                                _mqtt = new MelvanimateMQTT(this, addr);
                        }

                        DebugMelvanimatef("[Melvanimate::_initMQTT] (%u,%u,%u,%u) : %u \n", addr[0], addr[1], addr[2], addr[3], port );


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

                root["heap"] = ESP.getFreeHeap();
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


                        //	}

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

        DynamicJsonBuffer jsonBuffer;
        JsonObject& root = jsonBuffer.createObject();

        root["code"] = code;

/*

(page == "homepage") might be a breaking change for the GUI...

*/
        if (page == "homepage" || page == "all" ) {
                populateJson(root);
        }



        DebugMelvanimatef("[Melvanimate::_sendData] page = %s/n", page.c_str());

        if (page == "configpage" || page == "all") {

                root["pixels"] = getPixels();

                if (_mqtt) {
                        _mqtt->addJson(root);
                } else {
                        JsonObject & mqtt = root.createNestedObject("MQTT");
                        mqtt["enabled"] = false;
                }

        }

        if (page == "devicelist" || page == "all") {
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

        if (page == "presetspage") {

                JsonObject& settings = root.createNestedObject("settings");
                // adds minimum current effect name, if there if addJson returns false.
                if (Current()) {

                        settings["currentpreset"] = Current()->preset();
                        settings["currentpresetname"] = Current()->name();
                        addCurrentPresets(root);
                }

                addAllpresets(root);
        }

        // Serial.println("JSON REPLY");
        // root.prettyPrintTo(Serial);
        // Serial.println();

        _sendJsontoHTTP(root, request);

}

template <class T> void Melvanimate::_sendJsontoHTTP( const T & root, AsyncWebServerRequest *request)
{


        AsyncResponseStream *response = request->beginResponseStream("text/json");
        response->addHeader("Access-Control-Allow-Origin","*");
        response->addHeader("Cache-Control","no-store");
        root.printTo(*response);
        request->send(response);


}


void Melvanimate::_handleWebRequest(AsyncWebServerRequest *request)
{
        uint32_t start_time = millis();
        String page = "homepage";
        int8_t code = -1;

        //  this fires back an OK, but ignores the request if all the args are the same.  uses MD5.
//	if (_check_duplicate_req()) { _HTTP.setContentLength(0); _HTTP.send(200); return; }

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
                DebugMelvanimatef("[Melvanimate::_handleWebRequest] Page Requested = %s\n", page.c_str());
                page = request->getParam("data",true)->value();
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
                _timer.setTimeout(30, [num, this]() {
                        setPixels(num);
                });

                page = "layout";


                //  also submits mqtt data
                /*
                   [ARG:0] nopixels = 50
                   [ARG:1] enablemqtt = off
                   [ARG:2] mqtt_ip = 1.2.3.4
                   [ARG:3] mqtt_port = 123

                 */
        }


        if (p_root) {
                JsonObject & root = *p_root;

                if (request->hasParam("enablemqtt", true)) {

                        JsonObject & settings = root.createNestedObject("globals");
                        JsonObject & mqttjson = settings.createNestedObject("MQTT");

//		if ( request->getParam("enablemqtt") == "on" ) {

                        DebugMelvanimatef("[Melvanimate::_handleWebRequest] Enable MQTT..\n");

                        mqttjson["enabled"] = (  request->getParam("enablemqtt", true )->value() == "on") ? true : false;

                        IPAddress ip;

                        if (request->hasParam("mqtt_ip", true )) {
                                if (ip.fromString( request->getParam("mqtt_ip", true )->value())) {
                                        JsonArray & iparray = mqttjson.createNestedArray("ip");
                                        iparray.add(ip[0]);
                                        iparray.add(ip[1]);
                                        iparray.add(ip[2]);
                                        iparray.add(ip[3]);
                                }
                        }

                        if (request->hasParam("mqtt_port", true)) {
                                mqttjson["port"] = request->getParam("mqtt_port", true)->value();
                        }

#ifdef DebugMelvanimate
                        // Serial.println();
                        // mqttjson.prettyPrintTo(Serial);
                        // Serial.println();
#endif

                        _initMQTT(settings);

//		} else if (request->getParam("enablemqtt") == "off" ) {
//			DebugMelvanimatef("[_handleWebRequest] Disable mqtt..");





//		}


                }


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
                        Serial.printf("[Melvanimate::createAnimator] animator->IsAnimating() = true \n");
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
