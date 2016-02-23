/*-------------------------------------------------------------------------------------------------------

              ESP8266 & Arduino IDE
              Animation software to control WS2812 - several requirements.
              Software requires jquery + jquerymobile in SPIFFS, along with index.htm
              Use SPIFFS Data upload to flash files to SPIFFS.
              Connect WS2812 to PIN 2 of ESP8266.

  Sticilface - Beerware licence
--------------------------------------------------------------------------------------------------------*/

//#include <GDBStub.h>


#include <FS.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoOTA.h>

#include <ArduinoJson.h>
#include <NeoPixelBus.h>
//#include <pubsubclient.h> //  to be implemented
#include <Adafruit_GFX.h>

#include <ESPmanager.h>
#include <FSBrowser.h>
#include <Melvanimate.h>
#include "SimpleTimer/_SimpleTimer.h"

#include "class_effects.h"

//  this is the native SDK json lib.
//#include <json/json.h>

// #include <cont.h>
// #include <stddef.h>
// #include <ets_sys.h>


ESP8266WebServer HTTP(80);
FSBrowser fsbrowser(HTTP);
//ESPmanager settings(HTTP, SPIFFS, "Melvanimate-square", "SKY", "wellcometrust");

//ESPmanager settings(HTTP, SPIFFS, "Melvanimate-square", "SONET_1", "tachi123");

//ESPmanager settings(HTTP, SPIFFS, "Melvanimate", "VodafoneMobileWiFi-CDD1C0", "WCZ8J89175");

ESPmanager settings(HTTP, SPIFFS, "Melvanimate", "MobileWiFi-743e", "wellcometrust");



class testclass : public EffectHandler, public Color_property, public Brightness_property, public Palette_property
{

public:
  testclass(): Color_property(this), Brightness_property(this), Palette_property(this) {};


  bool Run() override
  {
    if ( millis() - _timer > 1000) {
      Serial.printf("[testclass::Run] bri = %u, col = (%u,%u,%u)\n", _brightness, _color.R, _color.G, _color.B);
      _timer = millis();
    }
  };
  bool Start() override
  {
    Serial.println("[testclass::Start]");
  };
  bool Stop() override
  {
    Serial.println("[testclass::Stop]");
  };
  void Refresh() override
  {
    Serial.println("[testclass::Refresh]");
  };

  // bool addEffectJson(JsonObject& settings) override
  // {
  //  Serial.printf("[CascadeEffect::addJson] Called\n");
  //  Serial.println("[CascadeEffect::addJson] root");
  //  settings.prettyPrintTo(Serial);
  //  Serial.println();
  // }

//  bool parseJsonEffect(JsonObject& root) override;

  // bool testFn()
  // {
  //  _color = RgbColor(0, 0, 0);
  //  _brightness = 255;
  // }

private:
  uint32_t _timer = 0;
};




struct XY_t {
  int x;
  int y;
} XY;

//  MQTT

IPAddress mqtt_server_ip(192, 168, 1, 1);

//WiFiClient mqtt_wclient;
//PubSubClient mqtt(mqtt_wclient, mqtt_server_ip);

//  This initialises everything.

Melvanimate lights;

uint32_t save_flag = 0;
bool modechange = false;


// foreward dec for arudino
// void StartAnimation( uint16_t pixel, uint16_t time, AnimUpdateCallback animUpdate);
// void FadeTo(RgbColor color);
// void FadeTo( uint16_t time, RgbColor color);
// void crashfunc();
// void handle_data();
// void Show_pixels(bool override);
// void FadeToAndBack(uint16_t pixel, RgbColor color, uint16_t time);
// void send_data(String page);



// void MarqueeFn(effectState state, EffectHandler* ptr);
// void offFn(effectState state, EffectHandler* ptr);
// void SimpleColorFn(effectState state, EffectHandler* ptr);
// void AdaLightFn(effectState state, EffectHandler* ptr);


// class SwitchEffect;
// class GeneralEffect;
// class AdalightEffect;
// class MarqueeEffect;


