
#pragma once

#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include "Melvanimate.h"
#include "IPAddress.h"
#include <functional>
#include <ArduinoJson.h>


#define DebugMelvanimateMQTT

#ifdef DebugMelvanimateMQTT
#define DebugMelvanimateMQTTf(...) Serial.printf(__VA_ARGS__)
#else
#define DebugMelvanimateMQTTf(...) {}
#endif

using namespace std::placeholders;

class Melvanimate;

class MelvanimateMQTT
{
public:

	MelvanimateMQTT(Melvanimate * lights, IPAddress Addr, uint16_t Port = 1883) : _melvanimate(lights), _addr(Addr), _port(Port)
	{
		_client.setClient(_espClient);
		_client.setServer(Addr, Port);
		_client.setCallback(  std::bind (&MelvanimateMQTT::_handle, this, _1, _2, _3));
	}

	~MelvanimateMQTT() {}

	void loop();

	operator bool()
	{
		return _client.connected();
	}

	bool publish(const char * topic, const char * payload); 
	bool publish(const char * topic, const char * payload, size_t length); 

	void sendFullJson() { _send_flag = millis(); } 
	void sendPresets();  

	bool addJson(JsonObject & root);
	bool parseJson(JsonObject & root) {}; 


private:

	void _sendFullJson(); 

	void _reconnect();
	void _handle(char* topic, byte* payload, unsigned int length);

	WiFiClient _espClient;
	PubSubClient _client;
	uint32_t _reconnectTimer{0};
	Melvanimate * _melvanimate{nullptr};
	uint32_t _send_flag{0}; 
	IPAddress _addr; 
	uint16_t _port; 

};