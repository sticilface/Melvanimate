

// ToDo

//  Delete empty settings Files
//  add device name to toolbar in jquery



#pragma once

#include "Arduino.h"
#include <functional>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "BufferedPrint.h"
#include "helperfunc.h"

#include <NeoPixelBus.h>
#include <NeoPixelAnimator.h>

#include <FS.h>

#define MELVANA_SETTINGS "/MelvanaSettings.txt"
#define EFFECT_WAIT_TIMEOUT 20000
#define DEFAULT_WS2812_PIN 2
#define MAX_NUMBER_OF_ANIMATIONS 300


#include "mybus.h"
#include "EffectManager.h"
#include "Melvtrix.h" // this is a variation on the NeomAtrix lib, that uses callbacks to pass x,y,pixel back to function 
#include "SimpleTimer/_SimpleTimer.h" // modified version that can return time to event
#include "ObjectManager.h"



#define DebugMelvanimate

#ifdef DebugMelvanimate
#define DebugMelvanimatef(...) Serial.printf(__VA_ARGS__)
#else
#define DebugMelvanimatef(...) {}
#endif

using namespace helperfunc; 


// globals for neopixels.
extern MyPixelBus * strip;
extern NeoPixelAnimator * animator;


class Melvanimate : public EffectManager
{
public:
	Melvanimate(ESP8266WebServer & HTTP, uint16_t pixels, uint8_t pin);

	bool begin();
	void loop() override;

	// pixel count functions
	void setPixels(const uint16_t pixels);
	inline const uint16_t getPixels() const  { return _pixels; }

	// Matrix functions
	void  grid(const uint16_t x, const uint16_t y);
	void  setmatrix(const uint8_t i);
	inline const uint8_t getmatrix() const { return _matrixconfig; }
	Melvtrix * matrix() { return _matrix; } //  returns pointer to the GFX melvtrix
	inline const uint16_t getX() const {  return _grid_x ; }
	inline const uint16_t getY() const {  return _grid_y; }
	bool multiplematrix = false; //

	// autowait functions
	void setWaiting(bool wait = true);
	void autoWait();
	bool returnWaiting();

	// timer functions
	bool setTimer(int timer, String command, String effect = String() );

	int getTimeLeft(); 

	uint32_t getPower(); 

	bool createAnimator(uint16_t count); 
	bool createAnimator(); 
	void deleteAnimator(); 

	//static RgbColor dim( RgbColor input, const uint8_t brightness);

private:
	bool _saveGeneral(bool override = false);
	bool _loadGeneral();
	void _init_LEDs();
	void _init_matrix();

	void _sendData(String page, int8_t code); 
	void _handleWebRequest();
	template <class T> static void _sendJsontoHTTP( const T& root, ESP8266WebServer & _HTTP) ;
	bool _check_duplicate_req();



	uint16_t  _pixels;
	uint8_t _pin;
	Melvtrix * _matrix;
	uint8_t _matrixconfig;
	uint16_t _grid_x, _grid_y;
	bool _settings_changed;
	//File _settings;

	uint8_t _waiting{0};
	uint32_t _waiting_timeout{0};

	int _timerState{-1};
	SimpleTimer _timer;

	ESP8266WebServer & _HTTP;

	uint32_t _power{0}; 
	uint32_t _powertick{0}; 

};