void setup()
{

  Serial.begin(115200);
  Serial.println("");
  //Serial.setDebugOutput(true);

  SPIFFS.begin();

  lights.begin();

  settings.begin();
  fsbrowser.begin();


  HTTP.on("/crash", HTTP_ANY, crashfunc);

  // HTTP.on("/stack", HTTP_ANY, []() {
  //   cont_ stackvars;
  //   if (cont_get_free_stack(&stackvars)){

  //   }

  // });

  HTTP.on("/data.esp", HTTP_ANY, handle_data);
  HTTP.on("/debug", HTTP_GET, []() {
    static bool debugstate = false;
    debugstate = !debugstate;
    Serial.setDebugOutput(debugstate);
    HTTP.setContentLength(0);
    HTTP.send(200); // sends OK if were just receiving data...
  });

  HTTP.on("/command", HTTP_ANY, []() {
    if (HTTP.hasArg("save")) {
      if (HTTP.hasArg("name")) {
        lights.newSave(HTTP.arg("save").toInt(), HTTP.arg("name").c_str());
      } else {
        lights.newSave(HTTP.arg("save").toInt(), "No Name");

      }
      Serial.printf("[HTTP.on/command] done, heap: %u\n", ESP.getFreeHeap());
      HTTP.setContentLength(0);
      HTTP.send(200); // sends OK if were just receiving data...
    }

    if (HTTP.hasArg("load")) {
      lights.newLoad(HTTP.arg("load").toInt());
      Serial.printf("[load] done, heap: %u\n", ESP.getFreeHeap());
      Serial.printf("[load] current preset = %u\n", lights.Current()->getPreset());
      HTTP.setContentLength(0);
      HTTP.send(200); // sends OK if were just receiving data...
    }

    if (HTTP.hasArg("print")) {
      File f = SPIFFS.open(PRESETS_FILE, "r");
      Serial.println("SETTINGS_FILE");

      do {
        char buf[250];
        uint8_t number = (f.size() - f.position() > 250) ? 250 : f.size() - f.position();
        f.readBytes(buf, number);
        Serial.write(buf, number);
      } while (f.position() < f.size());

      Serial.println("---");

      HTTP.setContentLength(0);
      HTTP.send(200); // sends OK if were just receiving data...
    }

    if (HTTP.hasArg("remove")) {
      lights.removePreset(HTTP.arg("remove").toInt());
      HTTP.setContentLength(0);
      HTTP.send(200); // sends OK if were just receiving data...
    }

    if (HTTP.hasArg("list")) {

      Serial.printf("[list] _numberofpresets = %u\n", lights._numberofpresets);

      for (uint8_t i = 0; i < lights._numberofpresets; i++) {

        char * text = lights._preset_names[i];


        Serial.printf("[%u] %u (%s)\n", i, lights._presets[i], text) ;

      }
    }

  });

  void serveStatic(const char* uri, fs::FS & fs, const char* path, const char* cache_header = NULL );

  HTTP.serveStatic("/jqColorPicker.min.js", SPIFFS, "/jqColorPicker.min.js", "max-age=86400");

  HTTP.begin();

// -------------------------------------------------------- //



  lights.Add("Off", new SwitchEffect( offFn));                              // working
  lights.Add("SimpleColor", new SimpleEffect(SimpleColorFn));              // working
  lights.Add("CuriousCat", new Effect2); 
  
  // lights.Add("Adalight", new AdalightEffect(AdaLightFn));                    // working - need to test

  // lights.Add("UDP", new SwitchEffect(UDPFn));                              // working
  // // lights.Add("DMX", new SwitchEffect(DMXfn));                              // need to test - requires custom libs included
  // lights.Add("Marquee", new MarqueeEffect(MarqueeFn));                      // works. need to add direction....

  // lights.Add("Dummy", new DummyEffect(DummyFn));
  // lights.Add("PropertyTester", new CascadeEffect(CascadeEffectFn));



  lights.Add("test", new testclass);


  // lights.Add("RainbowCycle", new SwitchEffect(RainbowCycleFn));
  // lights.Add("Rainbow", new SwitchEffect(RainbowFn));
  // lights.Add("BobblySquares", new SwitchEffect(BobblySquaresFn));

// experimental and in testing

 // lights.Add("TIMINGfunc", new SwitchEffect(TimingFn));
  // lights.Add("generic", new Effect(SimpleFn));
  // lights.Add("complex", new ComplexEffect(ComplexFn));
  // lights.Add("oldsnakes", new SwitchEffect(SnakesFn));
  // lights.Add("Object", new SwitchEffect(ObjectFn));


  //timer.setTimeout(5000, []() { lights.Start("Marquee");} ) ;

  // timer.setInterval(1000, []() {
  //   Debugf("HEAP: %u\n", ESP.getFreeHeap());
  // });

  //Adalight_Flash();


  //timer.setTimeout(1000, []() { lights.Start("BobblySquares");} ) ;


  //timer.setTimeout(2000, []() { lights.Start("Off");} ) ;

  lights.Start("Off");

  // timer.setInterval(500, []() {
  //   text2Fn("Wellcome To the Jungle");
  // });

  Serial.print("HEAP: ");
  Serial.println(ESP.getFreeHeap());

  Serial.println(F("Melvanimate Ready"));

  // lights.Current()->GeneralEffect::*pmf()

  //bool (GeneralEffect::*fptr) (uint8_t) = &GeneralEffect::setBrightness;


//  (*fptr)(100);

  // Melvtrix & matrix =  *lights.matrix();

  // ShapeUpdateCallback ShapeCallback = [](uint16_t pixel, int16_t x, int16_t y) {
  //   Serial.printf("%3u ", pixel);
  // };


  // matrix.setShapeFn( ShapeCallback );


  // for (int x = 0; x < matrix.width(); x++) {
  //   for (int y = 0; y < matrix.height(); y++) {
  //     matrix.drawPixel(x, y);
  //   }
  //   Serial.println();
  // }

  // lights.palette().getModeString();


  // do {
  //   Serial.println("remove presets");
  // } while (SPIFFS.remove(PRESETS_FILE) );

  // do {
  //   Serial.println("remove presets");
  // } while (SPIFFS.remove("/MelvanaSettings.txt") );

}

