#pragma once

#include <functional>
#include "EffectHandler.h"

/* ------------------------------------------------------------------------
					Effect Handler SWITCH - MAIN effect handler....
					Handles most of my effects... (required)
--------------------------------------------------------------------------*/
enum effectState { PRE_EFFECT = 0, RUN_EFFECT, POST_EFFECT, EFFECT_PAUSED, EFFECT_REFRESH };


class SwitchEffect : public EffectHandler
{

public:
	typedef std::function<void(effectState&, EffectHandler* )> EffectHandlerFunction;
	SwitchEffect(EffectHandlerFunction Fn) : _Fn(Fn) {};
	bool Run() override;
	virtual bool Start() override { _state = PRE_EFFECT ; _Fn(_state, this) ; _state = RUN_EFFECT; }
	virtual bool Stop() override { _state = POST_EFFECT; _Fn(_state, this); };
	virtual void SetTimeout (uint32_t timeout) { _timeout = timeout; };
	virtual void Refresh();


private:
	EffectHandlerFunction _Fn;
	effectState _state;
	uint32_t _lastTick = 0;
	uint32_t _timeout = 0;
};