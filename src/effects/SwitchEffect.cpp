#include "SwitchEffect.h"


bool SwitchEffect::Run()
{
	if (_state == PRE_EFFECT) {
		_Fn(_state, this);
		_state = RUN_EFFECT;
		return true;
	}

	if (_state == RUN_EFFECT) {
		if (millis() - _lastTick > _timeout) {
			_Fn(_state, this);
			_lastTick = millis();
		}
	}
};

void SwitchEffect::Refresh()
{
	_state = EFFECT_REFRESH;
	_Fn(_state, this) ;
	if ( _state == EFFECT_REFRESH ) { _state = RUN_EFFECT; }
}