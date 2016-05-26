
#pragma once

#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include "Melvanimate.h"
#include "IPAddress.h"
#include <functional>
#include <ArduinoJson.h>

//#define DebugMelvanimateMQTT

#if defined(DEBUG_ESP_PORT) && defined(DebugMelvanimateMQTT)
#define DebugMelvanimateMQTTf(...) DEBUG_ESP_PORT.printf(__VA_ARGS__)
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

	//bool publish(const char * topic, const char * payload);
	bool publish(const char * topic, uint8_t * payload, size_t length, bool retained = false );
	bool publish(const char * topic, const char * payload, bool retained = false );

	//void sendFullJson() { _send_flag = millis(); }
	void sendJson(bool onlychanged); //  { _send_changed_flag = millis();  _onlychanged = onlychanged; }
	void sendPresets();

	bool addJson(JsonObject & root);
	bool parseJson(JsonObject & root) {};
	PubSubClient * mqttclient() { return &_client; }
	Melvanimate * pmelvanimate() { return _melvanimate; }

private:

	struct mqtt_message {
		mqtt_message(MelvanimateMQTT * host, const char * topic, uint8_t * msg, size_t plength = 0 , bool retained = false)
			: _host(host), _plength(plength), _retained(retained), _next(nullptr)
		{
			_topic = strdup(topic);

			_msg = new uint8_t[plength + 1];

			if (_msg) {
				memset(_msg, '\0', plength + 1);
				memcpy(_msg, msg, plength);
			}
		};
		~mqtt_message()
		{
			if (_topic) {
				free( (void*)_topic);
			}
			if (_msg) {
				delete[] _msg;
			}
		};
		bool publish();
		const char * topic() const { return _topic; }
		const char * msg() const { return (const char *)_msg; }
		void next(mqtt_message * next) { _next = next; }
		mqtt_message * next() { return _next; }
	private:
		//PubSubClient * _mqttclient{nullptr};
		MelvanimateMQTT * _host{nullptr};
		mqtt_message * _next{nullptr};
		const char * _topic{nullptr};
		uint8_t * _msg{nullptr};
		size_t _plength{0};
		bool _retained;

	};

	void _sendASync();

	//void _sendFullJson();
	//void _sendJson();

	void _reconnect();
	void _handle(char* topic, byte* payload, unsigned int length);

	WiFiClient _espClient;
	PubSubClient _client;
	uint32_t _reconnectTimer{0};
	Melvanimate * _melvanimate{nullptr};
	//uint32_t _send_flag{0};
	//uint32_t _send_changed_flag{0};
	IPAddress _addr;
	uint16_t _port;

	mqtt_message * _firstmessage{nullptr};
	uint32_t _asyncTimeout{0};
	bool _onlychanged{nullptr};
	uint16_t _reconnecttries{0}; 

};