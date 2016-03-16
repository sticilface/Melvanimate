#pragma once

#include "EffectHandler.h"
#include "Melvtrix.h"
#include "ObjectManager.h"

using namespace std::placeholders;


class Snakes : public EffectHandler
{

public:

	typedef std::function<void(EffectObjectHandler *)> ShapeCallback;
	enum Shapetype {RANDOM = 0, FILLCIRCLE, DRAWCIRCLE, FILLSQUARE, DRAWSQUARE, FILLTRIANGLE, DRAWTRIANGLE };


	Snakes() {};
	bool InitVars();
	bool Run() override;
	bool Start() override;
	bool Stop() override;
	void Refresh()
	{
		Start();
	}

	void setshape(Snakes::ShapeCallback Fn) { _shape = Fn; }
	void setshape(Shapetype shape); 

	void shape(EffectObjectHandler * Object);


	inline uint8_t speed() { return getVar<uint8_t>("speed"); }
	inline uint8_t brightness() { return getVar<uint8_t>("brightness"); }
	inline Melvtrix*  matrix() { return getVar<MelvtrixMan*>("Matrix")->getMatrix(); }
	inline MelvtrixMan*  matrixMan() { return getVar<MelvtrixMan*>("Matrix"); }
	inline bool usematrix() { return getVar<bool>("use_matrix");}
	inline Palette* palette() { return getVar<Palette*>("Palette"); }
	inline uint8_t effectnumber() { return getVar<uint8_t>("effectnumber"); }
	inline uint8_t size() { return map(getVar<uint8_t>("size"), 0, 255, 1, 20); }
	inline Shapetype shapemode() { return (Shapetype)getVar<uint8_t>("shapemode"); }

private:

	void fillCircle(EffectObjectHandler * Object) ;
	void drawCircle(EffectObjectHandler * Object) ;
	void drawRect(EffectObjectHandler * Object) ;
	void fillRect(EffectObjectHandler * Object) ;
	void fillTriangle(EffectObjectHandler * Object);
	void drawTriangle(EffectObjectHandler * Object); 

	struct position_s {
		uint16_t x = 0;
		uint16_t y = 0;
	};

	struct SnakesVars {
		uint32_t position{0};
		uint32_t lasttick{0};
		EffectGroup* manager{nullptr};
		EffectObjectHandler ** pGroup{nullptr};
		//position_s * pPosition{nullptr};



	};

	SnakesVars * _vars{nullptr};
	ShapeCallback  _shape; // {nullptr};

};



