#pragma once

#include "Arduino.h"

#include <NeoPixelBus.h>
#include "Melvanimate.h"
#include "Melvtrix.h"
#include <FS.h>
#include <ArduinoJson.h>


#define MELVANA_SETTINGS "/MelvanaSettings.txt"
#define maxLEDanimations 300
#define EFFECT_WAIT_TIMEOUT 20000
#define DEFAULT_WS2812_PIN 2


#define DebugMelvana

#ifdef DebugMelvana
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

extern const uint16_t TOTALPIXELS;
extern NeoPixelBus * strip;
extern NeoPixelAnimator * animator;
extern uint8_t* stripBuffer;


class Melvana : public EffectManager
{
public:
	Melvana();
	const RgbColor  getColor() { return dim(_color); }
	const RgbColor  getColor2() { return dim(_color2); }

	const RgbColor  dim( RgbColor input) { return dim(input, _brightness); }
	static const RgbColor 	dim( RgbColor input, const uint8_t brightness);

	const uint8_t   getBrightness() { return _brightness;}
	void      		setBrightness(const uint8_t bright);

	void      color(const RgbColor color);
	const RgbColor  color() { return _color; }
	RgbColor nextcolor() { 
		if(_palette) { return dim(_palette->next()); } else { return RgbColor(0); }
	}
	void      color2(const RgbColor color);

	const RgbColor  color2() { return _color2; }
	void      speed(const uint8_t speed) { _settings_changed = true;  _speed = speed ; Refresh(); }
	const uint8_t   speed() { return _speed; }

	const int       serialspeed() { return _serialspeed; }
	void      serialspeed(const int speed);
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
	void 		setText(String var);
	const char * getText();

	void setWaiting(bool wait = true);
	void autoWait();
	bool returnWaiting();

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

};