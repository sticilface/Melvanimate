#include "effectobject.h"
#include "Arduino.h"


EffectGroup::~EffectGroup()
{
	//  iterate through all objects and delete them properly...
	EffectObjectHandler * holding = _firstHandle->next() , *todelete;

	do {
		todelete = holding;
		holding = holding->next();
		if (todelete) delete todelete;
	} while (holding);

}

EffectObjectHandler * EffectGroup::Add(uint16_t id, uint32_t timeout , EffectObjectHandler* handle)
{
	if (!handle) { return 0; }

	handle->id(id);
	handle->Timeout(timeout);


	if (!_lastHandle) {
		_firstHandle = handle; //  set the first handle
		_lastHandle = handle;  // set this so we know first slot gone.

	} else {
		_lastHandle->next(handle); // give the last handler address of next
		_lastHandle = handle;  // move on..
	}

	return handle;
	//Serial.printf("EFFECT Object ID:%u \n", _lastHandle->id());
}

EffectObjectHandler* EffectGroup::Get(uint16_t x)
{

	EffectObjectHandler* handler; ;

	for (handler = _firstHandle; handler; handler = handler->next() ) {
		if ( handler->id() == x)
			break;
	}
	return handler;
}




void EffectGroup::Update()
{

	EffectObjectHandler* handler;

	for (handler = _firstHandle; handler; handler = handler->next() ) {

		uint32_t lasttick = handler->Lasttick();
		uint32_t timeout = handler->Timeout();
		if (millis() - lasttick > timeout || lasttick == 0) {

			handler->UpdateObject(); 
			handler->StartAnimations(); 
			handler->Lasttick(millis());
		
		}

	}
}

void EffectGroup::Run()
{
	// EffectObjectHandler* handler;
	// for (handler = _firstHandle; handler; handler = handler->next() ) {
	// 	handler->StartAnimations();
	// }

}

bool EffectGroup::Inuse(uint16_t pixel)
{

	EffectObjectHandler* handler;
	for (handler = _firstHandle; handler; handler = handler->next() ) {

		int16_t * data = handler->getdata();

		for (uint16_t i = 0; i < handler->total(); i++) {
			if (data[i] == pixel) return true;
		}

	}

	return false;
}

bool EffectObject::UpdateObject()
{
	if (!_ObjUpdate) return false;
	_ObjUpdate();
}

bool EffectObject::StartAnimations()
{

	if (!_AniUpdate) return false;

	for (uint16_t i = 0; i < _total; i++) {

		int16_t  current = _details[i];
		if (current == -1) break;
		_AniUpdate(i, current);
	}

	return true;
}

void EffectObject::SetObjectUpdateCallback(ObjectUpdateCallback Fn)
{
	_ObjUpdate = Fn;
}

void EffectObject::SetPixelUpdateCallback(AnimationUpdateCallback Fn)
{
	_AniUpdate = Fn;
}

