#pragma once

#include "EffectHandler.h"

class RainbowChase : public EffectHandler
{

public:
	RainbowChase(){};

	bool InitVars()
	{
		addVar(new Variable<uint8_t>("brightness", 255));
		addVar(new Variable<uint8_t>("speed", 30));
		addVar(new Variable<bool>("usematrix", false));
		addVar(new Variable<bool>("wholerainbow", false));

		if (_vars) {
			delete _vars; 
		}
		_vars = new RainbowChaseVars; 
	}

	bool Run() override;
	bool Start() override;  
	bool Stop() override;  
	void Refresh() {
		Start(); 
	} 


	inline uint8_t speed() { return getVar<uint8_t>("speed"); }
	inline uint8_t brightness() { return getVar<uint8_t>("brightness"); }
	inline bool usematrix() { return getVar<bool>("usematrix"); }
	inline bool wholerainbow() { return getVar<bool>("wholerainbow"); }
	

	
private:

	struct RainbowChaseVars {
		uint32_t position{0}; 
		uint32_t lasttime{0};
	}; 

	RainbowChaseVars * _vars{nullptr}; 

	void _rainbowCycle();
	void _rainbow(); 

};