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
#include <SimpleTimer.h>
#include <NeoPixelBus.h>
#include <ESPmanager.h>
#include <FSBrowser.h>

#include <Adafruit_GFX.h>


#include <Melvanimate.h>






ESP8266WebServer HTTP(80);
FSBrowser fsbrowser(HTTP);
//ESPmanager settings(HTTP, SPIFFS, "Melvanimate-square", "SKY", "wellcometrust");

//ESPmanager settings(HTTP, SPIFFS, "Melvanimate-square", "SONET_1", "tachi123");

//ESPmanager settings(HTTP, SPIFFS, "Melvanimate", "VodafoneMobileWiFi-CDD1C0", "WCZ8J89175");

ESPmanager settings(HTTP, SPIFFS, "Melvanimate", "MobileWiFi-743e", "wellcometrust");


SimpleTimer timer;

Palette palette;

struct XY_t {
  int x;
  int y;
} XY;


//  This initialises everything.

Melvanimate lights;


void setup()
{

  Serial.begin(115200);
  Serial.println("");

  SPIFFS.begin();

  lights.begin();

  settings.begin();
  fsbrowser.begin();


  HTTP.on("/crash", HTTP_ANY, crashfunc);

  HTTP.on("/data.esp", HTTP_ANY, handle_data);
  HTTP.on("/debug", HTTP_GET, []() {
    static bool debugstate = false;
    debugstate = !debugstate;
    Serial.setDebugOutput(debugstate);
  });

  void serveStatic(const char* uri, fs::FS & fs, const char* path, const char* cache_header = NULL );

  HTTP.serveStatic("/jqColorPicker.min.js", SPIFFS, "/jqColorPicker.min.js", "max-age=86400");

  HTTP.begin();

// -------------------------------------------------------- //

  lights.Add("Off", new SwitchEffect(offFn));                              // working
  lights.Add("SimpleColor", new SwitchEffect(SimpleColorFn));              // working
  lights.Add("Adalight", new SwitchEffect(AdaLightFn));                    // working - need to test
  lights.Add("UDP", new SwitchEffect(UDPFn));                              // working
  lights.Add("DMX", new SwitchEffect(DMXfn));                              // need to test - requires custom libs included
  lights.Add("Marquee", new SwitchEffect(MarqueeFn));                      // works. need to add direction....
  lights.Add("RainbowCycle", new SwitchEffect(RainbowCycleFn));
  lights.Add("Rainbow", new SwitchEffect(RainbowFn));
  lights.Add("BobblySquares", new SwitchEffect(BobblySquaresFn));

// experimental and in testing

  lights.Add("TIMINGfunc", new SwitchEffect(TimingFn));
  lights.Add("generic", new Effect(SimpleFn));
  lights.Add("complex", new ComplexEffect(ComplexFn));
  lights.Add("oldsnakes", new SwitchEffect(SnakesFn));
  lights.Add("Object", new SwitchEffect(ObjectFn));


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

  Serial.println(F("Melvanimate"));



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

  lights.palette().getModeString();




}

