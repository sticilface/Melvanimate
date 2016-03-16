/*

Handler to add / create / run multiple Effect Objects at the same time...




*/

#pragma once
#include <functional>
#include "Arduino.h"
#include <NeoPixelAnimator.h>

//#define DebugObjectman

#ifdef DebugObjectman
#define Debugobjf(...) Serial.printf(__VA_ARGS__)
#else
#define Debugobjf(...) {}
#endif


class EffectObjectHandler;

typedef std::function<bool()> ObjectUpdateCallback;
typedef std::function<void(uint16_t, uint16_t)> AnimationUpdateCallback;


class EffectGroup
{
public:
	EffectGroup() : _firstHandle(nullptr), _currentHandle(nullptr), _lastHandle(nullptr) {};
	~EffectGroup();
	EffectObjectHandler* Add(uint16_t i, uint32_t timeout, EffectObjectHandler* Fn);
	void Update();
	void Run();
	bool Inuse(EffectObjectHandler* exclude, uint16_t pixel);
	EffectObjectHandler*  Get(uint16_t x);
	EffectObjectHandler*  operator[] (uint16_t x)
	{
		return Get(x);
	}

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

	virtual void SetObjectUpdateCallback(ObjectUpdateCallback Fn) {}
	virtual void SetPixelUpdateCallback(AnimationUpdateCallback Fn) {}
	virtual void Addpixel(uint16_t n, uint16_t p) {}
	virtual void Addpixel(uint16_t p) {}

	virtual uint16_t * pixels() { }
	virtual bool StartAnimations() {}
	virtual bool UpdateObject() { return false; }

	virtual uint16_t total() {};

	void Running(bool running) { _running = running; }
	bool Running() { return _running; }
	void Timeout(uint32_t timeout) { _timeout = timeout; };
	uint32_t Timeout() { return _timeout; }
	void Lasttick(uint32_t lasttick) { _lasttick = lasttick; };
	uint32_t Lasttick() { return _lasttick; }

	EffectObjectHandler* next() { return _next; } //  ASK what is next
	void next (EffectObjectHandler* next) { _next = next; } //  Set what is next
	void id (uint16_t id) { _id = id; }
	const uint16_t id() { return _id; };
	bool virtual reset() {}

	uint16_t x{0};
	uint16_t y{0};
	uint8_t size{0}; 

private:
	EffectObjectHandler* _next{nullptr};
	uint16_t _id;
	uint32_t _timeout{0};
	uint32_t _lasttick{0};
	bool _running{false};
protected:


};


// trying not to use
class EffectObject : public EffectObjectHandler
{
private:
	uint16_t * _details;
	ObjectUpdateCallback _ObjUpdate = nullptr;
	AnimationUpdateCallback _AniUpdate = nullptr;
	uint16_t _position = 0;
	uint16_t _total;
public:

	enum Direction {UP = 0, DOWN, LEFT, RIGHT};

	EffectObject(uint16_t size) : _total(size), _ObjUpdate(nullptr), _AniUpdate(nullptr)
	{
		_details = new uint16_t[_total];
		std::fill_n(_details, _total, 0);  // change...

		Debugobjf("Created Object size %u \n", _total);
	};
	~EffectObject() override
	{
		Debugobjf("[EffectObject] Deconstructor\n");
		delete[] _details;
	}

	uint16_t total() override { return _total; } ;

	void SetObjectUpdateCallback(ObjectUpdateCallback Fn) override;
	void SetPixelUpdateCallback(AnimationUpdateCallback Fn) override;

	inline void Addpixel(uint16_t p) override
	{
		uint16_t n = _position++;
		if (_position > _total) { return; }
		_details[n] = p;
	}

	inline void Addpixel(uint16_t n, uint16_t p) override
	{
		if (n > _total) { return; }
		_details[n] = p;
	}

	uint16_t * pixels() override { return _details;};

//	bool StartAnimations() override;
	bool UpdateObject() override;
	bool reset() override
	{
		_position = 0;
		for (uint16_t i = 0; i < _total; i++) {
			_details[i] = -1;
		}
	}


};



// much simpler
class SimpleEffectObject : public EffectObjectHandler
{
private:
	ObjectUpdateCallback _ObjUpdate = nullptr;

	enum Direction {UP = 0, DOWN, LEFT, RIGHT};

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

	bool UpdateObject() override;
	uint16_t * pixels() override { return _pixels; };


	void SetObjectUpdateCallback(ObjectUpdateCallback Fn) override
	{
		_ObjUpdate = Fn;
	}

private:
	uint16_t * _pixels{nullptr};
	uint16_t _total{0};


};




//  need to move tick to class derivative not in object manager.. so that animations can be handled...
class AnimatedEffectObject : public EffectObjectHandler
{
private:
	ObjectUpdateCallback _ObjUpdate = nullptr;

	enum Direction {UP = 0, DOWN, LEFT, RIGHT};

public:
	AnimatedEffectObject() : _ObjUpdate(nullptr)
	{

	};
	~AnimatedEffectObject() override
	{
		if (_pixels) {
			delete[] _pixels;
		}
		if (animator) {
			delete animator;
		}
	}
	void create(uint16_t size)
	{
		if (_pixels) {
			delete[] _pixels;
		}
		_pixels = new uint16_t[size];
		animator = new NeoPixelAnimator(size);
		total = size;
	}

	void SetObjectUpdateCallback(ObjectUpdateCallback Fn) override
	{
		_ObjUpdate = Fn;
	}

	uint16_t * pixels() override { return _pixels;};


	bool UpdateObject() override
	{
		bool updateflag = false;
		if (millis() - Lasttick() > Timeout() ) {
			if (_ObjUpdate) {
				if (_ObjUpdate()) {
					Timeout(millis());
					updateflag = true;
				} ;

			}
		}

		if (animator && animator->IsAnimating() ) {
			animator->UpdateAnimations();
		}

		return updateflag;
	}

	uint16_t total{0};
	NeoPixelAnimator * animator{nullptr};

private:
	uint16_t * _pixels{nullptr};

};

