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

#define DebugEQ

#ifdef DebugEQ
#define DebugEQf(...) Serial.printf(__VA_ARGS__)
#else
#define DebugEQf(...) {}
#endif

struct EQParam {
	uint8_t channel;
	uint16_t level;
	uint8_t bpm;
	uint8_t average;
};

typedef std::function<void(const EQParam& param)> EQCallback;

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
	EQ() {};
	~EQ();

	//void DetectBeats(bool value) { _detectbeats = value;}
	void Initialise(uint32_t samples, uint32_t totaltime);
	void SetBeatCallback(EQCallback EQcallback) {EQcallbackFN = EQcallback;};

	void loop();

	void Start() { _enabled = true; }
	void Stop() { _enabled = false; }
	void End() { Stop(); _deinitialise(); }
	
	uint16_t data[7] = {0};


	void GetEQ(uint16_t * data);
	void EndEQ();
	void StartEQ();

	bool addJson(JsonObject & root);
	bool parseJson(JsonObject & root); 

private:
	void _deinitialise(); 

	bool _enabled {false}; 

	uint8_t _resetPin = 12;
	uint8_t _strobePin = 13;

	uint32_t _tick{0};

	EQData_s ** _data{nullptr};

	uint32_t _samples = 0;
	uint32_t _sampletime = 0; 
	uint32_t _freq = 0;
	EQCallback EQcallbackFN;

	float _peakfactor{2.1};
	uint8_t _beatskiptime{200};

};



template <>
class Variable<EQ*>: public AbstractPropertyHandler
{
public:
	Variable()
	{
		_name = "EQ";
		_var = new EQ;
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

