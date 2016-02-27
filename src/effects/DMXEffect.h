#pragma once

#include "EffectHandler.h"
#include <IPAddress.h>

#include "e131/_E131.h"


class DMXEffect : public EffectHandler
{

public:
	DMXEffect(): _vars(nullptr), _e131(nullptr) {};

	bool Start() override;
	bool Stop() override;
	bool Run() override;
	bool InitVars() override;

	uint8_t  universe() { return getVar<uint8_t>("dmx_universe"); }
	uint8_t  ppu() { return getVar<uint8_t>("dmx_ppu"); }
	uint8_t  channelstart() { return getVar<uint8_t>("dmx_channel_start"); }
	uint16_t port() { return getVar<uint16_t>("dmx_port"); }
	bool usemulticast() { return getVar<bool>("dmx_usemulticast"); }
	IPAddress multicastipaddress() { return getVar<IPAddress>("dmx_multicast_ip_addr"); }

private:

	E131* _e131;


//   static uint8_t         *seqTracker;    /* Current sequence numbers for each Universe */
//   static uint8_t         ppu, uniTotal, universe, channel_start, uniLast;
//   static uint16_t        count, bounds ;
//   static uint32_t        *seqError;      /* Sequence error tracking for each universe */
//   static uint32_t timeout_data = 0;


	struct DMXEffectVars {
		uint8_t*  seqTracker{nullptr};
		uint8_t   uniTotal{0}, uniLast{0};
		uint16_t  count{0}, bounds{0};
		uint32_t* seqError{nullptr};
		uint32_t  timeoutvar{0};

	};

	DMXEffectVars * _vars;

};