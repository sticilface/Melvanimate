/*

Handler to add / create / run multiple Effect Objects at the same time...




*/

#pragma once
#include <functional>
#include "Arduino.h"

class EffectObjectHandler;

typedef std::function<void()> ObjectUpdateCallback;
typedef std::function<void(uint16_t, uint16_t)> AnimationUpdateCallback;


class EffectGroup
{
public:
	EffectGroup() : _firstHandle(nullptr), _currentHandle(nullptr), _lastHandle(nullptr) {};
	~EffectGroup();
	EffectObjectHandler* Add(uint16_t i, uint32_t timeout, EffectObjectHandler* Fn);
	void Update();
	void Run();
	bool Inuse(uint16_t pixel);
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
	virtual void SetObjectUpdateCallback(ObjectUpdateCallback Fn) {}
	virtual void SetPixelUpdateCallback(AnimationUpdateCallback Fn) {}
	virtual void Addpixel(uint16_t n, uint16_t p) {}
	virtual void Addpixel(uint16_t p) {}

	virtual int16_t * getdata() { }
	virtual bool StartAnimations() {}
	virtual bool UpdateObject() {}
	virtual uint16_t total() {};

	void Timeout(uint32_t timeout) { _timeout = timeout; };
	uint32_t Timeout() { return _timeout; }
	void Lasttick(uint32_t lasttick) { _lasttick = lasttick; };
	uint32_t Lasttick() { return _lasttick; }

	EffectObjectHandler* next() { return _next; } //  ASK what is next
	void next (EffectObjectHandler* next) { _next = next; } //  Set what is next
	void id (uint16_t id) { _id = id; }
	const uint16_t id() { return _id; };
	bool virtual reset() {}

private:
	EffectObjectHandler* _next = nullptr;
	uint16_t _id;
	uint32_t _timeout = 0;
	uint32_t _lasttick = 0;
protected:


};


class EffectObject : public EffectObjectHandler
{
private:
	int16_t * _details;
	ObjectUpdateCallback _ObjUpdate = nullptr; 
	AnimationUpdateCallback _AniUpdate = nullptr; 
	uint16_t _position = 0;
	uint16_t _total;
public:

	enum Direction {UP = 0, DOWN, LEFT, RIGHT};

	EffectObject(uint16_t size) : _total(size), _ObjUpdate(nullptr), _AniUpdate(nullptr)
	{
		_details = new int16_t[_total];
		std::fill_n(_details, _total, -1);

		Serial.printf("Created Object size %u \n", _total);
	};
	~EffectObject()
	{
		delete[] _details;
	}

	uint16_t total() override { return _total; } ;

	void SetObjectUpdateCallback(ObjectUpdateCallback Fn) override;
	void SetPixelUpdateCallback(AnimationUpdateCallback Fn) override;

	inline void Addpixel(uint16_t p) override
	{
		uint16_t n = _position++;
		if (_position > _total) return;
		_details[n] = p;
	}

	inline void Addpixel(uint16_t n, uint16_t p) override
	{
		if (n > _total) return;
		_details[n] = p;
	}

	int16_t * getdata() override { return _details;};

	bool StartAnimations() override;
	bool UpdateObject() override;
	bool reset() override
	{
		_position = 0;
		for (uint16_t i = 0; i < _total; i++) {
			_details[i] = -1;
		}
	}


};

