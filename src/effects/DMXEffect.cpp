#include "DMXEffect.h"
#include "NeopixelBus.h"

extern NeoPixelBus * strip;


bool DMXEffect::InitVars()
{
	if (_vars) { delete _vars;}

	_vars = new DMXEffectVars;

	if (_e131) {
		delete _e131;
		_e131 = nullptr;
	}

	_e131 = new E131;

	addVar(new Variable<uint8_t>("dmx_universe",1));
	addVar(new Variable<uint8_t>("dmx_ppu",1));
	addVar(new Variable<uint8_t>("dmx_channel_start",1));

	// networking
	addVar(new Variable<uint16_t>("dmx_port", 8888));
	addVar(new Variable<bool>("dmx_usemulticast",true));
	//setVar<bool>("dmx_usemulticast", true);
	addVar(new Variable<IPAddress>("dmx_multicast_ip_addr", IPAddress(224, 0, 0, 0)));
	///setVar<IPAddress>("dmx_multicast_ip_addr", IPAddress(224, 0, 0, 0) );

}



bool DMXEffect::Start()
{

//return 1; 

	if (_e131 && _vars) {
		//    if (millis() > 30000) Adalight_Flash();

		_vars->count = strip->PixelCount() * 3;

		_vars->bounds = ppu() * 3;
		if (_vars->count % _vars->bounds) {
			_vars->uniLast = universe() + _vars->count / _vars->bounds;
		} else {
			_vars->uniLast = universe() + _vars->count / _vars->bounds - 1;
		}

		_vars->uniTotal = (_vars->uniLast + 1) - universe();

		if (_vars->seqTracker) { free(_vars->seqTracker); }
		if ((_vars->seqTracker = (uint8_t *)malloc(_vars->uniTotal))) {
			memset(_vars->seqTracker, 0x00, _vars->uniTotal);
		}

		if (_vars->seqError) { free(_vars->seqError); }
		if ((_vars->seqError = (uint32_t *)malloc(_vars->uniTotal * 4))) {
			memset(_vars->seqError, 0x00, _vars->uniTotal * 4);
		}

		Serial.printf("Count = %u, bounds = %u, uniLast = %u, uniTotal = %u\n", _vars->count, _vars->bounds, _vars->uniLast, _vars->uniTotal);

		if (usemulticast()) {
			_e131->begin( E131_MULTICAST , universe() ) ; // E131_MULTICAST // universe is optional and only used for Multicast configuration.
			Serial.printf("[DMXEffect::Start] Multicast Started\n");

		} else {
			_e131->begin( E131_UNICAST, universe());
			Serial.printf("[DMXEffect::Start] Unicast Started\n");
		}
	}

}

bool DMXEffect::Run()
{

}

bool DMXEffect::Stop()
{
	if (strip) {
		strip->ClearTo(0);
	}

	if (_e131) {
		delete _e131;
		_e131 = nullptr;
	}

	if (_vars) { delete _vars;}


	if (_vars->seqTracker) { free(_vars->seqTracker); }
	_vars->seqTracker = nullptr;

	if (_vars->seqError) { free(_vars->seqError); }
	_vars->seqError = nullptr;

}

