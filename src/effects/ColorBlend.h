


#pragma once

#include "EffectHandler.h"
#include "Melvtrix.h"
#include "ObjectManager.h"



class ColorBlend : public EffectHandler {
public:

	bool InitVars();

	bool Start() override;
	bool Stop() override;
	bool Run() override;

	void Refresh() override; 
	void Draw();


	inline RgbColor color1() { return getVar<RgbColor>("color1"); }
	inline RgbColor color2() { return getVar<RgbColor>("color2"); }
	inline uint8_t brightness() { return getVar<uint8_t>("brightness"); }
	inline uint8_t blendMode() { return getVar<uint8_t> ("blendmode"); }
	inline uint32_t speed() { return map(getVar<uint8_t> ("speed"), 0, 255, 0, 60000); }

	inline Melvtrix*  matrix() { return getVar<MelvtrixMan*>("Matrix")->getMatrix(); }
	inline MelvtrixMan*  matrixMan() { return getVar<MelvtrixMan*>("Matrix"); }
	inline bool usematrix() { return getVar<bool>("use_matrix");}
	inline Palette* palette() { return getVar<Palette*>("Palette"); }

private:

	uint16_t _pixels{0};
	RgbColor c1, c2;
	uint32_t _timeout{0};


};