#pragma once

#include "EffectHandler.h"
#include "Melvtrix.h"
#include "ObjectManager.h"

using namespace std::placeholders;


class Blobs : public EffectHandler
{

public:

	typedef std::function<void(EffectObjectHandler *)> ShapeCallback;

	Blobs() {};
	bool InitVars();
	bool Run() override;
	bool Start() override;
	bool Stop() override;
	void Refresh()
	{
		Start();
	}

	void setshape(Blobs::ShapeCallback Fn) { _shape = Fn; }

	void shape(EffectObjectHandler * Object);


	inline uint8_t speed() { return getVar<uint8_t>("speed"); }
	inline uint8_t brightness() { return getVar<uint8_t>("brightness"); }
	inline Melvtrix*  matrix() { return getVar<MelvtrixMan*>("Matrix")->getMatrix(); }
	inline MelvtrixMan*  matrixMan() { return getVar<MelvtrixMan*>("Matrix"); }
	inline bool usematrix() { return getVar<bool>("use_matrix");}
	inline Palette* palette() { return getVar<Palette*>("Palette"); }
	inline uint8_t effectnumber() { return getVar<uint8_t>("effectnumber"); }
	inline uint8_t size() { return map(getVar<uint8_t>("size"), 0, 255, 1, 20); }

private:

	void fillcircle(EffectObjectHandler * Object) ;
	void drawCircle(EffectObjectHandler * Object) ;
	void drawRect(EffectObjectHandler * Object) ;
	void fillRect(EffectObjectHandler * Object) ;


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
	ShapeCallback  _shape; // {nullptr};

};



// class foo {
// public:
// 	typedef std::function<void(int)> Callback; // define callback
// 	void setcallback(&foo::Callback Fn) { _Callback = Fn; }  //  function used to set which function to call
// 	void runcallback(int a)  // function  that calls what ever function is set by setcallback
// 	{
// 		_Callback(a);
// 	}
// 	void thefunction(int value)  //  one of any number of functions that can be called....
// 	{
// 		Serial.printf("yes %u\n", value);
// 	}

// 	void randomfunction()   //  random function that sets the callback function...
// 	{
// 		setcallback(std::bind(&foo::thefunction, this, _1);
// 	}
// private:
// 	Callback _Callback;  // stores the current function...

// };


