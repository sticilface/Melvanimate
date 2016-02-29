#include "UDPEffect.h"

#include "NeopixelBus.h"

extern NeoPixelBus * strip;
extern NeoPixelAnimator * animator;

bool UDPEffect::Run()
{
	int packetSize;

	if (!_udp || !strip || !_vars) {
		Serial.println("ERROR"); 
		return 0;
	}

	packetSize = _udp->parsePacket();

	if  (_udp->available())  {
		for (int i = 0; i < packetSize; i = i + 3) {
			if (i > strip->PixelCount() * 3) { break; }         // Stops reading if LED count is reached.
			strip->Pixels()[i + 1] = _udp->read();   // direct buffer is GRB,
			strip->Pixels()[i]     = _udp->read();
			strip->Pixels()[i + 2] = _udp->read();
		}
		_udp->flush();
		strip->Dirty();
		strip->Show();
		_vars->timeoutvar = millis();
	}

	if (millis() - _vars->timeoutvar > 5000)  {
		strip->ClearTo(0, 0, 0);
		_vars->timeoutvar = millis();
	}

	return true;

}

bool UDPEffect::Start()
{
	if (strip)
	{
		strip->ClearTo(0);
	}
	
	if (_udp) {
		if (*_udp) {
			_udp->stop();
		}

		if (usemulticast()) {
//			Serial.printf("[UDPEffect::Start] beginMulticast port = %u\n", port());
			_udp->beginMulticast(WiFi.localIP(), multicastaddress(), port() );
		} else {
//			Serial.printf("[UDPEffect::Start] beginUnicast port = %u\n", port());
			_udp->begin(port());
		}
	}

}


bool UDPEffect::Stop()
{
	if (strip) {
		strip->ClearTo(0);
	}
	
	if (_udp) {
		_udp->stop();
		delete _udp;
		_udp = nullptr;
	}

	if (_vars) {
		delete _vars;
		_vars = nullptr;
	}

	if (animator) {
		animator->FadeTo(250, 0);
	}

}