void loop()
{
  uint32_t _tick = millis();
  uint32_t _arrays[10] = {0};
  uint8_t poss = 0;
  _arrays[0] = millis();

  HTTP.handleClient();

  settings.handle();

  lights.Loop();

  Show_pixels(false); // bool is show pixels override...

  timer.run();

  if (save_flag) {
    if (millis() - save_flag > 100) {
      save_flag = 0;
      lights.save(modechange); //  will only save if actually required.
    }
  }



  // if (millis() - _tick > 1000 ) {
  //   Serial.printf("Loop >1S %u\n", millis() - _tick);
  // }

}

void Show_pixels(bool override)
{
  static uint32_t tick = 0;
  if (override) { tick = 0; }
  if ( millis() - tick < 30) { return; }
  if (lights.animations()) {
    if ( animator->IsAnimating() ) { animator->UpdateAnimations(100); }
  }
  strip->Show();
  tick = millis();
}









void testshapegenerater()
{
  uint16_t pixels[4] ;
  Palette localP(WHEEL, 8 * 4);
  static uint8_t position = 0;
  static uint8_t counter = 0;
  RgbColor next = Palette::wheel(counter);

  Melvtrix& matrix = *lights.matrix();

  matrix.setShapeFn( [&localP, &next] (uint16_t pixel, int16_t x, int16_t y) {
    FadeToAndBack(pixel, next, 400);
  });

  matrix.drawRect(0 + position, 0 + position, 8 -  2 * position, 8 - 2 * position, 0);

  int16_t circle[20] { -1};
  int16_t line[20] { -1};


  position++;
  position %= 4;
  counter += 10;
}

void FadeToAndBack(uint16_t pixel, RgbColor color, uint16_t time)
{
  RgbColor originalcolor = strip->GetPixelColor(pixel);
  AnimUpdateCallback animUpdate = [pixel, originalcolor, color] (float progress) {
    RgbColor updatedColor;
    if (progress < 0.5) {
      updatedColor = RgbColor::LinearBlend(originalcolor, color, progress * 2 );
    } else {
      updatedColor = RgbColor::LinearBlend(color, 0, (progress - 0.5) * 2 );
    }
    strip->SetPixelColor(pixel, updatedColor);
  };
  StartAnimation(pixel, time , animUpdate);
}

void OnOff(uint16_t pixel, RgbColor color, uint16_t time)
{
  AnimUpdateCallback animUpdate = [pixel, color] (float progress) {
    if (progress < 1.0) {
      strip->SetPixelColor(pixel, color);
    } else {
      strip->SetPixelColor(pixel, RgbColor(0));
    }

  };
  StartAnimation(pixel, time , animUpdate);
}


