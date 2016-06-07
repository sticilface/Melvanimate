/*-------------------------------------------------------------------------------------------------------

              ESP8266 & Arduino IDE
              Animation software to control WS2812 - several requirements see 



  Sticilface - Beerware licence

--------------------------------------------------------------------------------------------------------*/

#include <FS.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESPmanager.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>
#include <NeoPixelBus.h>
#define MQTT_MAX_PACKET_SIZE 256 //  this overrides the default packet size for pubsubclient packet.. otherwise it is 128 bytes, too small.  
#include <PubSubClient.h>
#include <Adafruit_GFX.h>
#include "Melvanimate.h"


//  these are default effect... comment them out here and in setup to remove.  Thats it. 
#include "effects/SwitchEffect.h"
#include "effects/SimpleEffect.h"
#include "effects/DMXEffect.h"
#include "effects/AdalightEffect.h"
#include "effects/UDPEffect.h"
#include "effects/RainbowChase.h"
#include "effects/Shapes.h"


const uint16_t defaultpixelcount =  48;
const char* devicename = "MyWS2812";  
const char* ssid     = "ssid";
const char* password = "password";


ESP8266WebServer HTTP(80);
Melvanimate lights(HTTP, defaultpixelcount , 2);  //  pin is ignored, should use DMA (RXD) or UART (GPIO2) methods.
ESPmanager settings(HTTP, SPIFFS, "ESPManager"); //Integrated from ESPman
using namespace helperfunc; // used for things like dim. 


void setup()
{

  Serial.begin(115200);
  Serial.println("");
  
  SPIFFS.begin();
  
  Serial.println();
  Serial.println();

  Serial.println("SPIFFS FILES:");
  {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      Serial.printf("     FS File: %s\n", fileName.c_str());
    }
    Serial.printf("\n");
  }



  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());


//  Add effects to the manager.
  lights.Add("Off",          new SwitchEffect( offFn), true);        //  **  Last true indicates this is the default effect... ie... off...
  lights.Add("SimpleColor",  new SimpleEffect(SimpleColorFn));       
  lights.Add("RainbowChase", new RainbowChase); 
  lights.Add("Shapes",       new Shapes); 
  lights.Add("Adalight",     new AdalightEffect(Serial, 115200));   //  default serial device and baud. 
  lights.Add("UDP",          new UDPEffect);                        
  lights.Add("DMX",          new DMXEffect );                       // need to test - requires custom libs included

  
  lights.begin();

  lights.deviceName(devicename);  
  lights.Start("Off");

  HTTP.begin();

  Serial.print(F("Free Heap: "));
  Serial.println(ESP.getFreeHeap());
  Serial.println("Ready"); 

}

void loop()
{
  HTTP.handleClient();
  settings.handle(); //Inserted from ESPMan
  lights.loop();
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

      if (animator) {
        delete animator;
      }

      // have to be careful of number of pixels.. < 300 generally OK. 
      lights.createAnimator();
      effect.SetTimeout(1000); //  set speed through the effect
      lights.autoWait(); //  halts progress through states untill animator has finished..

      if (animator) {

        AnimEaseFunction easing = NeoEase::QuadraticInOut;

        for (uint16_t pixel = 0; pixel < strip->PixelCount(); pixel++) {

          RgbColor originalColor = strip->GetPixelColor(pixel);

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

          RgbColor originalColor = strip->GetPixelColor(pixel);

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













