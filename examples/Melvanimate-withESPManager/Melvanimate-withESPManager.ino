/*-------------------------------------------------------------------------------------------------------

              ESP8266 & Arduino IDE
              Animation software to control WS2812 - several requirements see



  Sticilface - Beerware licence

--------------------------------------------------------------------------------------------------------*/

#include <FS.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoOTA.h>
#include <NeoPixelBus.h>
#include <ArduinoJson.h>
#include <AsyncJson.h>
#include <AsyncMqttClient.h>
#include <Adafruit_GFX.h>
#include <ESPmanager.h>
#include <Melvanimate.h>
#include <Hash.h> //  required for platformio build
#include <ESP8266mDNS.h> // required for platformio build

//  these are default effects... comment them out here and in setup to remove.  Thats it.
#include "effects/SwitchEffect.h"
#include "effects/SimpleEffect.h"
#include "effects/DMXEffect.h"
#include "effects/AdalightEffect.h"
#include "effects/UDPEffect.h"
#include "effects/RainbowChase.h"
#include "effects/Shapes.h"
#include "effects/White.h"


const uint16_t defaultpixelcount =  20;
const char* devicename = "MyWS2812";



AsyncWebServer HTTP(80);
Melvanimate lights(HTTP, defaultpixelcount);  //  METHOD defaults to use RX pin, GPIO3, using DMA method... to change see mybus.h within Melvanimate
ESPmanager manager(HTTP, SPIFFS, devicename);

using namespace helperfunc; // used for things like dim.

// forward declarations sometimes needed!
void offFn(effectState &state, EffectHandler* ptr);
void SimpleColorFn(effectState &state, EffectHandler* ptr);

void setup()
{

  Serial.begin(115200);
  Serial.println("");
  Serial.println("Melvanimate - WS2812 control");


  SPIFFS.begin();

  manager.begin();

//  Add effects to the manager.
  lights.Add("Off",          new SwitchEffect( offFn), true);        //  **  Last true indicates this is the default effect... ie... off...
  lights.Add("SimpleColor",  new SimpleEffect(SimpleColorFn));
  lights.Add("RainbowChase", new RainbowChase);
  lights.Add("Shapes",       new Shapes);
  lights.Add("Adalight",     new AdalightEffect(Serial, 115200));   //  default serial device and baud.
  lights.Add("UDP",          new UDPEffect);
  lights.Add("DMX",          new DMXEffect );                       // need to test - requires custom libs included
  lights.Add("White",        new White); //



  lights.begin( manager.deviceName() );

  HTTP.serveStatic("/", SPIFFS , "/");
  HTTP.begin();

  Serial.print(F("Free Heap: "));
  Serial.println(ESP.getFreeHeap());
  Serial.print("Ready IP: ");
  Serial.println(WiFi.localIP());
}

void loop()
{

  lights.loop();
  manager.handle();


}



/*-----------------------------------------------
*                      offFn
*------------------------------------------------*/

void offFn(effectState &state, EffectHandler* ptr)
{

  if (ptr) {

    //  cast pointer to the class defined in the Add... allows you to access any functions within it..
    SwitchEffect& effect = *static_cast<SwitchEffect*>(ptr);

    switch (state) {

    case PRE_EFFECT: {


      // have to be careful of number of pixels.. < 300 generally OK.
      lights.createAnimator();
      effect.SetTimeout(1000); //  set speed through the effect
      lights.autoWait(); //  halts progress through states untill animator has finished..

      if (animator) {

        AnimEaseFunction easing = NeoEase::QuadraticInOut;

        for (uint16_t pixel = 0; pixel < strip->PixelCount(); pixel++) {

          RgbColor originalColor = myPixelColor(strip->GetPixelColor(pixel));  //myPixelColor is wrapper to convert RgbwColor to RgbColor if 4 color is used

          AnimUpdateCallback animUpdate = [ = ](const AnimationParam & param) {
            //float progress = easing(param.progress);
            float progress = param.progress;
            RgbColor updatedColor = RgbColor::LinearBlend(originalColor, RgbColor(0), progress);
            strip->SetPixelColor(pixel, updatedColor);
          };

          animator->StartAnimation(pixel, 1000, animUpdate);

        }
      }

    }

    break;
    case RUN_EFFECT: {
      strip->ClearTo(0);
      lights.deleteAnimator();  //  Not needed once off.
    }
    break;
    case POST_EFFECT: {

    }
    }
  }
}


/*-----------------------------------------------
*
*                 SimpleColorFn
*
*------------------------------------------------*/

void SimpleColorFn(effectState &state, EffectHandler* ptr)
{

  if (ptr) {

    SimpleEffect& effect = *static_cast<SimpleEffect*>(ptr);

    //  gets the next colour
    //  dim is located in helperfunc;
    RgbColor newColor = dim( effect.color(), effect.brightness() );

    switch (state) {

    case PRE_EFFECT: {

      // creates animator, default size is number of pixels.
      lights.createAnimator();

      effect.SetTimeout(2000); //  set speed through the effect

      lights.autoWait(); //  halts progress through states until animator has finished animating

      if (animator) {

        AnimEaseFunction easing = NeoEase::QuadraticInOut;

        for (uint16_t pixel = 0; pixel < strip->PixelCount(); pixel++) {

          RgbColor originalColor = myPixelColor(strip->GetPixelColor(pixel));  //myPixelColor is wrapper to convert RgbwColor to RgbColor if 4 color is used

          AnimUpdateCallback animUpdate = [ = ](const AnimationParam & param) {

            //float progress = easing(param.progress);
            float progress = param.progress;
            RgbColor updatedColor = RgbColor::LinearBlend(originalColor, newColor, progress);
            strip->SetPixelColor(pixel, updatedColor);
          };

          animator->StartAnimation(pixel, 1000, animUpdate);

        }
      }

      break;
    }

    case RUN_EFFECT: {
      if (strip) {
        strip->ClearTo(newColor);
      }

      lights.deleteAnimator();

      break;
    }
    case POST_EFFECT: {

      break;
    }
    case EFFECT_REFRESH: {
      state = PRE_EFFECT;
      break;
    }

    }
  }
}