void print_args()
{


  for (uint8_t i = 0; i < HTTP.args(); i++) {
    Serial.print("[ARG:");
    Serial.print(i);
    Serial.print("] ");
    Serial.print(HTTP.argName(i));
    Serial.print(" = ");
    Serial.println(HTTP.arg(i));
    Serial.flush();
  }
}

//  this is required as some
bool check_duplicate_req()
{
  static uint32_t last_time = 0;
  static char last_request[16] = {0};
  if (HTTP.hasArg("data")) { return false; }

  MD5Builder md5;
  md5.begin();

  for (uint8_t args = 0; args < HTTP.args(); args++) {
    String req = HTTP.argName(args) + HTTP.arg(args);
    md5.add(req);
  }

  md5.calculate();
  bool match = false;
  //Serial.printf("[MD5] %s\n", md5.toString().c_str());
  char this_request[16] = {0};
  md5.getChars(this_request);

  if (memcmp(last_request, this_request, 16) == 0) {
    match = true;
    Serial.println("Request ignored: duplicate");
  }

  memcpy(last_request, this_request, 16);

  bool time_elapsed = (millis() - last_time > 10000) ? true : false;
  last_time = millis();

  return match & !time_elapsed;

}
void handle_data()
{
  uint32_t start_time = millis();
  String page = "homepage";
  //  this fires back an OK, but ignores the request if all the args are the same.  uses MD5.
  if (check_duplicate_req()) { HTTP.setContentLength(0); HTTP.send(200); return; }

  Serial.println();
  print_args();

  if (HTTP.hasArg("plain")) {
    //  ABANDONED
    // DynamicJsonBuffer jsonBufferplain;
    // JsonObject& root = jsonBufferplain.parseObject(HTTP.arg("plain").c_str());
    // if (root.success()) {

    //   if (lights.Current()) {
    //     if (lights.Current()->parseJsonArgs(root)) {
    //       Serial.println("[handle] JSON (via Plain) Setting applied");
    //     }
    //   }

    // }
  }

  if (HTTP.hasArg("enable")) {
    if (HTTP.arg("enable").equalsIgnoreCase("on")) {
      lights.Start();
    } else if (HTTP.arg("enable").equalsIgnoreCase("off")) {
      lights.Start("Off");
    }
  }

  if (HTTP.hasArg("mode")) {
    modechange = lights.Start(HTTP.arg("mode"));
    if (HTTP.arg("mode") != "Off") { lights.SetToggle(HTTP.arg("mode").c_str()); }
  }


  if (HTTP.hasArg("preset")) {
    uint8_t preset = HTTP.arg("preset").toInt();
    if (lights.newLoad(preset)) {
      //  try to switch current effect to preset...
      Serial.printf("[handle] Loaded preset %u\n", preset);
    }

  }



  // puts all the args into json...
  // might be better to send pallette by json instead..

  DynamicJsonBuffer jsonBuffer;
  JsonObject & root = jsonBuffer.createObject();

  for (uint8_t i = 0; i < HTTP.args(); i++) {
    root[HTTP.argName(i)] = HTTP.arg(i);
  }



  if (HTTP.hasArg("nopixels") && HTTP.arg("nopixels").length() != 0) {
    lights.setPixels(HTTP.arg("nopixels").toInt());
    page = "layout";

  }

  if (HTTP.hasArg("palette")) {
    //lights.palette().mode(HTTP.arg("palette").c_str());
    page = "palette"; //  this line might not be needed... palette details are now handled entirely by the effect for which they belong

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

    palettenode["mode"] = (uint8_t)Palette::stringToEnum(HTTP.arg("palette").c_str());


    if (HTTP.hasArg("palette-random")) {
      palettenode["randmode"] = (uint8_t)Palette::randommodeStringtoEnum(HTTP.arg("palette-random").c_str());
    }

    if (HTTP.hasArg("palette-spread")) {
      palettenode["range"] = HTTP.arg("palette-spread");

    }

    if (HTTP.hasArg("palette-delay")) {
      palettenode["delay"] = HTTP.arg("palette-delay");

    }
    Serial.println("[handle_data] JSON dump");
    root.prettyPrintTo(Serial);
    Serial.println();

  }




//  this has to go last for the JSON to be passed to the current effect
  if (lights.Current()) {
    if (lights.Current()->parseJson(root)) {
      Serial.println("[handle] JSON Setting applied");
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

  if (HTTP.hasArg("grid_x") && HTTP.hasArg("grid_y")) {
    lights.grid(HTTP.arg("grid_x").toInt(), HTTP.arg("grid_y").toInt() );
    page = "layout";
  }

  if (HTTP.hasArg("matrixmode")) {
    page = "layout";
    uint8_t matrixvar = 0;
    if (HTTP.arg("matrixmode") == "singlematrix") { lights.multiplematrix = false; }
    if (HTTP.arg("firstpixel") == "topleft") { matrixvar += NEO_MATRIX_TOP + NEO_MATRIX_LEFT; }
    if (HTTP.arg("firstpixel") == "topright") { matrixvar += NEO_MATRIX_TOP + NEO_MATRIX_RIGHT; }
    if (HTTP.arg("firstpixel") == "bottomleft") { matrixvar += NEO_MATRIX_BOTTOM + NEO_MATRIX_LEFT; }
    if (HTTP.arg("firstpixel") == "bottomright") { matrixvar += NEO_MATRIX_BOTTOM + NEO_MATRIX_RIGHT; }

    if (HTTP.arg("axis") == "rowmajor") { matrixvar += NEO_MATRIX_ROWS; }
    if (HTTP.arg("axis") == "columnmajor") { matrixvar += NEO_MATRIX_COLUMNS ; }

    if (HTTP.arg("sequence") == "progressive") { matrixvar += NEO_MATRIX_PROGRESSIVE ; }
    if (HTTP.arg("sequence") == "zigzag") { matrixvar += NEO_MATRIX_ZIGZAG ; }

    if (HTTP.arg("matrixmode") == "multiplematrix") {
      lights.multiplematrix = true;
      if (HTTP.arg("multimatrixtile") == "topleft") { matrixvar += NEO_TILE_TOP + NEO_TILE_LEFT; }
      if (HTTP.arg("multimatrixtile") == "topright") { matrixvar += NEO_TILE_TOP + NEO_TILE_RIGHT; }
      if (HTTP.arg("multimatrixtile") == "bottomleft") { matrixvar += NEO_TILE_BOTTOM + NEO_TILE_LEFT; }
      if (HTTP.arg("multimatrixtile") == "bottomright") { matrixvar += NEO_TILE_BOTTOM + NEO_TILE_RIGHT; }
      if (HTTP.arg("multimatrixaxis") == "rowmajor") { matrixvar += NEO_TILE_ROWS ; }
      if (HTTP.arg("multimatrixaxis") == "columnmajor") { matrixvar += NEO_TILE_COLUMNS ; }
      if (HTTP.arg("multimatrixseq") == "progressive") { matrixvar += NEO_TILE_PROGRESSIVE ; }
      if (HTTP.arg("multimatrixseq") == "zigzag") { matrixvar += NEO_TILE_ZIGZAG ; }
    }

    Debugf("NEW Matrix params: %u\n", matrixvar);
    lights.setmatrix(matrixvar);
  }


  if (HTTP.hasArg("flashfirst")) {
    page = "layout";
    lights.Start("Off");
    lights.Stop();
    strip->ClearTo(0);
    AnimUpdateCallback animUpdate = [] (float progress) {
      strip->SetPixelColor(0, Palette::wheel( (uint8_t)(progress * 255) ));
      if (progress == 1.0) { strip->SetPixelColor(0, 0); }
    };
    StartAnimation(0, 5000 , animUpdate);



  }

  if (HTTP.hasArg("revealorder")) {
    page = "layout";
    lights.Start("Off");
    lights.Stop();
    strip->ClearTo(0);
    // ToDo
    float ratio = 1.0 / strip->PixelCount();

    for (uint16_t pixel = 0; pixel < strip->PixelCount() ; pixel++) {
      AnimUpdateCallback animUpdate = [ratio, pixel] (float progress) {
        if ( (uint8_t)(progress * 100) == (uint8_t)(pixel * ratio * 100)) {
          strip->SetPixelColor(pixel, Palette::wheel( (uint8_t)(ratio * 255)));
          strip->SetPixelColor( (pixel > 2) ? pixel - 2 : 0 , 0 );

        }
        if (progress == 1.0) { lights.Start("Off"); }
      };
      StartAnimation(pixel, 5000 , animUpdate);
    }


  }



  // if (HTTP.hasArg("palette-random")) {
  //   lights.palette().randommode(HTTP.arg("palette-random").c_str());
  //   page = "palette";
  // }


  // if (HTTP.hasArg("palette-spread")) {
  //   lights.palette().range(HTTP.arg("palette-spread").toFloat());
  //   page = "palette";
  // }

  // if (HTTP.hasArg("palette-delay")) {
  //   lights.palette().delay(HTTP.arg("palette-delay").toInt());
  //   page = "palette";
  // }


  if (HTTP.hasArg("data")) {
    send_data(HTTP.arg("data")); // sends JSON data for whatever page is currently being viewed
    return;
  }

  if (HTTP.hasArg("enabletimer")) {
    page = "timer";
    if (HTTP.arg("enabletimer") == "on") {

      if (HTTP.hasArg("timer") && HTTP.hasArg("timercommand")) {

        String effect =  (HTTP.hasArg("timeroption")) ? HTTP.arg("timeroption") : String();

        if (lights.setTimer(HTTP.arg("timer").toInt(), HTTP.arg("timercommand"), effect )) {
          Serial.println("[handle] Timer command accepted");
        }
      }
    } else if (HTTP.arg("enabletimer") == "off") {
      lights.setTimer(0, "off");
    }

  }

  //HTTP.setContentLength(0);
  //HTTP.send(200); // sends OK if were just receiving data...
  send_data(page);


  save_flag = millis();
  Serial.printf("[handle] time %u: [Heap] %u\n", millis() - start_time, ESP.getFreeHeap());
  return;

}






void send_data(String page)
{
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  /*
        Home page
  */

  if (page == "homepage" || page == "palette" || page == "all") {
    JsonArray& modes = root.createNestedArray("modes");
    //Serial.printf("Total effects: %u\n", lights.total());
    for (uint8_t i = 0; i < lights.total(); i++) {
      modes.add(lights.getName(i));
    }
    // creates settings node for web page
    JsonObject& settings = root.createNestedObject("settings");
    // adds minimum current effect name, if there if addJson returns false.
    if (lights.Current()) {
      settings["currentpreset"] = lights.Current()->getPreset();

      if (!lights.Current()->addJson(settings)) {
        settings["effect"] = lights.Current()->name();
      }

      if (!settings.containsKey("effect")) {
        settings["effect"] = lights.Current()->name();
      }

      if (lights._numberofpresets) {
        JsonObject& currentpresets = root.createNestedObject("currentpresets");
        for (uint8_t i = 0; i < lights._numberofpresets; i++ ) {
          currentpresets[ String(lights._presets[i])] = lights._preset_names[i];
        }
      }
    }



// *  Not needed as palette is added within the add of a secific effect..

    // Palette * palette = lights.Current()->getPalette();
    // if (palette) {
    //   root["palettename"] = String(palette->getModeString());
    // }



  }
  /*
        Layout Page

  [ARG:3] matrixmode = singlematrix
  [ARG:4] firstpixel = topleft
  [ARG:5] axis = rowmajor
  [ARG:6] sequence = progressive
  [ARG:7] multimatrixtile = topleft
  [ARG:8] multimatrixaxis = rowmajor
  [ARG:9] multimatrixseq = progressive
  */
  if (page == "layout" || page == "all") {
    root["pixels"] = lights.getPixels();
    root["grid_x"] = lights.getX();
    root["grid_y"] = lights.getY();
    root["multiplematrix"] = lights.multiplematrix;

    root["matrixconfig"] = lights.getmatrix();

    uint8_t matrixconfig = lights.getmatrix();
    bool bottom = (matrixconfig & NEO_MATRIX_BOTTOM) ;
    bool right = (matrixconfig & NEO_MATRIX_RIGHT) ;

// single matrix
    if (!bottom && !right) { root["firstpixel"] = "topleft"; }
    if (!bottom && right) { root["firstpixel"] = "topright"; }
    if (bottom && !right) { root["firstpixel"] = "bottomleft"; }
    if (bottom && right ) { root["firstpixel"] = "bottomright"; }

    if ((matrixconfig & NEO_MATRIX_AXIS) == NEO_MATRIX_ROWS) {
      root["axis"] = "rowmajor";
    } else {
      root["axis"] = "columnmajor";
    }

    if ((matrixconfig & NEO_MATRIX_SEQUENCE) == NEO_MATRIX_PROGRESSIVE) {
      root["sequence"] = "progressive";
    } else {
      root["sequence"] = "zigzag";
    }


// Tiles

    bottom = (matrixconfig & NEO_TILE_BOTTOM) ;
    right = (matrixconfig & NEO_TILE_RIGHT) ;

    if (!bottom && !right) { root["multimatrixtile"] = "topleft"; }
    if (!bottom && right) { root["multimatrixtile"] = "topright"; }
    if (bottom && !right) { root["multimatrixtile"] = "bottomleft"; }
    if (bottom && right ) { root["multimatrixtile"] = "bottomright"; }

    if ((matrixconfig & NEO_TILE_AXIS) == NEO_TILE_ROWS) {
      root["multimatrixaxis"] = "rowmajor";
    } else {
      root["multimatrixaxis"] = "columnmajor";
    }


    if ((matrixconfig & NEO_TILE_SEQUENCE) == NEO_TILE_PROGRESSIVE) {
      root["multimatrixseq"] = "progressive";
    } else {
      root["multimatrixseq"] = "zigzag";
    }


  }

  /*
        palette page
  */

  // Palette is now handled by each effect handler... WICKED

  // if (page == "palette" || page == "all") {

  //   Palette * palette = lights.Current()->getPalette();

  //   if (palette) {

  //     if (page != "all") { root["palette"] = String(palette->getModeString()); } // ignore if already sent
  //     root["paletterandom"] = String(palette->randommodeAsString());
  //     root["palettespread"] = String(palette->range());
  //     root["palettedelay"] = String(palette->delay());

  //     if (palette->addJson( root)) {
  //       Serial.println("[send_data] palette data added");
  //       JsonObject& Palette = root["Palette"];
  //       Palette["modeString"] = String(palette->getModeString()); //  This adds it as string.. saves having it saved to SPIFFS.
  //     }

  //   }
  // }

  if (page == "timer" || page == "all") {

    JsonObject& timerobj = root.createNestedObject("timer");
    timerobj["running"] = lights.isTimerRunning();
    if (lights.isTimerRunning()) {
      JsonArray& remaining = timerobj.createNestedArray("remaining");
      int minutes = timer.getTimeLeft(lights.getTimer()) / ( 1000 * 60) ;
      int seconds = timer.getTimeLeft(lights.getTimer()) / 1000 ;
      seconds %= 60;
      remaining.add(minutes);
      remaining.add(seconds);
    }

  }

  // Serial.println("JSON REPLY"); 
  // root.prettyPrintTo(Serial);
  // Serial.println(); 

  ESPmanager::sendJsontoHTTP(root, HTTP);

}

void StartAnimation( uint16_t pixel, uint16_t time, AnimUpdateCallback animUpdate)
{
  if (lights.animations()) {
    animator->StartAnimation(pixel, time, animUpdate);
  }

}

void FadeTo(RgbColor color)
{
  uint32_t current_brightness = 0;
  uint32_t target_brightness = color.CalculateBrightness();
  uint32_t brightness = 0;

  for (uint16_t i = 0; i < strip->PixelCount(); i++) {
    current_brightness += strip->GetPixelColor(i).CalculateBrightness();
  }
  current_brightness /= strip->PixelCount();

  if (current_brightness > target_brightness) {
    brightness = current_brightness;
  } else {
    brightness = target_brightness;
  }

  //int32_t difference = abs(brightness - color.CalculateBrightness() );

  Serial.printf("[FadeTo] current brightness %u, target brightness %u, Brightness Diff = %u, time %ums\n", current_brightness, target_brightness, brightness, brightness * 8);

  FadeTo(brightness * 8, color);
}

void FadeTo( uint16_t time, RgbColor color)
{
  if (lights.animations()) {
    animator->FadeTo(time, color);
  } else {
    strip->ClearTo(color);
  }


}


void crashfunc()
{

  NeoPixelBus * voidpointer;

  voidpointer->Show();

}







