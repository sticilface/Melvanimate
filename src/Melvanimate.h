


#pragma once

#include "Arduino.h"
#include <functional>

#include <NeoPixelBus.h>
#include <FS.h>

#define MELVANA_SETTINGS "/MelvanaSettings.txt"
#define EFFECT_WAIT_TIMEOUT 20000
#define DEFAULT_WS2812_PIN 2
#define DEFAULT_TOTAL_PIXELS 64


#include "EffectManager.h"
#include "Melvtrix.h" // this is a variation on the NeomAtrix lib, that uses callbacks to pass x,y,pixel back to function 
#include "SimpleTimer/_SimpleTimer.h" // modified version that can return time to event
#include "ObjectManager.h"



//#define DebugMelvanimate

#ifdef DebugMelvanimate
#define DebugMelvanimatef(...) Serial.printf(__VA_ARGS__)
#else
#define DebugMelvanimatef(...) {}
#endif


// globals for neopixels.
extern NeoPixelBus * strip;
extern NeoPixelAnimator * animator;


class Melvanimate : public EffectManager
{
public:
	Melvanimate(uint16_t pixels, uint8_t pin);

	static const RgbColor dim( RgbColor input, const uint8_t brightness);

	void  grid(const uint16_t x, const uint16_t y);
	void  setmatrix(const uint8_t i);
	const uint8_t getmatrix() { return _matrixconfig; }
	Melvtrix *  matrix() { return _matrix; } //  returns pointer to the GFX melvtrix

	const uint16_t    getX() {  return _grid_x ; }
	const uint16_t    getY() {  return _grid_y; }
	const uint16_t    getPixels() { return _pixels; }

	void setPixels(const uint16_t pixels);
	bool begin();
	void Loop() override; 

	void setWaiting(bool wait = true);
	void autoWait();
	bool returnWaiting();

	bool setTimer(int timer, String command, String effect = String() );
	inline bool isTimerRunning() const { return (_timerState > -1); }
	inline int getTimer() const { return _timerState;  }

	bool multiplematrix = false; //

private:
	bool _saveGeneral(bool override = false);
	bool _loadGeneral();
	void _init_LEDs();
	void _init_matrix();
	uint16_t  _pixels;
	uint8_t _pin; 
	Melvtrix * _matrix;
	uint8_t _matrixconfig;
	uint16_t _grid_x, _grid_y;
	bool _settings_changed;
	File _settings{};

	uint8_t _waiting;
	uint32_t _waiting_timeout = 0;

	int _timerState = -1;
	SimpleTimer _timer; 

};








