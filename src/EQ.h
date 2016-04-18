/*

Provides Methods for MSGEQ7 chip

Checked with OsillyScope and the delays are not required.
MSGEQ7 chip RESET pulse is 0.1us min, strobe pulse 0.018us min.  And it works fine without any delays.



*/


#pragma once
#include <Arduino.h>
#include <functional>
#include "PropertyManager.h"
#include <ArduinoJson.h>

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#define EQ_DEFAULT_STROBE_PIN 13
#define EQ_DEFAULT_RESET_PIN  12

//#define DebugEQ


#ifdef DebugEQ
	#define DebugEQf(...) Serial.printf(__VA_ARGS__)
#else
	#define DebugEQf(...) {}
#endif

struct EQParam {
	uint32_t seq_no; 
	uint8_t channel;
	uint16_t level;
	uint8_t bpm;
	uint8_t average;
};

typedef std::function<void(const EQParam& param)> EQCallback;

enum EQ_MODE { EQ_OFF = 0, EQ_ON, EQ_RECEIVEUDP};


struct EQData_s {
	EQData_s(uint16_t size)
	{
		_size = size;
		_data = new uint8_t[size];
	}
	~EQData_s()
	{
		if (_data) { delete[] _data; }
	}

	void add(uint8_t value)
	{
		if (_data) {
			_sum -= _data[_position];
			_data[_position] = value;
			_sum += _data[_position];
			_position++;
			if (_position == _size) { _position = 0; }
			if (_count < _size) { _count++; }
		}
	};

	uint8_t average()
	{
		if (_count) {
			return _sum / _count;
		}
		return 0;
	}

	// uint8_t max()
	// {
	// 	uint8_t value = 0;
	// 	for (uint32_t i = 0; i < _count; i++ ) {
	// 		if (_data[i] > value) {
	// 			value = _data[i];
	// 		}
	// 	}
	// 	return value;
	// }

	// uint8_t min()
	// {
	// 	uint8_t value = _data[0];

	// 	for (uint32_t i = 0; i < _count; i++ ) {
	// 		if (_data[i] < value) {
	// 			value = _data[i];
	// 		}
	// 	}
	// 	return value;
	// }

	uint32_t lastbeat{0};

private:
	uint32_t _size{0};
	uint32_t _count{0};
	uint32_t _position{0};
	//uint8_t _channel{0};
	uint32_t _sum{0};
	uint8_t * _data{nullptr};



};


class EQ
{
public:


	EQ(uint32_t samples = 0, uint32_t totaltime = 0); 
	~EQ();

	void SetBeatConfig(uint32_t samples, uint32_t totaltime);
	void SetBeatCallback(EQCallback EQcallback) {_EQcallbackFN = EQcallback;};

	void loop();

	void setMode(EQ_MODE mode) { _mode = mode; _initialise(); }

	uint16_t data[7] = {0};


	void GetEQ(uint16_t * data);

	uint32_t seq() { return seq_no; }

	bool addJson(JsonObject & root);
	bool parseJson(JsonObject & root);

private:
	void _initialise();
	void _deinitialise();
	void _sendUDP(const EQParam& param);
	void _endEQ();
	void _startEQ();


	EQ_MODE _mode {EQ_ON}; // default mode is on!

	uint8_t _resetPin = EQ_DEFAULT_RESET_PIN;
	uint8_t _strobePin = EQ_DEFAULT_STROBE_PIN;

	uint32_t _tick{0};

	EQData_s ** _data{nullptr};

	uint32_t _samples = 0;
	uint32_t _sampletime = 0;
	uint32_t _freq = 0;
	EQCallback _EQcallbackFN;

	float _peakfactor{2.1};
	uint8_t _beatskiptime{200};

//  UDP stuff

	IPAddress _addr = IPAddress(224, 0, 0, 0);
	uint16_t _port = 9988;
	WiFiUDP * _udp{nullptr};
	bool _send_udp{false};

	uint32_t seq_no{0}; 

};



template <>
class Variable<EQ*>: public AbstractPropertyHandler
{
public:
	Variable(uint32_t samples = 0, uint32_t totaltime = 0)
	{
		_name = "EQ";
		_var = new EQ(samples, totaltime);
	};
	~Variable() override
	{
		if (_var) {
			delete _var;
			_var = nullptr;
		}
	}

	EQ * get() { return _var; }

	void set(EQ * value) { _var = value; }

	bool addJsonProperty(JsonObject & root, bool onlychanged = false) override
	{
		if (onlychanged && !_changed) { return false; }
		return _var->addJson(root);
	}

	bool parseJsonProperty(JsonObject & root) override
	{
		if (root.containsKey(_name)) {
			_changed = _var->parseJson(root);
			return _changed;
		} else {
			return false;
		}
	}

private:
	EQ * _var{nullptr};
};

