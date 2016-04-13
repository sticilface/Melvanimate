#include "EQ.h"

#define EQFILTER 80

EQ::~EQ()
{
	_deinitialise();
};


void EQ::StartEQ()
{
	pinMode(_strobePin, OUTPUT);
	pinMode(_resetPin, OUTPUT);
	digitalWrite(_resetPin, LOW);
	digitalWrite(_strobePin, HIGH);
}

void EQ::GetEQ(uint16_t * data)
{
	digitalWrite(_resetPin, HIGH);
	digitalWrite(_resetPin, LOW);
	digitalWrite(_strobePin, LOW);

	for (int i = 0; i < 7; i++) {
		data[i] = analogRead(0);  // the time taken is for the analogue read....
		digitalWrite(_strobePin, HIGH);
		digitalWrite(_strobePin, LOW);
	}
}

void EQ::EndEQ()
{
	pinMode(_strobePin, INPUT);
	pinMode(_resetPin, INPUT);
}

void EQ::loop()
{

	if (_enabled && millis() - _tick > _freq) {

		//uint16_t data[7];
		GetEQ(data);


		for (uint8_t i = 0; i < 7; i++) {
			data[i] = constrain(data[i], EQFILTER, 1023);
			uint8_t value = map(data[i], EQFILTER, 1023, 0, 255);

			if (_data[i]) {

				EQData_s & current = *_data[i];

				if (value > 100 &&
				        value > (current.average() * _peakfactor) &&
				        millis() - current.lastbeat > _beatskiptime
				   ) {
					EQParam params;
					params.channel = i;
					params.average = current.average();
					params.level = value;
					params.bpm = ( 60000 / millis() - current.lastbeat );
					EQcallbackFN(params);
					current.lastbeat = millis();
				}

				current.add(value);
			}
		}
		_tick = millis();
	}

}

void EQ::_deinitialise()
{
	DebugEQf("[EQ::_deinitialise] Called\n");
	if (_data) {
		for (uint8_t i = 0; i < 7; i++) {
			if (_data[i]) {
				delete _data[i];
				_data[i] = nullptr;
			}
		}
		delete[] _data;
		_data = nullptr;
	}

	EndEQ();
}

void EQ::Initialise(uint32_t samples, uint32_t totaltime)
{
	_samples = samples;
	_sampletime = totaltime;
	_freq = totaltime / samples ;
	DebugEQf("[EQ::initialise] Called %u, %ums, freq = %u\n", samples, totaltime, _freq);

	_deinitialise();
	uint32_t startheap = ESP.getFreeHeap() ;
	_data = new EQData_s*[7];

	for (uint8_t i = 0; i < 7; i++) {
		_data[i] = new EQData_s(samples);
	}

	DebugEQf("[EQ::initialise] END heap used = %u\n", startheap - ESP.getFreeHeap() );

	StartEQ();

}

bool EQ::addJson(JsonObject & root)
{

	JsonObject& EQjson = root.createNestedObject("EQ");
	EQjson["enablebeats"] = _enabled;
	EQjson["resetpin"] = _resetPin;
	EQjson["strobepin"] = _strobePin;
	EQjson["peakfactor"] = _peakfactor;
	EQjson["beatskiptime"] = _beatskiptime;

	EQjson["samples"] = _samples;
	EQjson["sampletime"] = _sampletime;



}

bool EQ::parseJson(JsonObject & root)
{
	bool changed = false;

	if (!root.containsKey("EQ") ) {
		return false;
	}

	JsonObject& EQjson = root["EQ"];

	if (EQjson.containsKey("enablebeats")) {
		if (_enabled != EQjson["enablebeats"]) {
			_enabled = EQjson["enablebeats"];
			changed = true;
		}
	}

	if (_enabled) {

		if (EQjson.containsKey("resetpin")) {
			if (_resetPin != EQjson["resetpin"]) {
				_resetPin = EQjson["resetpin"];
				changed = true;
			}
		}

		if (EQjson.containsKey("strobepin")) {
			if (_strobePin != EQjson["strobepin"]) {
				_strobePin = EQjson["strobepin"];
				changed = true;
			}
		}

		if (EQjson.containsKey("peakfactor")) {
			if (_peakfactor != EQjson["peakfactor"]) {
				_peakfactor = EQjson["peakfactor"];
				changed = true;
			}
		}

		if (EQjson.containsKey("beatskiptime")) {
			if (_beatskiptime != EQjson["beatskiptime"]) {
				_beatskiptime = EQjson["beatskiptime"];
				changed = true;
			}
		}

		if (EQjson.containsKey("samples") &&  EQjson.containsKey("sampletime")  ) {
			if (_samples != EQjson["samples"] ||  _sampletime != EQjson["sampletime"]) {

				_samples = EQjson["samples"];
				_sampletime = EQjson["sampletime"];
				//Initialise(_samples, _sampletime);
				changed = true;
			}
		}

	}

	if (changed && _enabled) {
		Initialise(_samples, _sampletime);
	} else {
		_deinitialise();
	}

#ifdef DebugEQ
	DebugEQf("EQ Json\n");
	EQjson.prettyPrintTo(Serial);
	DebugEQf("\n");
#endif


}

