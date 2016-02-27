#pragma once

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include "EffectHandler.h"

class UDPEffect : public EffectHandler
{

public:
	UDPEffect() : _vars(nullptr) {};

	bool Start() override;
	bool Stop() override;
	void Refresh() override
	{
		Start();
	}
	bool Run() override;

	//  InitVars is overridden from PropertyManager.  delete is called automagically on all vars created with addVar.
	bool InitVars() override
	{
		if (_vars) { delete _vars;} 
		_udp = new WiFiUDP; 
		_vars = new UDPEffectVars; 
		addVar(new Variable<uint16_t>("udp_port", 8888));
		addVar(new Variable<bool>("udp_usemulticast", true)); 
		addVar(new Variable<IPAddress>("udp_multicast_ip_addr", IPAddress(224, 0, 0, 0)));
		
	}

	inline uint16_t port()  {  return getVar<uint16_t>("UDPlightPort"); }
	inline bool usemulticast() { return getVar<bool>("udp_usemulticast");}
	inline IPAddress multicastaddress() { return getVar<IPAddress>("udp_multicast_ip_addr") ;}


	static void Adalight_Flash();

private:

	struct UDPEffectVars {
		uint32_t timeoutvar{0};

	};

	UDPEffectVars * _vars; 
	WiFiUDP* _udp;
	IPAddress address; 

};