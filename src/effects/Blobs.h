#pragma once

#include "EffectHandler.h"
#include "Melvtrix.h"

class Blobs : public EffectHandler
{

public:
	Blobs(){};

	bool InitVars()
	{
		addVar(new Variable<uint8_t>("brightness", 100));
		addVar(new Variable<uint8_t>("speed", 30));
		addVar(new Variable<MelvtrixMan*>("Matrix"));  // must be called Matrix.  very importnat... 
		addVar(new Variable<Palette*>("Palette"));
		
		//addVar(new Variable<bool>("wholerainbow", false));
		if (_vars) {
			delete _vars; 
		}
		_vars = new BlobsVars; 
	}

	bool Run() override;
	bool Start() override;  
	bool Stop() override;  
	void Refresh() {
		Start(); 
	} 

	inline uint8_t speed() { return getVar<uint8_t>("speed"); }
	inline uint8_t brightness() { return getVar<uint8_t>("brightness"); }
	inline MelvtrixMan*  matrix() { return getVar<MelvtrixMan*>("Matrix"); }	
	inline Palette* palette() { return getVar<Palette*>("Palette"); }
	
private:

	struct BlobsVars {
		uint32_t position{0}; 
		uint32_t lasttime{0};
	}; 

	BlobsVars * _vars{nullptr}; 

};