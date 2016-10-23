#pragma once

#include "EffectHandler.h"
#include "helperfunc.h"
#include "EQ.h"
#include <NeoPixelAnimator.h>


class Beats : public EffectHandler
{
public:
	Beats() {};

	//  InitVars is overridden from PropertyManager.  delete is called automagically on all vars created with addVar.
	bool InitVars() override; 

	bool Start() override;
	bool Stop() override;
	void Refresh() override { Start(); }
	bool Run() override;


	inline uint8_t brightness() { return getVar<uint8_t>("brightness"); }

	inline uint8_t mode() { return getVar<uint8_t>("BeatMode"); }


	inline Palette* palette() { return getVar<Palette*>("Palette"); }


	// inline uint8_t filter() { return getVar<uint8_t>("filter"); }
	// inline float beatsratio() { return getVar<float>("beatsratio"); }
	// inline uint8_t beatstimeout() { return getVar<uint8_t>("beatstimeout"); }

	void effect1(EQParam params);
	void effect2(EQParam params);
	void bassEffect(EQParam params); 
	void snakeEffectCb(EQParam params); 

	void snakeEffectRun();

private:
	//uint16_t _spectrumValue[7] = {0};  
    uint16_t _pixels = 1; 
    uint32_t _tick = 0; 
    //uint8_t _colours[7] = {0}; 
    EQ * _EQ{nullptr}; 
    RgbColor _currentColor; 
    uint32_t _colortimeout{0}; 
    uint32_t _beattimeout{0}; 
    uint16_t _position{0}; 
    int8_t _direction{1}; 
    uint8_t _speed{30}; 
    uint8_t _tail{5};
    //float _pixelfudgefactor{1}; 
    AnimUpdateCallback _animUpdate1; 

};