#include "DMXEffect.h"
#include "NeopixelBus.h"
#include "mybus.h"


extern MyPixelBus * strip;


bool DMXEffect::InitVars()
{
	Serial.println("[DMXEffect::InitVars] Called\n"); 
	if (_vars) { delete _vars;}

	_vars = new DMXEffectVars;

	if (_e131) {
		delete _e131;
		_e131 = nullptr;
	}

	_e131 = new E131;

	addVar(new Variable<uint16_t>("dmx_bin", 8));
	addVar(new Variable<uint8_t>("dmx_universe", 1));
	addVar(new Variable<uint8_t>("dmx_ppu", 1));
	addVar(new Variable<uint8_t>("dmx_channel_start", 1));

	// networking
	addVar(new Variable<uint16_t>("dmx_port", 5568));
	addVar(new Variable<bool>("dmx_usemulticast", true));
	addVar(new Variable<IPAddress>("dmx_multicast_ip_addr", IPAddress(224, 0, 0, 0)));
}



bool DMXEffect::Start()
{

	if (_e131 && _vars && strip) {
		//    if (millis() > 30000) Adalight_Flash();
		strip->ClearTo(0);

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

//		Serial.printf("Count = %u, bounds = %u, uniLast = %u, uniTotal = %u\n", _vars->count, _vars->bounds, _vars->uniLast, _vars->uniTotal);

		// set the port before calling begin..  default 5568
		_e131->setport(port());

		if (usemulticast()) {
			_e131->begin( E131_MULTICAST , universe() ) ; // E131_MULTICAST // universe is optional and only used for Multicast configuration.
//			Serial.printf("[DMXEffect::Start] Multicast Started\n");
			IPAddress add = IPAddress(239, 255, ((universe() >> 8) & 0xff), ((universe() >> 0) & 0xff));
			setVar<IPAddress>("dmx_multicast_ip_addr",  add);

		} else {
			_e131->begin( E131_UNICAST, universe());
//			Serial.printf("[DMXEffect::Start] Unicast Started\n");
		}

		_vars->bin = getVar<uint16_t>("dmx_bin"); 
		_vars->universe = getVar<uint8_t>("dmx_universe");
		_vars->ppu = getVar<uint8_t>("dmx_ppu");

	}

}

bool DMXEffect::Run()
{
	if (_e131 && _vars && strip) {
		if (_e131->parsePacket()) {
			if ((_e131->universe >= universe()) && (universe() <= _vars->uniLast)) {
				/* Universe offset and sequence tracking */
				uint8_t uniOffset = (_e131->universe - universe());

				if (_e131->packet->sequence_number != _vars->seqTracker[uniOffset]++) {
					_vars->seqError[uniOffset]++;
					_vars->seqTracker[uniOffset] = _e131->packet->sequence_number + 1;
				}

				//Find out starting pixel based off the Universe
				uint16_t pixelStart = uniOffset * ppu();

				/* Calculate how many pixels we need from this buffer */
				uint16_t pixelStop = strip->PixelCount() / bin(); 

				// if ((pixelStart + ppu()) < pixelStop) {
				// 	pixelStop = pixelStart + ppu();
				// }

				/* Offset the channel if required for the first universe */
				uint16_t offset = 0;
				if (_e131->universe == universe()) {
					offset = channelstart() - 1;
				}

				/* Set the pixel data */
				uint16_t buffloc = 0;


				for (uint16_t i = pixelStart; i < pixelStop; i++) {

					uint16_t j = buffloc++ * 3 + offset;

					//pixels.setPixelColor(i, e131.data[j], e131.data[j+1], e131.data[j+2]);
					for (uint8_t k = 0; k < bin(); k++) {

						uint16_t pixel = ( bin() * i ) + k;

					//	if (pixel > strip->PixelCount()) { break; }

						strip->SetPixelColor( pixel, RgbColor(_e131->data[j], _e131->data[j + 1], _e131->data[j + 2]));
						//Serial.printf("[%u]%u\n",i,pixel);
					}

				}

				/* Refresh when last universe shows up  or within 10ms if missed */
				if ((_e131->universe == _vars->uniLast) || (millis() - _vars->timeoutvar > 10)) {
					//if (e131.universe == uniLast) {
					//if (millis() - lastPacket > 25) {
					_vars->timeoutvar = millis();
					if (strip) {
						strip->Show();
					}

				}
			}
		}
	}
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

	if (_vars->seqTracker) { free(_vars->seqTracker); }
	_vars->seqTracker = nullptr;

	if (_vars->seqError) { free(_vars->seqError); }
	_vars->seqError = nullptr;

	if (_vars) {
		delete _vars;
		_vars = nullptr;
	}


}

