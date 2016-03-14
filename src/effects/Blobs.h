#pragma once

#include "EffectHandler.h"
#include "Melvtrix.h"
#include "ObjectManager.h"

class Blobs : public EffectHandler
{

public:
	Blobs() {};

	bool InitVars();
	bool Run() override;
	bool Start() override;
	bool Stop() override;
	void Refresh()
	{
		Start();
	}

	inline uint8_t speed() { return getVar<uint8_t>("speed"); }
	inline uint8_t brightness() { return getVar<uint8_t>("brightness"); }
	inline Melvtrix*  matrix() { return getVar<MelvtrixMan*>("Matrix")->getMatrix(); }
	inline MelvtrixMan*  matrixMan() { return getVar<MelvtrixMan*>("Matrix"); }
	inline bool usematrix() { return getVar<bool>("use_matrix");}
	inline Palette* palette() { return getVar<Palette*>("Palette"); }
	inline uint8_t effectnumber() { return getVar<uint8_t>("effectnumber"); }
	inline uint8_t size() { return map(getVar<uint8_t>("size"),0,255,1,20); }

private:

	struct position_s {
		uint16_t x = 0;
		uint16_t y = 0;
	};

	struct BlobsVars {
		uint32_t position{0};
		uint32_t lasttick{0};
		EffectGroup* manager{nullptr};
		EffectObjectHandler ** pGroup{nullptr};
		//position_s * pPosition{nullptr};  


	};

	BlobsVars * _vars{nullptr};

};





