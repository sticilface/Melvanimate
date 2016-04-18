#pragma once

#include "EffectHandler.h"
#include "Melvtrix.h"
#include "ObjectManager.h"

using namespace std::placeholders;


class Shapes : public EffectHandler
{

public:

	typedef std::function<void(SimpleEffectObject *)> ShapeCallback;
	enum Shapetype {RANDOM = 0, FILLCIRCLE, DRAWCIRCLE, FILLSQUARE, DRAWSQUARE, FILLTRIANGLE, DRAWTRIANGLE };


	Shapes() {};
	bool InitVars();
	bool Run() override;
	bool Start() override;
	bool Stop() override;
	void Refresh()
	{
		Start();
	}

	void setshape(Shapes::ShapeCallback Fn) { _shape = Fn; }
	void setshape(Shapetype shape); 

	void shape(SimpleEffectObject * Object);


	inline uint8_t speed() { return getVar<uint8_t>("speed"); }
	inline uint8_t brightness() { return getVar<uint8_t>("brightness"); }
	inline RgbColor color() { return getVar<RgbColor>("color1"); }
	inline Melvtrix*  matrix() { return getVar<MelvtrixMan*>("Matrix")->getMatrix(); }
	inline MelvtrixMan*  matrixMan() { return getVar<MelvtrixMan*>("Matrix"); }
	inline bool usematrix() { return getVar<bool>("use_matrix");}
	inline Palette* palette() { return getVar<Palette*>("Palette"); }
	inline uint8_t effectnumber() { return getVar<uint8_t>("effectnumber"); }
	inline uint8_t size() { return map(getVar<uint8_t>("size"), 0, 255, 1, 20); }
	inline Shapetype shapemode() { return (Shapetype)getVar<uint8_t>("shapemode"); }

private:

	void fillCircle(SimpleEffectObject * Object) ;
	void drawCircle(SimpleEffectObject * Object) ;
	void drawRect(SimpleEffectObject * Object) ;
	void fillRect(SimpleEffectObject * Object) ;
	void fillTriangle(SimpleEffectObject * Object);
	void drawTriangle(SimpleEffectObject * Object); 



	struct ShapesVars {
		uint32_t position{0};
		uint32_t lasttick{0};
		EffectGroup* manager{nullptr};
		EffectObjectHandler ** pGroup{nullptr};
	};

	ShapesVars * _vars{nullptr};
	ShapeCallback  _shape; // {nullptr};

};



