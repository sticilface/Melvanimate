

// ToDo

//  Delete empty settings Files
//  add device name to toolbar in jquery



#pragma once

#include "Arduino.h"
#include <functional>
#include "IPAddress.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "BufferedPrint.h"
#include "helperfunc.h"

#include "MelvanimateMQTT.h"


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



//#define DebugMelvanimate

#ifdef DebugMelvanimate
#define DebugMelvanimatef(...) Serial.printf(__VA_ARGS__)
#else
#define DebugMelvanimatef(...) {}
#endif

using namespace helperfunc; 


// globals for neopixels.
extern MyPixelBus * strip;
extern NeoPixelAnimator * animator;


class MelvanimateMQTT;


class Melvanimate : public EffectManager
{
public:
	Melvanimate(ESP8266WebServer & HTTP, uint16_t pixels, uint8_t pin);

	bool begin();
	void loop() override;
	void deviceName(const char * name) { _deviceName = name; }
	const char * deviceName() { return _deviceName; }


	// pixel count functions
	void setPixels(const uint16_t pixels);
	inline const uint16_t getPixels() const  { return _pixels; }

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

	void populateJson(JsonObject & root, bool onlychanged = false) ; 

private:
	bool _saveGeneral(bool override = false);
	bool _loadGeneral();
	void _init_LEDs();
	void _initMQTT(JsonObject & root);

	void _sendData(String page, int8_t code); 
	void _handleWebRequest();
	void _handleMQTTrequest(char* topic, byte* payload, unsigned int length);

	template <class T> static void _sendJsontoHTTP( const T& root, ESP8266WebServer & _HTTP) ;
	//bool _check_duplicate_req();


	MelvanimateMQTT * _mqtt{nullptr};

	const char * _deviceName{nullptr}; 
	uint16_t  _pixels;
	uint8_t _pin;
	bool _settings_changed;

	uint8_t _waiting{0};
	uint32_t _waiting_timeout{0};

	int _timerState{-1};
	SimpleTimer _timer;

	ESP8266WebServer & _HTTP;

	uint32_t _power{0}; 
	uint32_t _powertick{0}; 

};








