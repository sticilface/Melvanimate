//  toDo

/*

1.  ?Integrate Events, event posting,
2.  ?automatic refresh
3.  Use RTC memory to retrieve toggled settings!  think if it = OFF or default handle then only toggle the on variable of rtc data...




*/


#pragma once

#include <Arduino.h>

#include <NeoPixelBus.h>
#include <functional>
#include <IPAddress.h>
#include <ESP8266WiFi.h>

#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "helperfunc.h"
#include "EQ.h"

#include "MelvanimateMQTT.h"
#include "UDP_broadcast.h"

//#include <NeoPixelAnimator.h>
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

//#define DebugMelvanimate Serial
//#define RANDOM_MANIFEST
//#define DISABLE_MANIFEST   //  don't really want this.. as if manifest 404s then it is loaded from cache!  use random to force reload!

#if defined(DebugMelvanimate)
#define DebugMelvanimatef(...) DebugMelvanimate.printf(__VA_ARGS__) //  this saves around 5K RAM...
//#define DebugMelvanimatef(_1, ...) DebugMelvanimate.printf_P( PSTR(_1), ##__VA_ARGS__) //  this saves around 5K RAM...
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


	Melvanimate(AsyncWebServer & HTTP, uint16_t pixels, uint8_t pin = 2);

	bool begin(const char * name);
	void loop() override;
	void deviceName(const char * name) { _deviceName = name; }
	const char * deviceName() { return _deviceName; }

	void addJQueryhandlers();

	// pixel count functions
	void setPixels(const int pixels);
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

	bool parse(JsonObject & root);

	void setEventsServer(AsyncEventSource * events) {
		_events = events;
	}

	void sendEvent(const char * msg, const char * topic);


private:

	void _saveState();
	void _getState();

	bool _saveGeneral(bool override = false);
	bool _loadGeneral();
	void _init_LEDs();
	void _initMQTT(JsonObject & root);
	void _MQTTsubscribe();

	void _sendData(String page, int8_t code,AsyncWebServerRequest *request);
	void _handleWebRequest(AsyncWebServerRequest *request);
	void _handleManifest(AsyncWebServerRequest *request);
	void _checkheap();
	//void _handleMQTTrequest(char* topic, byte* payload, unsigned int length);

	template <class T> static void _sendJsontoHTTP( const T& root, AsyncWebServerRequest *request) ;

	MelvanimateMQTT * _mqtt{nullptr};

	const char * _deviceName{nullptr};
	uint16_t  _pixels;
	uint8_t _pin{DEFAULT_WS2812_PIN};
	bool _settings_changed{false};

	uint8_t _waiting{0};
	uint32_t _waiting_timeout{0};

	int _timerState{-1};
	SimpleTimer _timer;

	AsyncWebServer & _HTTP;

	uint32_t _power{0};
	uint32_t _powertick{0};
	uint32_t _heap{0};
	bool _reInitPixelsAsync{false};

	UDP_broadcast _locator;
	AsyncEventSource * _events{nullptr};

#ifdef RANDOM_MANIFEST_BOOT
	uint32_t _random_number{0};
#endif

};
