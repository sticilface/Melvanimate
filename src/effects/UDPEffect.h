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
		_udp = new WiFiUDP; 
		_vars = new UDPEffectVars; 
		addVar(new Variable<int>("port"));
		addVar(new Variable<bool>("Multicast")); 
		addVar(new Variable<IPAddress>("multicastIP"));
		
	}

	int port()  {  return getVar<int>("port"); }

	static void Adalight_Flash();

private:

	struct UDPEffectVars {

	};

	UDPEffectVars * _vars; 
	WiFiUDP* _udp;
	IPAddress address; 

};