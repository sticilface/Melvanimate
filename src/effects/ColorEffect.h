

#ifndef __COLOUR_EFFECT
#define __COLOUR_EFFECT

#include "EffectHandler.h"



class ColorEffect : public EffectHandler {
public:

	bool InitVars();

	bool Start() override;
	bool Stop() override;
	bool Run() override;

	void Refresh() override;
	void Draw();

	inline RgbColor color() { return getVar<RgbColor>("color1"); }
	inline uint8_t brightness() { return getVar<uint8_t>("brightness"); }
	inline Palette* palette() { return getVar<Palette*>("Palette"); }

private:

	RgbColor _actual_color;
  RgbColor _nextColor; 

};



#endif /* end of include guard: __COLOUR_EFFECT */
