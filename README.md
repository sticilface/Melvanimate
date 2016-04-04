# Melvanimate
An all-in-one WS2812 solution for the ESP8266!  
This is a complete rework of my previous lib [WS2812-WiFi](https://github.com/sticilface/ESP8266-wifi), it now uses C++ style coding and very flexible effect methods, using abstract handler classes and polymorphism! WiFi and OTA is notlonger manager in example, you now have to do it yourself, or use my [ESPManager](https://github.com/sticilface/ESPmanager).  This lib is a lot more stable than WS2812-Wifi and has a lot more heap.  Mainly as the ESP8266 only sends the required data in json, not the whole web page! 

## Features
+ Full control via JqueryMobile interface.  Fast, responsive and efficient (only shows relevant options)
+ Basic control via MQTT (can be disabled, saves RAM). 
+ Custom MQTT manager that buffers MQTT msg sending so they are not all send in one loop.  Big improvement to GUI and lights. 
+ Significant memory improvements!  Example heap sits at 35416 leaving lots for individual effects to run. 
+ Property manager handles effect variables reliably, easily and transparently.  
+ Full intelligent preset management, saving, creating, removing, loading.  All automatic you just add a variable to an effect. 
+ Palette creation to set available colours for effect.
+ Timer function. Turn lights off, or select preset after time. 
+ Full Adafruit GFX support, can write effects using grid, or draw text, including fonts also works. 
+ LED length set in software, technically only limited by heap. Some effects for example Simplecolour will create an animator whose size is = to pixelcount.  This is bad if there are too many. so 300 is a practical limit.  However if you design your own effect, you can handle this.. create a new strip instance, use that... etc...   
+ Effects can be added or removed with one line of code, in setup(), simple! 
+ GUI reports current heap and power usage of WS2812. 

## Important **PLEASE READ**
+ **WARNING**:  this is beta software, most likely to work with lateset pull of ESP8266 Ardunio and other dependancies. Most likely NOT to work with older versions.  
+ **WARNING**:  this is a fairly complex set of code for me... there will be BUGS. and crashes...  let me know if you fine one please :)
+ Runs by default on GPIO2 using UART. DMA method is available but the typdef located in ./mybus.h needs to be changed. 
+ The MQTT_MAX_PACKET_SIZE in sketch must be there, otherwise pubsub only supports tiny 128B packets. May have to increase if you have a lot of presets. 
+ Presets are saved to SPIFFS using json. Up to 1K, then a new file is created.  Settings files are chained together to avoid huge memory problems.  The files are read at boot, and each preset change, and each setting is placed in an array, with its name, effect and file location... ie...  this can get big too huge. 
+ Call a preset Default or default for it to be loaded by default when that effect is selected. usefull for say Adalight, where you want a particular baudrate. 
+ JqueryMobile files are loaded from a CDN, so you need a woring internet connection.  They 'can' be laoded locally but it is slower, and im trying to work out how to have the best of both worlds... 
+ My javascript is shocking!  It really is cobbled together, and buggy.  Any changes / comments welcome. 

## Dependancies
+ [NeoPixelBus](https://github.com/Makuna/NeoPixelBus) 
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
+ UDP (Stream RGB packets straight to ESP).
+ Snakes (to come).
+ Linear Blends (to come).

## Palette (not finished)
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

