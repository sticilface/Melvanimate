# Melvanimate 
[![Build Status](https://travis-ci.org/sticilface/Melvanimate.svg?branch=master)](https://travis-ci.org/sticilface/Melvanimate)

An all-in-one WS2812 solution for the ESP8266!  
This is a complete rework of my previous project [WS2812-WiFi](https://github.com/sticilface/ESP8266-wifi), it now uses C++ style coding and very flexible effect methods, using abstract handler classes and polymorphism! WiFi and OTA is no longer managed in the example, you now have to do it yourself, or use my [ESPManager](https://github.com/sticilface/ESPmanager).  This project is a lot more stable than WS2812-Wifi and has a lot more heap.  Mainly as the ESP8266 only sends the required data in json, not the whole web page!, which is kept in SPIFFS.  

## Features
+ Full control via JqueryMobile interface.  Fast, responsive and efficient (only shows relevant options)
+ Basic control via MQTT (can be disabled, saves about 1K RAM).
+ Custom MQTT manager that buffers MQTT msg sending so they are not all sent together which hangs the loop.  Big improvement to GUI and effects. 
+ Significant memory improvements, everything is allocated dynamically!  Melvanimate-Example heap sits at 35.4k leaving lots for individual effects to run. 
+ Property manager handles effect variables reliably, easily and transparently to GUI / SPIFFS. 
+ Full intelligent preset management, saving, creating, removing, loading.  All automatic you just add a variable to an effect.  Only required vars are saved with an effect, or loaded from a saved effect. 
+ Palette creates lots of different or similar colours for effects. 
+ Timer function. Turn lights off, or select preset after time. 
+ Full Adafruit GFX support, can write effects using grid, or draw text, including fonts also works, matrix settings are saved with that effect. You can have different matrix configurations for different effects. 
+ LED length set in software, technically only limited by RAM. Some effects for example Simplecolour will create an animator where size is equal to pixelcount.  This is bad if there are too many. Around 300 is what I've used with animations but that was starting with 20K heap.  With 30K  heap you can proably drive more with animations.  If you are not using animator then you can use even more.  just disable animator in off, and simplecolour.    
+ Effects can be added or removed with one line of code, in setup(), simple!  (can also remove the #include -/+ callback function)
+ GUI reports current heap and power usage of WS2812. 

## Important **PLEASE READ**
+ **WARNING**:  this is beta software, most likely to work with lateset pull of ESP8266 Ardunio and other dependancies. Most likely NOT to work with older versions.  
+ **WARNING**:  this is a fairly complex set of code for me... there will be BUGS. and crashes...  let me know if you fine one please, via issues:)
+ Runs by default on GPIO2 using UART. DMA method is available but the typdef located in ./mybus.h needs to be changed. See NeoPixelBus for options
+ The MQTT_MAX_PACKET_SIZE in sketch must be there, otherwise pubsub only supports tiny 128B packets. May have to increase if you have a lot of presets. 
+ Presets are saved to SPIFFS using json. Up to 1K, then a new file is created.  Settings files are chained together to avoid memory problems as whole json must fit into memory.  The files are read at boot, and each preset change, and each setting is placed in an array, with its name, effect and file location... ie...  the number of presets is limited only by uint_8 but don't go crazy.. i've not tested its limits. 
+ Call a preset Default or default for it to be loaded by default when that effect is selected. usefull for say Adalight, where you want a particular baudrate. 
+ JqueryMobile files are loaded from a CDN, so you need a working internet connection.  They 'can' be laoded locally but it is slower, and im trying to work out how to have the best of both worlds... 
+ My javascript is shocking!  It really is cobbled together, and buggy.  Any changes / comments welcome. 

## Dependancies
+ [NeoPixelBus](https://github.com/Makuna/NeoPixelBus) - requires current master. 
+ [ArduinoJson](https://github.com/bblanchon/ArduinoJson)
+ [PubSubClient](https://github.com/knolleary/pubsubclient)
+ [Adafruit_GFX](https://github.com/adafruit/Adafruit-GFX-Library)
+ **Requires index.html and jqColorPicker.min.js.gz to be placed in SPIFFS root.**
+ Requires several ArduinoESP8266 Libs
  * FS.h
  * ESP8266WiFi.h
  * ESP8266WebServer.h


## Included Effects
+ SimpleColour (Fades all pixels to one colour).
+ RainbowChase (Adafruit's rainbow chase). 
+ Blobs (A complex effect, working with linear strips, or 2D matrix (with shapes), fading in and out, varies in number)
+ Adalight (Via serial up to 2,000,000 baud tested).
+ DMX (via [E131](https://github.com/forkineye/E131)) Not perfect yet. 
+ UDP (Stream RGB packets straight to ESP, works from [PixelController](http://pixelinvaders.ch/?page_id=160)).
+ Snakes (to come).
+ Linear Blends (to come).

## Palette (not totally finished)
+ Created to give different scenes of colours, similar to different.  
+ If effect has Palette defined with addVar in initVar(), it will appear as menu option in GUI. 
+ When created using addVar() all options saved with presets and sent to GUI.
+ see [here](http://www.tigercolor.com/color-lab/color-theory/color-harmonies.htm) for some explaination: 
+ Available options:  ```off``` - no palette management in = out, [```complementary```](http://www.tigercolor.com/color-lab/color-theory/color-harmonies.htm), [```monochromatic```](http://www.tigercolor.com/color-lab/color-theory/color-harmonies.htm), [```analogous```](http://www.tigercolor.com/color-lab/color-theory/color-harmonies.htm), [```splitcomplements```](http://www.tigercolor.com/color-lab/color-theory/color-harmonies.htm), ```triadic``` - a three colour multi, ```tetradic``` - a 4 colour multi, ```multi``` - not fully implemented yet.. get any number of points spread around the 360 deg colour wheel, ```wheel``` - cycles through colours of adafruit colour wheel. 
+ The input colour for the palette choice is set by randomness.  Can be Off, timebased, or loopbased. Timebased choses a new random input colour every X seconds. 

## Matrix
+ Uses Adafruit GFX lib. 
+ Custom Matrix implementation that uses a callback to draw the corresponding pixel
+ See blobs for how it is used. 
+ Can be used to draw text, and shapes using neopixels. 
```c++
				// here matrix() returns pointer to the matrix created with addVar(); 
				matrix()->setShapeFn( [](uint16_t pixel, int16_t x, int16_t y) {
					strip->SetPixelColor( pixel, RgbColor(255,0,0)); 
				});
```

## MQTT (might be buggy) 
+ Configured dynaimcally to save memory.  Turn on/off in config menu. 
+ Might need some more testing.... 
+ Adapted manager that saves messages and sends them async, so loop doesn't stall if you send 10 messages. 
+ device subscribes to deviceid/+/set, the + can be any variable created with addVar().  Eg..  device/brightness/255 will set brightness to 255.
+ It should reply with the new value. 
+ On boot, it will send a variety of things, in json format... subscribe using MQTTspy and take a look. 
+ On boot, it will send a variety of things, in json format... subscribe using MQTTspy and take a look. (not working at the moment)
+ General Commands
  * device/toggle/set on/off (turns it on or off, if it has been on since boot)
  * device/effect/set effectname (starts the effect, case insensitive)
  * device/preset/set ID of preset (loads preset of ID x OR name x  either will work, but it will load the first matching name.  Case insensitive)


## Effect Design
+ See included effects for the many different ways of doing things. 
  * Create your own class, derive from EffectHandler. (see RainbowChase, DMXEffect, AdalightEffect)
  * Use switch effect (Register a callback at definition in .ino file) (see offFn in EffectCallbacks.ino)
  ```lights.Add("Off", new SwitchEffect( offFn), true); ``` (the true at the end designates the default effect... ie.. off)
  * Use SimpleEffect, derived from SwitchEffect, but now includes variables brightness and colour. 
+ animator should be created dynamically within the effect if needed. global ```NeoPixelAnimator * animator``` can be using for this. Updated every 30ms when running.  Otherwise declare your own and handle it in the run function.  
+ To run effects require to override the following.  	
	1.  ```virtual bool Start();``` - Called after initVars() so you can use effects
	2.  ```virtual bool Run();``` - Called everyloop. rate can be set in SwitchEffect using SetTimeout(uint32_t). 
	3.  ```virtual bool Stop();```  - Called before next effect is started, can delete any memory allocated, fade out etc.. 
+ To add variables in the InitVars() of effect.  This will then handle saving of this variable, and sending of this variable to GUI front end.  Uses templating to store and retrieve values. Supported types include your basic numbers bool, uint8_t, uint16_t, uint32_t, float, const char *, IPAddress, Palette*, MelvtrixMan* (only one).  So you can declare a palette and then use it.  The name given is what it is stored as and sent to the GUI/MQTT as.  So it has to match for the frontend to pick it up. brightness for example will create a slider 0-255 on front page. declare a palette here, and the pallette option will show up. 
```c++	
	bool InitVars()
	{
	  // addVar(new Variable<TYPE T>(const char * name, T default ));
		addVar(new Variable<uint8_t>("brightness", 255)); // example of 8bit variable
		addVar(new Variable<RgbColor>("color1", RgbColor(0))); // example of RgbColor Object
	} 
```
+ To use variables, declare member functions that return the value from getVar. 

```c++
	inline uint8_t speed() { return getVar<uint8_t>("speed"); }
	inline uint8_t brightness() { return getVar<uint8_t>("brightness"); } 
```


Happy Coding... 

#Credits (in progress)
+ [Arduino ESP8266](https://github.com/esp8266/arduino) 
+ [Adafruit GFX](https://github.com/adafruit/Adafruit-GFX-Library) + [Matrix](https://github.com/adafruit/Adafruit_NeoMatrix) - modified matrix to work with callbacks. 
+ Makuna [NeoPixelBus](https://github.com/Makuna/NeoPixelBus)
+ [SimpleTimer](https://github.com/infomaniac50/SimpleTimer) - Used internally with version that can give timeleft.  
+ Forkineye [E131](https://github.com/forkineye/E131)