void loop()
{



  HTTP.handleClient();

  settings.handle();


  lights.Loop();
  Show_pixels(false); // bool is show pixels override...
  yield();
  timer.run();

  static uint32_t _tick = 0;

  if (millis() - _tick > 1000 ) {
    Serial.printf("Loop >1S %u\n", millis() - _tick);
  }

  _tick = millis();
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

void SimpleFn()
{
  static uint32_t tick = 0;
  if (millis() - tick < 20000) { return; }
  Serial.println("SimpleFn RUN");
  tick = millis();
}




void ComplexFn(char *i, char *j, double k)
{

}

struct XY_t FookinComplex(bool a , bool b, String c)
{

}



void interesting()
{


  //EffectObject * blob = new EffectObject (2);

  // //EffectObject blobO  (3);
  // static EffectObject * blob;

  // Serial.println("interesting called");
  // static bool switched = false;

  // const uint16_t number = 3;

  // if (!switched) {
  //   if (blob) delete blob;
  //   blob = new EffectObject (number);

  //   uint16_t offset = random(5);
  //   for (uint16_t i = 0; i < number; i++) {

  //     blob->Addpixel(i, i + offset);
  //     blob->SetColor(i, RgbColor(255, 0, 0));
  //   }
  //   blob->StartBasicAnimations(500);

  // }
  // else
  // {
  //   for (uint16_t i = 0; i < number; i++) {
  //     blob->SetColor(i, RgbColor(0, 0, 0));
  //   }
  //   blob->StartBasicAnimations(100);
  //   delete blob;
  //   blob = nullptr;
  // }

  // switched = !switched;


}


void interesting2()
{

  EffectObject blob0(2);
  EffectObject * blob = &blob0;

  //if (!blob) blob = new EffectObject(2);

  blob->SetObjectUpdateCallback( [&blob]() {

    for (uint8_t i = 0; i < 2; i++) {
      uint16_t pix = random(7);
      blob->Addpixel(i, pix);
    }
  });

  blob->SetPixelUpdateCallback(  [] (uint16_t position, uint16_t pixel) { // called per pixel of the effect, sent to neopixelbus

    RgbColor originalcolor = strip->GetPixelColor(pixel);

    AnimUpdateCallback animUpdate = [pixel, originalcolor] (float progress) {
      RgbColor updatedColor;

      if (progress < 0.5) {
        updatedColor = RgbColor::LinearBlend(originalcolor, RgbColor(255, 0, 0), progress * 2 );
      } else {
        updatedColor = RgbColor::LinearBlend(RgbColor(255, 0, 0), 0, (progress - 0.5) * 2 );
      }
      strip->SetPixelColor(pixel, updatedColor);
    };

    StartAnimation(pixel, 1000, animUpdate);

  });






  blob->UpdateObject();
  blob->StartAnimations();
  //delete blob;
  //blob = nullptr;

}


void ObjectFn(effectState state)
{
  static uint32_t tick = 0;
  static EffectObject * blob;
  static Palette* palette;

  tick = millis();

  switch (state) {

  case PRE_EFFECT: {
    const uint8_t points = 5;
    const uint32_t timeout = 5000;
    Serial.println("PRE: Blob");
    if (blob) { delete blob; }
    blob = nullptr;
    blob = new EffectObject(points);

    if (palette) { delete palette; }
    palette = new Palette;
    palette->mode(WHEEL);

    lights.SetTimeout(timeout);

    blob->SetObjectUpdateCallback( []() {
      uint16_t pixarray[points];
      for (uint8_t i = 0; i < points; i++) {
        bool found = true;
        uint8_t pix;
        do {
          pix = random(7);
          bool therealready = false;
          for (uint8_t j = 0; j < points; j++) {
            if (pix == pixarray[j]) {
              found = false;
              break;
            }
            found = true;
          }

        } while (found == false);
        pixarray[i] = pix;
      }
      for (uint8_t i = 0; i < points; i++) {
        blob->Addpixel(i, pixarray[i]);
      }
    });

    blob->SetPixelUpdateCallback(  [] (uint16_t position, uint16_t pixel) { // called per pixel of the effect, sent to neopixelbus
      RgbColor originalcolor = strip->GetPixelColor(pixel);
      AnimUpdateCallback animUpdate = [pixel, originalcolor] (float progress) {
        RgbColor nextcolour = palette->next();

        RgbColor updatedColor;
        if (progress < 0.5) {
          updatedColor = RgbColor::LinearBlend(originalcolor, nextcolour, progress * 2 );
        } else {
          updatedColor = RgbColor::LinearBlend(nextcolour, 0, (progress - 0.5) * 2 );
        }
        strip->SetPixelColor(pixel, updatedColor);
      };
      animator->StartAnimation(pixel, timeout, animUpdate);
    });


  }

  break;
  case RUN_EFFECT: {

    blob->UpdateObject();
    blob->StartAnimations();

  }
  break;

  case POST_EFFECT: {
    Serial.println("End: Blob");
    if (blob) { delete blob; }
    blob = nullptr;
    if (palette) { delete palette; }
    palette = nullptr;
  }
  break;
  }

}



struct EFFECT_s {

  struct position_s {
    uint16_t x = 0;
    uint16_t y = 0;
  } ;
  position_s * pPosition;

  EFFECT_s(uint8_t _count, uint8_t LEDs)//, uint8_t colorscount = 0)
  {
    count = _count;
    manager = new EffectGroup; // create effect group
    pPosition = new position_s[_count];
    matrix = lights.matrix();

    pGroup = new EffectObjectHandler* [count];
    Serial.println("Pre-create");
    delay(5);
    for (uint8_t i = 0; i < count; i++) {
      pGroup[i] =  manager->Add(i, lights.speed() , new EffectObject( LEDs ) );
      if (!pGroup[i]) { Serial.println("nullptr returned"); }
    }
    Serial.println("Post-create");
    delay(10);


  }
  ~EFFECT_s()
  {
    delete manager;
    delete[] colors;
    delete[] pGroup;
    delete[] pPosition;
  }
  void Run()
  {
    static bool triggered = false;
    if (!triggered) { Serial.println("run effect_s function hit"); triggered = true; };
    manager->Update();
  }
  EffectGroup* manager;
  RgbColor * colors;
  EffectObjectHandler ** pGroup;
  Melvtrix * matrix;
  uint8_t count = 0;

  position_s & position(uint16_t i) { return pPosition[i];}


};
//  Generates random squares, no fill...
//  Does
void BobblySquaresFn_create(struct EFFECT_s *&, bool, bool);

void BobblySquaresFn(effectState & state)
{
  static EFFECT_s* EFFECT = nullptr; // dont forget to initialise pointers... ARGHHHHHHHH


  switch (state) {

  case PRE_EFFECT: {
    Serial.println("PRE: Creating Objects");
    lights.SetTimeout( 0);
    lights.palette().mode(WHEEL);
    lights.palette().total(255) ;

    if (EFFECT) { delete EFFECT; EFFECT = nullptr; }

    EFFECT = new EFFECT_s(5, 25);

    BobblyShapeFn_create(EFFECT, true, true, random(0, 3));

  }

  break;
  case RUN_EFFECT: {
    static bool triggered = false;
    if (!triggered) { Serial.println("run function hit"); triggered = true; };
    EFFECT->Run();

  }
  break;

  case POST_EFFECT: {
    Serial.println("End: Blob");
    if (EFFECT) {
      delete EFFECT;
      EFFECT = nullptr;
    }
  }
  break;
  case EFFECT_REFRESH: {
    Serial.println("Refresh");
    state = PRE_EFFECT;
  }
  break;

  }

}

void BobblyShapeFn_create(struct EFFECT_s *& EFFECT, bool random1, bool random2, uint8_t shape)
{


  for (uint8_t obj = 0; obj < EFFECT->count; obj++) {

    EffectObjectHandler * current =  EFFECT->pGroup[obj];  //    pointer to current group of pixels...

    current->SetObjectUpdateCallback( [ current, EFFECT, obj, random1, random2, shape ]() {

      current->reset(); // new set of pixels...
      EFFECT->matrix->setShapeFn( [ EFFECT, obj, current, random1 ] (uint16_t pixel, int16_t x, int16_t y) {
        current->Addpixel(pixel);
      });

      uint8_t size = random(2, 6);
      uint16_t x = EFFECT->position(obj).x = random(0, lights.matrix()->width() - size + 1);
      uint16_t y = EFFECT->position(obj).y = random(0, lights.matrix()->height() - size + 1);

      uint16_t add_factor = (random1) ? random(5, 10) : 10;
      current->Timeout( lights.speed() * add_factor); // update speed of current effect!

      switch (shape) {
      case 0:
        EFFECT->matrix->drawRect(x, y,  size, size, 0); //  fills shape with
        break;
      case 1:
        EFFECT->matrix->drawCircle(x, y, size, 0); //  fills shape with
        break;
      case 2:
        EFFECT->matrix->drawTriangle(x, y, x + size, y, x + (size / 2), y + size, 0); //  fills shape with
        break;
      case 3:
        EFFECT->matrix->fillTriangle(x, y, x + size, y, x + (size / 2), y + size , 0); //  fills shape with
        break;
      }

      //EFFECT->matrix->drawRect(x, y,  size, size, 0); //  fills shape with

    });

    current->SetPixelUpdateCallback( [random2] (uint16_t n, uint16_t pixel) {
      uint16_t add_factor = (random2) ? random(5, 10) : 10;
      FadeToAndBack(pixel, lights.nextcolor(), lights.speed() * random(5, 10) );
      //FadeToAndBack(pixel, RgbColor(5,0,0), lights.speed() * random(5, 10) );

    });
  }

}

//  Text scrolling function.  Just called every 200ms by a timer.  simple...
// void displaytextOLD(const char * text, uint16_t timeout)
// {
//   static bool reset = true;
//   static int16_t count = 0;

//   if (reset) {
//     count = lights.getX();
//     reset = false;
//   }

//   uint16_t len = strlen(text) * 7;

//   Melvtrix & matrix = *lights.matrix();
//   matrix.setTextWrap(false);

//   if (count < lights.getX()) {
//     matrix.setShapeFn( [] (uint16_t pixel, int16_t x, int16_t y) {
//       strip->SetPixelColor(pixel, 0);
//     });

//     matrix.setCursor( count + 1, lights.getY() - 8  );
//     matrix.print(text);
//   }

//   RgbColor color = lights.dim(palette.next());

//   matrix.setShapeFn( [color] (uint16_t pixel, int16_t x, int16_t y) {


//     strip->SetPixelColor(pixel, color  );
//   });

//   matrix.setCursor( count--,  lights.getY() - 8  );
//   matrix.print(text);

//   if (count < -len) {
//     reset = true;
//     Serial.println("reset");
//   }

// }



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


  // matrix.setShapeFn( [&localP] (uint16_t pixel) {
  //   FadeToAndBack(pixel, localP.next(), 1000);
  // });

  // matrix.drawCircle(3, 3, 2, 0);

  //matrix.drawTriangle(1, 1, 1, 6, 6, 3, 0);


//  drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color),

  //animator->FadeTo(random(10000,60000), Palette::wheel(random(255)));

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
  if (HTTP.hasArg("data")) return false;

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
    //Serial.println("Request ignored: duplicate");
  }

  memcpy(last_request, this_request, 16);

  bool time_elapsed = (millis() - last_time > 10000) ? true : false;
  last_time = millis();

  return match & !time_elapsed;

}
void handle_data()
{
  //  this fires back an OK, but ignores the request if all the args are the same.  uses MD5.
  if (check_duplicate_req()) { HTTP.send(200); return; }

  print_args();
  bool modechange = false;
  if (HTTP.hasArg("plain")) {

    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(HTTP.arg("plain").c_str());
    if (root.success()) {
      if (root.containsKey("color")) {
        JsonObject& color = root["color"];
        RgbColor input;
        String nameid = color["name"];
        input.R = color["R"];
        input.G = color["G"];
        input.B = color["B"];
        if (nameid == "color1") {
          lights.color(input);
          Debugf("Settings: RGB(%u,%u,%u)\n", lights.color().R, lights.color().G, lights.color().B);
        }
        if (nameid == "color2") {
          lights.color2(input);
          Debugf("Settings: RGB(%u,%u,%u)\n", lights.color2().R, lights.color2().G, lights.color2().B);
        }
      }
    }
  }
  if (HTTP.hasArg("mode")) {
    modechange = lights.Start(HTTP.arg("mode"));
  }

  if (HTTP.hasArg("brightness")) {
    lights.setBrightness(HTTP.arg("brightness").toInt());
  }

  if (HTTP.hasArg("speed")) {
    lights.speed(HTTP.arg("speed").toInt());
  }

  if (HTTP.hasArg("rotation")) {
    uint8_t rotation = HTTP.arg("rotation").toInt();
    if (rotation > 3) rotation = 0;
    lights.matrix()->setRotation( rotation );
    lights.Refresh();
  }

  if (HTTP.hasArg("nopixels") && HTTP.arg("nopixels").length() != 0) {
    lights.setPixels(HTTP.arg("nopixels").toInt());
  }

  if (HTTP.hasArg("palette")) {
    lights.palette().mode(HTTP.arg("palette").c_str());
  }

  if (HTTP.hasArg("marqueetext")) {
    lights.setText(HTTP.arg("marqueetext")) ;
    lights.Refresh();
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
  }

  if (HTTP.hasArg("matrixmode")) {
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


  if (HTTP.hasArg("serialspeed")) {
    lights.serialspeed(HTTP.arg("serialspeed").toInt());
  }

  if (HTTP.hasArg("flashfirst")) {
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


  if (HTTP.hasArg("data")) {
    send_data(HTTP.arg("data")); // sends JSON data for whatever page is currently being viewed
    return;
  }

  lights.save(modechange); //  will only save if actually required.


  HTTP.send(200); // sends OK if were just receiving data...

}

void send_data(String page)
{
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  /*
        Home page
  */

  if (page == "homepage" || page == "all") {
    JsonArray& modes = root.createNestedArray("modes");
    //Serial.printf("Total effects: %u\n", lights.total());
    for (uint8_t i = 0; i < lights.total(); i++) {
      modes.add(lights.getName(i));
    }

    root["currentmode"] = lights.getName();
    root["brightness"] = lights.getBrightness();
    root["speed"] = lights.speed();

    JsonObject& color = root.createNestedObject("color1");
    color["R"] = lights.color().R;
    color["G"] = lights.color().G;
    color["B"] = lights.color().B;

    JsonObject& color2 = root.createNestedObject("color2");
    color2["R"] = lights.color2().R;
    color2["G"] = lights.color2().G;
    color2["B"] = lights.color2().B;

    root["serialspeed"] = lights.serialspeed();

    root["rotation"] = lights.matrix()->getRotation();
    root["marqueetext"] = lights.getText();

    root["palette"] = String(lights.palette().getModeString());
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


  if (page == "palette" || page == "all") {

    if (page != "all") root["palette"] = String(lights.palette().getModeString()); // ignore if already sent
    root["paletterandom"] = String(lights.palette().randommodeAsString());
    root["palettespread"] = String(lights.palette().range());
    root["palettedelay"] = String(lights.palette().delay());




  }

//  root.prettyPrintTo(Serial);
//  Serial.println();

  ESPmanager::sendJsontoHTTP(root, HTTP);

}

void StartAnimation( uint16_t pixel, uint16_t time, AnimUpdateCallback animUpdate)
{
  if (lights.animations()) {
    animator->StartAnimation(pixel, time, animUpdate);
  }

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







