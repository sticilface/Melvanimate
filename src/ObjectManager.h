/*

Handler to add / create / run multiple Effect Objects at the same time...




*/

#pragma once
#include "Arduino.h"
#include <NeoPixelAnimator.h>
#include "Melvtrix.h"
#include "EQ.h"
#include <functional>

//#define DebugObjectman

#if defined(DEBUG_ESP_PORT) && defined(DebugObjectman)
#define Debugobjf(...) DEBUG_ESP_PORT.printf(__VA_ARGS__)
//#define Debugobjf(_1, ...) DEBUG_ESP_PORT.printf_P( PSTR(_1), ##__VA_ARGS__) //  this saves around 5K RAM...


#else
#define Debugobjf(...) {}
#endif

class EffectObjectHandler;

typedef std::function<bool()> ObjectUpdateCallback;



class EffectGroup
{
public:
	EffectGroup() : _firstHandle(nullptr), _currentHandle(nullptr), _lastHandle(nullptr) {};
	~EffectGroup();
	EffectObjectHandler* Add(uint16_t i, EffectObjectHandler* Fn);
	void Update();
	bool Inuse(EffectObjectHandler* exclude, uint16_t pixel);
	EffectObjectHandler*  Get(uint16_t x);

private:

	EffectObjectHandler*  _currentHandle;
	EffectObjectHandler*  _firstHandle;
	EffectObjectHandler*  _lastHandle;

};


class EffectObjectHandler
{
public:
	EffectObjectHandler() : _next(nullptr) {}
	virtual ~EffectObjectHandler() {};
	virtual uint16_t * pixels() { }
	virtual bool UpdateObject() { return false; }
	virtual uint16_t total() {};

	EffectObjectHandler* next() { return _next; } //  ASK what is next
	void next (EffectObjectHandler* next) { _next = next; } //  Set what is next
	void id (uint16_t id) { _id = id; }
	const uint16_t id() { return _id; };

private:
	EffectObjectHandler* _next{nullptr};
	uint16_t _id;
	bool _running{false};
protected:


};

// much simpler and Actually in USE.
class SimpleEffectObject : public EffectObjectHandler
{
private:
	ObjectUpdateCallback _ObjUpdate;
	uint16_t * _pixels{nullptr};
	uint16_t _total{0};
	uint32_t _timeout{30};
	uint32_t _lasttick{0};

public:
	SimpleEffectObject() : _ObjUpdate(nullptr)
	{

	};
	~SimpleEffectObject() override
	{
		if (_pixels) {
			delete[] _pixels;
		}
	}
	void create(uint16_t size)
	{
		if (_pixels) {
			delete[] _pixels;
		}
		_pixels = new uint16_t[size];
		_total = size;
	}
	void end()
	{
		if (_pixels) {
			delete[] _pixels;
		}
		_pixels = nullptr;
	}
	uint16_t total() override { return _total; }

	void Timeout(uint32_t timeout) { _timeout = timeout; };

	bool UpdateObject() override
	{
		if (millis() - _lasttick > _timeout ) {
			if (_ObjUpdate) {
				if (_ObjUpdate()) {
					_lasttick = millis();
					_timeout = 0;
					return true;
				} else {
					_lasttick = millis();
					_timeout = 200; //  back off

				}
			}
		}
		return false;
	}

	uint16_t * pixels() override { return _pixels; };


	void SetObjectUpdateCallback(ObjectUpdateCallback Fn)
	{
		_ObjUpdate = Fn;
	}

	int16_t x{0};
	int16_t y{0};
	uint8_t size{0};

};




















// // trying not to use
// class EffectObject : public EffectObjectHandler
// {
// private:
// 	uint16_t * _details;
// 	ObjectUpdateCallback _ObjUpdate = nullptr;
// 	AnimationUpdateCallback _AniUpdate = nullptr;
// 	uint16_t _position = 0;
// 	uint16_t _total;
// public:

// 	enum Direction {UP = 0, DOWN, LEFT, RIGHT};

// 	EffectObject(uint16_t size) : _total(size), _ObjUpdate(nullptr), _AniUpdate(nullptr)
// 	{
// 		_details = new uint16_t[_total];
// 		std::fill_n(_details, _total, 0);  // change...

// 		Debugobjf("Created Object size %u \n", _total);
// 	};
// 	~EffectObject() override
// 	{
// 		Debugobjf("[EffectObject] Deconstructor\n");
// 		delete[] _details;
// 	}

// 	uint16_t total() override { return _total; } ;

// 	void SetObjectUpdateCallback (ObjectUpdateCallback Fn) override;
// 	void SetPixelUpdateCallback(AnimationUpdateCallback Fn) override;

// 	inline void Addpixel(uint16_t p) override
// 	{
// 		uint16_t n = _position++;
// 		if (_position > _total) { return; }
// 		_details[n] = p;
// 	}

// 	inline void Addpixel(uint16_t n, uint16_t p) override
// 	{
// 		if (n > _total) { return; }
// 		_details[n] = p;
// 	}

// 	uint16_t * pixels() override { return _details;};

// //	bool StartAnimations() override;
// 	bool UpdateObject() override;
// 	bool reset() override
// 	{
// 		_position = 0;
// 		for (uint16_t i = 0; i < _total; i++) {
// 			_details[i] = -1;
// 		}
// 	}


// };








// //  need to move tick to class derivative not in object manager.. so that animations can be handled...
// class AnimatedEffectObject : public EffectObjectHandler
// {
// private:
// 	NeoPixelAnimator * animator{nullptr};
// 	ObjectUpdateCallback _ObjUpdate = nullptr;
// 	Melvtrix * _matrix{nullptr};
// 	uint16_t * _pixels{nullptr};
// 	uint16_t _total{0};
// 	bool _constrain{false};

// public:

// 	enum Direction {N = 0, NE , E, SE, S, SW, W, NW} ; //

// 	AnimatedEffectObject(Melvtrix * matrix) : _ObjUpdate(nullptr), _matrix(matrix)
// 	{

// 	};
// 	~AnimatedEffectObject() override
// 	{
// 		if (_pixels) {
// 			delete[] _pixels;
// 		}
// 		if (animator) {
// 			delete animator;
// 		}
// 	}
// 	void create(uint16_t size)
// 	{
// 		if (_pixels) {
// 			delete[] _pixels;
// 		}
// 		_pixels = new uint16_t[size];
// 		animator = new NeoPixelAnimator(size);
// 		_total = size;
// 	}

// 	void SetObjectUpdateCallback(ObjectUpdateCallback Fn) override
// 	{
// 		_ObjUpdate = Fn;
// 	}

// 	uint16_t * pixels() override { return _pixels;};
// 	uint16_t total() override { return _total; }

// 	bool animating()
// 	{
// 		if (animator)
// 		{
// 			return animator->IsAnimating();
// 		}
// 		return false;
// 	}

// 	bool move(Direction dir);
// 	void bounce(bool value) { _constrain = value; }

// 	bool UpdateObject() override
// 	{
// 		bool updateflag = false;
// 		if (millis() - Lasttick() > Timeout() ) {
// 			if (_ObjUpdate) {
// 				if (_ObjUpdate()) {
// 					Timeout(millis());
// 					updateflag = true;
// 				} ;

// 			}
// 		}

// 		if (animator && animator->IsAnimating() ) {
// 			animator->UpdateAnimations();
// 		}

// 		return updateflag;
// 	}


// };
