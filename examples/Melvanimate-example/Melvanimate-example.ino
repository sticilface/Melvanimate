/*-------------------------------------------------------------------------------------------------------

              ESP8266 & Arduino IDE
              Animation software to control WS2812 - several requirements.
              Software requires jquery + jquerymobile in SPIFFS, along with index.htm
              Use SPIFFS Data upload to flash files to SPIFFS.
              Connect WS2812 to PIN 2 of ESP8266.

  Sticilface - Beerware licence




  TODO urgent...

  Load effect....

  -> get effects used the file flag on effect but this is not passed to webpage...
  ->  webpage should add it to the
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

#define MQTT_MAX_PACKET_SIZE 256 //  this overrides the default packet size for pubsubclient packet.. otherwise it is 128 bytes. 

#include <PubSubClient.h>

#include <Adafruit_GFX.h>

#include <ESPmanager.h>
#include <FSBrowser.h>

#include "Melvanimate.h"


#include "effects/SwitchEffect.h"
#include "effects/SimpleEffect.h"
#include "effects/Effect2.h"
#include "effects/DMXEffect.h"
#include "effects/AdalightEffect.h"
#include "effects/UDPEffect.h"
#include "effects/RainbowChase.h"
#include "effects/blobs.h"


#define WS2812_PIXELS 118

#define Debug

#ifdef Debug
#define Debugf(...) Serial.printf(__VA_ARGS__)
#else
#define Debugf(...) {}
#endif


using namespace helperfunc;


// extern "C"  {
//   #include <cont.h>
//   extern cont_t g_cont;
// }

//  this is the native SDK json lib.
//#include <json/json.h>


ESP8266WebServer HTTP(80);
FSBrowser fsbrowser(HTTP);
//IPAddress mqtt_server_ip(192, 168, 1, 1);


SimpleTimer timer;

//ESPmanager settings(HTTP, SPIFFS, "Melvanimate-square", "SKY", "wellcometrust");
//ESPmanager settings(HTTP, SPIFFS, "Melvanimate-square", "fyffest", "wellcometrust");

//ESPmanager settings(HTTP, SPIFFS, "Melvanimate-square", "SONET_1", "tachi123");
//ESPmanager settings(HTTP, SPIFFS, "Melvanimate", "VodafoneMobileWiFi-CDD1C0", "WCZ8J89175");
//ESPmanager settings(HTTP, SPIFFS, "Melvanimate", "MobileWiFi-743e", "wellcometrust");

//ESPmanager settings(HTTP, SPIFFS, "Melvanimate", "Andrew's iPhone", "jok4axwt4vf4u");
ESPmanager settings(HTTP, SPIFFS);//, "lamp", "fyffest", "wellcometrust");



Melvanimate lights(HTTP, WS2812_PIXELS, 2);
//MelvanimateMQTT MQTT(lights);

void install_effects()
{


  lights.Add("Off",          new SwitchEffect( offFn), true);        //  **  Last true indicates this is the default effect... ie... off... REQUIRED
  lights.Add("SimpleColor",  new SimpleEffect(SimpleColorFn));       
  lights.Add("RainbowChase", new RainbowChase); 
  lights.Add("BobblySquares", new SimpleEffect2(BobblySquaresFn)); 
  lights.Add("Blobs", new Blobs); 

  lights.Add("CuriousCat",   new Effect2);
  lights.Add("Adalight",     new AdalightEffect(Serial, 115200));    // working 
  lights.Add("UDP",          new UDPEffect);                        // working
  lights.Add("DMX",          new DMXEffect );                       // need to test - requires custom libs included
  
  // for (uint8_t i = 0; i < 30; i++) {
  //   String in = "CuriousCat" + String(i);
  //   const char * string = strdup(in.c_str());
  //   lights.Add(7, string ,  new Effect2, true);
  // }


  // lights.Add("Marquee", new MarqueeEffect(MarqueeFn));                      // works. need to add direction....
  // lights.Add("Dummy", new DummyEffect(DummyFn));
  // lights.Add("PropertyTester", new CascadeEffect(CascadeEffectFn));
  // lights.Add("test", new testclass);
  // lights.Add("RainbowCycle", new SwitchEffect(RainbowCycleFn));
  // lights.Add("Rainbow", new SwitchEffect(RainbowFn));
  // lights.Add("BobblySquares", new SwitchEffect(BobblySquaresFn));

// experimental and in testing

//  lights.Add("TIMINGfunc", new SwitchEffect(TimingFn));
  // lights.Add("generic", new Effect(SimpleFn));
  // lights.Add("complex", new ComplexEffect(ComplexFn));
  // lights.Add("oldsnakes", new SwitchEffect(SnakesFn));
  // lights.Add("Object", new SwitchEffect(ObjectFn));

}

//  MQTT

//WiFiClient mqtt_wclient;
//PubSubClient mqtt(mqtt_wclient, mqtt_server_ip);

void setup()
{

  Serial.begin(115200);
  Serial.println("");
  //Serial.setDebugOutput(true);
  SPIFFS.begin();

  //SPIFFS.remove(PRESETS_FILE);

  install_effects();

  lights.begin();

  settings.begin();

  lights.deviceName(settings.deviceName());  //  must go after page load.


  fsbrowser.begin();

  Serial.println("SPIFFS FILES:");
  {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      Serial.printf("     FS File: %s, size: %s\n", fileName.c_str(), String(fileSize).c_str());

      if (fileName.startsWith(PRESETS_FILE)) {
        // SPIFFS.remove(fileName);
        // Serial.printf("     Deleted: %s\n", fileName.c_str());

      }

    }
    Serial.printf("\n");
  }


  HTTP.on("/restart", HTTP_ANY, []() {         ESP.restart();   });

  // HTTP.on("/memory", HTTP_ANY, []() {
  //    size_t freemem = cont_get_free_stack(&g_cont);

  //   Serial.printf("[stack] free = %u\n",freemem ); 
  //   Serial.printf("[heap] free = %u\n",ESP.getFreeHeap() ); 
  //   HTTP.send(200); // sends OK if were just receiving data...


  // });

  HTTP.on("/debug", HTTP_GET, []() {
    static bool debugstate = false;
    debugstate = !debugstate;
    Serial.setDebugOutput(debugstate);
    HTTP.setContentLength(0);
    HTTP.send(200); // sends OK if were just receiving data...
  });

  HTTP.on("/array", HTTP_ANY, [] () {
    lights.fillPresetArray(); 
  });

   HTTP.on("/command", HTTP_ANY, []() {
    Serial.println();
    if (HTTP.hasArg("save")) {
      if (HTTP.hasArg("name")) {
        lights.Save(HTTP.arg("save").toInt(), HTTP.arg("name").c_str());
      } else {
        lights.Save(HTTP.arg("save").toInt(), "No Name");

      }
      Serial.printf("[HTTP.on/command] done, heap: %u\n", ESP.getFreeHeap());
      HTTP.setContentLength(0);
      HTTP.send(200); // sends OK if were just receiving data...
    }

    if (HTTP.hasArg("load")) {
      lights.Load(HTTP.arg("load").toInt());
      Serial.printf("[load] done, heap: %u\n", ESP.getFreeHeap());
      Serial.printf("[load] current preset = %u\n", lights.Current()->preset());
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

  });

  //void serveStatic(const char* uri, fs::FS & fs, const char* path, const char* cache_header = NULL );


  HTTP.begin();

// -------------------------------------------------------- //



  lights.Start("Off");


  Debugf("HEAP: ");
  Debugf("%u\n", ESP.getFreeHeap());

  Debugf("Melvanimate Ready\n");



  // //network.send(44, "ESPtest123", 10, 1000000);
  // network.set_receiver(receiver_function);


  // timer.setInterval(1000, []() {
  //   uint32_t tick = micros();
  //   size_t len = strlen(settings.getHostname());

  //   network.send(44, (char*)settings.getHostname(), len);

   
  //   //Serial.printf("[PIRC send] %uus\n", micros() - tick); 
  // });

}

void loop()
{

    // size_t freemem = cont_get_free_stack(&g_cont);

  uint32_t _tick = millis();
  uint32_t _arrays[10] = {0};
  uint8_t poss = 0;
  _arrays[0] = millis();

  HTTP.handleClient();

 settings.handle();

  lights.loop();

  //Show_pixels(false); // bool is show pixels override...

  timer.run();




  // if (millis() - _tick > 1000 ) {
  //   Serial.printf("Loop >1S %u\n", millis() - _tick);
  // }

}

// void Show_pixels(bool override)
// {
//   static uint32_t tick = 0;
//   if (override) { tick = 0; }
//   if ( millis() - tick < 30) { return; }
//   if (animator) {
//     if ( animator->IsAnimating() ) {
//      animator->UpdateAnimations(); 
//    }
//   }
//   strip->Show();
//   tick = millis();
// }

// void testshapegenerater()
// {
//   uint16_t pixels[4] ;
//   Palette localP(WHEEL, 8 * 4);
//   static uint8_t position = 0;
//   static uint8_t counter = 0;
//   RgbColor next = Palette::wheel(counter);

//   Melvtrix& matrix = *lights.matrix();

//   matrix.setShapeFn( [&localP, &next] (uint16_t pixel, int16_t x, int16_t y) {
//     FadeToAndBack(pixel, next, 400);
//   });

//   matrix.drawRect(0 + position, 0 + position, 8 -  2 * position, 8 - 2 * position, 0);

//   int16_t circle[20] { -1};
//   int16_t line[20] { -1};


//   position++;
//   position %= 4;
//   counter += 10;
// }

// void FadeToAndBack(uint16_t pixel, RgbColor color, uint16_t time)
// {
  // RgbColor originalcolor = strip->GetPixelColor(pixel);
  // AnimUpdateCallback animUpdate = [pixel, originalcolor, color] (float progress) {
  //   RgbColor updatedColor;
  //   if (progress < 0.5) {
  //     updatedColor = RgbColor::LinearBlend(originalcolor, color, progress * 2 );
  //   } else {
  //     updatedColor = RgbColor::LinearBlend(color, 0, (progress - 0.5) * 2 );
  //   }
  //   strip->SetPixelColor(pixel, updatedColor);
  // };
  // StartAnimation(pixel, time , animUpdate);
//}

// void OnOff(uint16_t pixel, RgbColor color, uint16_t time)
// {
//   AnimUpdateCallback animUpdate = [pixel, color] (float progress) {
//     if (progress < 1.0) {
//       strip->SetPixelColor(pixel, color);
//     } else {
//       strip->SetPixelColor(pixel, RgbColor(0));
//     }

//   };
//   StartAnimation(pixel, time , animUpdate);
// }










// void StartAnimation( uint16_t pixel, uint16_t time, AnimUpdateCallback animUpdate)
// {
//   if (animator) {
//     animator->StartAnimation(pixel, time, animUpdate);
//   }

// }

// void FadeTo(RgbColor color)
// {
//   uint32_t current_brightness = 0;
//   uint32_t target_brightness = color.CalculateBrightness();
//   uint32_t brightness = 0;

//   for (uint16_t i = 0; i < strip->PixelCount(); i++) {
//     current_brightness += strip->GetPixelColor(i).CalculateBrightness();
//   }
//   current_brightness /= strip->PixelCount();

//   if (current_brightness > target_brightness) {
//     brightness = current_brightness;
//   } else {
//     brightness = target_brightness;
//   }

//   //int32_t difference = abs(brightness - color.CalculateBrightness() );

// //  Serial.printf("[FadeTo] current brightness %u, target brightness %u, Brightness Diff = %u, time %ums\n", current_brightness, target_brightness, brightness, brightness * 8);

//   FadeTo(brightness * 8, color);
// }

// void FadeTo( uint16_t time, RgbColor color)
// {
//   if (animator) {
//     animator->FadeTo(time, color);
//   } else {
//     strip->ClearTo(color);
//   }

// }










