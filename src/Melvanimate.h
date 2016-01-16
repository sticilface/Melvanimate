#pragma once

#include "Arduino.h"

#include <NeoPixelBus.h>
#include <FS.h>
#include <ArduinoJson.h>
#include <MD5Builder.h>
#include <functional>


#include "EffectManager.h"
#include "ObjectManager.h"
#include "Melvtrix.h" // this is a variation on the NeomAtrix lib, that uses callbacks to pass x,y,pixel back to function 
#include "Palette.h"
#include "e131/_E131.h"
#include "SimpleTimer/_SimpleTimer.h"



#define MELVANA_SETTINGS "/MelvanaSettings.txt"
#define maxLEDanimations 300
#define EFFECT_WAIT_TIMEOUT 20000
#define DEFAULT_WS2812_PIN 2


#define DebugMelvanimate

#ifdef DebugMelvanimate
#define Debug(x)    Serial.print(x)
#define Debugln(x)  Serial.println(x)
#define Debugf(...) Serial.printf(__VA_ARGS__)
#else
#define Debug(x)    {}
#define Debugln(x)  {}
#define Debugf(...) {}
#endif

class EffectManager;
class Palette;

// globals for various things, including neopixels...
extern const uint16_t TOTALPIXELS;
extern NeoPixelBus * strip;
extern NeoPixelAnimator * animator;
extern uint8_t* stripBuffer;
extern WiFiUDP Udp;
extern const IPAddress multicast_ip_addr; // Multicast broadcast address
extern const uint16_t UDPlightPort;
extern E131* e131;
extern SimpleTimer timer;


/*
 * uses effectmanager as base class to manage effects...
 * need to tidy..
 *
 * // ToDo
 *	1. Palletes...  including sending and choosing paletes
 *	2. Saving currnet mode / effect  ...  big ask..
 *	3. Create Entoopy variable...  Randomness selection...
 *  4. REMOVE VARS... colours, brightness etc... should be managed by effect itself...
 *
 *
 *
 */


class Melvanimate : public EffectManager
{
public:
	Melvanimate();
	const RgbColor  getColor();

	//const RgbColor  getColor2() { return dim(_color2); }

	const RgbColor  dim( RgbColor input) { return dim(input, getBrightness()); }
	static const RgbColor 	dim( RgbColor input, const uint8_t brightness);

	const uint8_t   getBrightness();
	void      		setBrightness(const uint8_t bright);

	void      color(const RgbColor color);
	const RgbColor  color();
	RgbColor nextcolor();

	void      color2(const RgbColor color);
	const RgbColor  color2() { return _color2; }
	void      speed(const uint8_t speed) { _settings_changed = true;  _speed = speed ; Refresh(); }
	const uint8_t   speed() { return _speed; }

	//const int       serialspeed() { return _serialspeed; }
	//void      serialspeed(const int speed);
	void        grid(const uint16_t x, const uint16_t y);
	void        setmatrix(const uint8_t i);

	const uint8_t     getmatrix() { return _matrixconfig; }
	Melvtrix *        matrix() { return _matrix; } //  returns pointer to the GFX melvtrix

	Palette & palette() { return *_palette; }

	const uint16_t    getX() {  return _grid_x ; }
	const uint16_t    getY() {  return _grid_y; }
	const uint16_t    getPixels() { return _pixels; }

	void        setPixels(const uint16_t pixels);
	bool        save() { return save(false); }
	bool 		save(bool);

	bool        load();
	bool        begin();
	bool		animations() {return _animations; }
	//void 		setText(String var);
	//const char * getText();

	void setWaiting(bool wait = true);
	void autoWait();
	bool returnWaiting();

	bool setTimer(int timer, String command, String effect = String() );
	bool isTimerRunning() { return (_timer > -1); }
	int getTimer() { return _timer;  }

	bool multiplematrix = false; //
	uint32_t timeoutvar;  //  parameters used by some effects...
	int32_t effectposition; //  just keep one copy of them.. save having loads of statics!

private:
	void _init_LEDs();
	void _init_matrix();
	uint16_t  _pixels;
	uint8_t _brightness;
	uint8_t _speed;
	RgbColor _color;
	RgbColor _color2;
	int _serialspeed;
	Melvtrix * _matrix;
	uint8_t _matrixconfig;
	uint16_t _grid_x, _grid_y;
	bool _settings_changed;
	String _marqueetext = "Welcome to Melvana"; // default text!
	bool _animations;
	File _settings = File();
	uint8_t _waiting;
	uint32_t _waiting_timeout = 0;

	Palette * _palette;
	int _timer = -1;


};








