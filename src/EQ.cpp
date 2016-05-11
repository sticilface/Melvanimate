#include "EQ.h"

#define EQFILTER 80


/*
  virtual int beginPacketMulticast(IPAddress multicastAddress,
                                   uint16_t port,
                                   IPAddress interfaceAddress,
                                   int ttl = 1);
  // Finish off this packet and send it
  // Returns 1 if the packet was sent successfully, 0 if there was an error
  virtual int endPacket();
  // Write a single byte into the packet
  virtual size_t write(uint8_t);
  // Write size bytes from buffer into the packet
  virtual size_t write(const uint8_t *buffer, size_t size);
  */
EQ::EQ(uint32_t samples, uint32_t totaltime): _samples(samples), _sampletime(totaltime)
{
	if (_samples && _sampletime) {
		_freq = _sampletime / _samples ;
	} else {
		_freq = 0;
	}
	_initialise();
};

EQ::~EQ()
{
	_deinitialise();
};

void EQ::_initialise()
{
	DebugEQf("[EQ::initialise] _mode = %u\n", _mode);
	_deinitialise();


	switch (_mode) {

	case EQ_ON: {


		uint32_t startheap = ESP.getFreeHeap() ;
		if (_samples) {
			_data = new EQData_s*[7];
			if (_data) {
				for (uint8_t i = 0; i < 7; i++) {
					_data[i] = new EQData_s(_samples);
				}
			}
		}

		if (_send_udp) {
			DebugEQf("[EQ::initialise] WiFiUDP SEND Service Started\n");

			if (!_udp) {
				_udp = new WiFiUDP;
			}
		}
		DebugEQf("[EQ::initialise] END heap used = %u\n", startheap - ESP.getFreeHeap() );

		_startEQ();
	}
	break;
	case EQ_RECEIVEUDP: {

		if (!_udp) {
			_udp = new WiFiUDP;
		}

		uint8_t code = _udp->beginMulticast(WiFi.localIP(), _addr, _port);

		DebugEQf("[EQ::initialise] UDP beginMulticast code: %u\n", code );

	}


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

	if (_udp) {
		delete _udp;
		_udp = nullptr;
	}

	_endEQ();
}


void EQ::_startEQ()
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

void EQ::_endEQ()
{
	pinMode(_strobePin, INPUT);
	pinMode(_resetPin, INPUT);
}

void EQ::loop()
{

	if (_mode == EQ_ON && millis() - _tick > _freq) {

		//uint16_t data[7];
		GetEQ(data);

		if (_data) {
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
						params.seq_no = seq_no++; 
						_EQcallbackFN(params);
						_sendUDP(params);
						current.lastbeat = millis();
					}

					current.add(value);
				}
			}
		}
		_tick = millis();

	} else if (_mode == EQ_RECEIVEUDP) {
		//  recieve UDP Packets if mode set
		if (_udp) {
			
			size_t packetSize = _udp->parsePacket();

			if (packetSize == sizeof(EQParam)) {
				seq_no++; 
				EQParam params;
				_udp->read( (unsigned char*)&params,  sizeof(EQParam)  );
				_EQcallbackFN(params);

			} else if (packetSize) {
				DebugEQf("[EQ::loop] Packet Wrong size: is %u, expecting %u\n", packetSize, sizeof(EQParam));
				_udp->flush(); 
			}
		}


	}

}

void EQ::_sendUDP(const EQParam& param)
{
	if (!_udp) {
		return;
	}

	if (!_send_udp) {
		return;
	}

	if (_udp->beginPacketMulticast(_addr, _port, WiFi.localIP() )) {
		size_t len = sizeof(param);
		_udp->write( (const char *)&param, len);
		if (_udp->endPacket()) {
			DebugEQf("[EQ::_sendUDP] Packet Sent\n");
		}
	}
}




void EQ::SetBeatConfig(uint32_t samples, uint32_t totaltime)
{
	_samples = samples;
	_sampletime = totaltime;

	if (_samples && _sampletime) {
		_freq = totaltime / samples ;
	} else {
		_freq = 0;
	}

	DebugEQf("[EQ::initialise] Called %u, %ums, freq = %u\n", _samples, _sampletime, _freq);
	_initialise();

}





bool EQ::addJson(JsonObject & root)
{

	JsonObject& EQjson = root.createNestedObject("EQ");
	EQjson["eqmode"] = (uint8_t)_mode;

	if (_mode == EQ_ON) {
		EQjson["resetpin"] = _resetPin;
		EQjson["strobepin"] = _strobePin;
		EQjson["peakfactor"] = _peakfactor;
		EQjson["beatskiptime"] = _beatskiptime;
		EQjson["samples"] = _samples;
		EQjson["sampletime"] = _sampletime;
		EQjson["eq_send_udp"] = _send_udp; 
	}

	if (_mode == EQ_ON || _mode == EQ_RECEIVEUDP) {
		EQjson["eq_port"] = _port;
		JsonArray & ip = EQjson.createNestedArray("eq_addr");
		ip.add(_addr[0]);
		ip.add(_addr[1]);
		ip.add(_addr[2]);
		ip.add(_addr[3]);
	}
	return true;

}

bool EQ::parseJson(JsonObject & root)
{
	bool changed = false;

	if (!root.containsKey("EQ") ) {
		return false;
	}

	JsonObject& EQjson = root["EQ"];

	if (EQjson.containsKey("eqmode")) {

		EQ_MODE temp = (EQ_MODE)EQjson["eqmode"].as<long>();
		DebugEQf("[EQ::parseJson] mode received = %u\n", temp);

		if (_mode != temp) {
			_mode = temp;
			changed = true;
		}
	}

//	if (_mode == ON) {

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

//	}
	/*

		IPAddress addr = IPAddress(224, 0, 0, 0);
		WiFiUdp * udp = nullptr;
		_port = EQjson["port"];
		bool send_udp{false};   EQjson.containsKey("send_udp")

		*/

	if (EQjson.containsKey("eq_send_udp")) {
		if (_send_udp != EQjson["eq_send_udp"]) {
			_send_udp = EQjson["eq_send_udp"];
			changed = true;
		}
	}

	if (EQjson.containsKey("eq_port")) {
		if (_port != EQjson["eq_port"]) {
			_port = EQjson["eq_port"];
			changed = true;
		}
	}

	if (root.containsKey("eq_addr") ) {

		if ( root["eq_addr"].is<JsonArray&>()) {
			JsonArray & IP = root["eq_addr"];
			IPAddress ret;
			for (uint8_t i = 0; i < 4; i++) {
				ret[i] = IP[i];
			}

			if (_addr != ret) {
				_addr = ret;
				changed = true;
			}
		} else {
			const char * input = root["eq_addr"];
			IPAddress temp;
			if (temp.fromString(input)) {
				if (temp != _addr) {
					_addr = temp;
					changed = true;
				}
			}
		}
	}



	if (changed) {

		// if (_mode == OFF) {
		// 	_deinitialise();
		// } else if (_mode == ON || _mode == RECEIVEUDP) {
			_initialise();
		//}
	}

	return changed;


#ifdef DebugEQ
	DebugEQf("EQ Json\n");
	EQjson.prettyPrintTo(Serial);
	DebugEQf("\n");
#endif


}

