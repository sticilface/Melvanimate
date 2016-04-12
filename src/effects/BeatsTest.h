#pragma once

#include "EffectHandler.h"
#include "helperfunc.h"
#include "EQ.h"


class BeatsTest : public EffectHandler
{
public:
	BeatsTest() {};

	//  InitVars is overridden from PropertyManager.  delete is called automagically on all vars created with addVar.
	bool InitVars() override; 

	bool Start() override;
	bool Stop() override;
	void Refresh() override {}
	bool Run() override;



	inline uint8_t brightness() { return getVar<uint8_t>("brightness"); }
	//inline EQ* EQ() { return getVar<EQ*>("EQ"); }


	// inline uint8_t filter() { return getVar<uint8_t>("filter"); }
	// inline float beatsratio() { return getVar<float>("beatsratio"); }
	// inline uint8_t beatstimeout() { return getVar<uint8_t>("beatstimeout"); }



private:
	//uint16_t _spectrumValue[7] = {0};  
    //uint16_t _filter = 80; 
    uint32_t _tick = 0; 
    //uint8_t _colours[7] = {0}; 
    EQ * _EQ{nullptr}; 
};