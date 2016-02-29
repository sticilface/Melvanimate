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
	void Refresh() override 
	{
		Start();
	}


	inline uint16_t bin() { return getVar<uint16_t>("dmx_bin"); }
	inline uint8_t  universe() { return getVar<uint8_t>("dmx_universe"); }
	inline uint8_t  ppu() { return getVar<uint8_t>("dmx_ppu"); }
	inline uint8_t  channelstart() { return getVar<uint8_t>("dmx_channel_start"); }
	inline uint16_t port() { return getVar<uint16_t>("dmx_port"); }
	inline bool usemulticast() { return getVar<bool>("dmx_usemulticast"); }
	inline IPAddress multicastipaddress() { return getVar<IPAddress>("dmx_multicast_ip_addr"); }

private:

	E131* _e131;

	struct DMXEffectVars {
		uint8_t*  seqTracker{nullptr};
		uint8_t   uniTotal{0}, uniLast{0};
		uint16_t  count{0}, bounds{0};
		uint32_t* seqError{nullptr};
		uint32_t  timeoutvar{0};
	};

	DMXEffectVars * _vars;

};