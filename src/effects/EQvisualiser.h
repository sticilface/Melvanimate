#pragma once

#include "EffectHandler.h"
#include "helperfunc.h"
#include "EQ.h"


class EQvisualiser : public EffectHandler, public EQ
{
public:
	EQvisualiser() {};

	//  InitVars is overridden from PropertyManager.  delete is called automagically on all vars created with addVar.
	bool InitVars() override; 

	bool Start() override;
	bool Stop() override;
	void Refresh() override {}
	bool Run() override;



	inline uint8_t brightness() { return getVar<uint8_t>("brightness"); }

private:
	uint16_t _spectrumValue[7] = {0};  
    uint16_t _filter = 80; 
    uint32_t _tick = 0; 
    uint8_t _colours[7] = {0}; 
};