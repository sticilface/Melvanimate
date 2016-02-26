#pragma once

class AdalightEffect : public SwitchEffect
{

public:
	AdalightEffect(EffectHandlerFunction Fn);

	//  InitVars is overridden from PropertyManager.  delete is called automatically on all vars created with addVar.
	bool InitVars() override
	{
		uint32_t heap = ESP.getFreeHeap();
		addVar(new Variable<int>("serialspeed"));
	}

	int serialspeed()  {  return getVar<int>("serialspeed"); }

};