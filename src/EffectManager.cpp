
//  need to add wait for init and end animations
//

#include "Arduino.h"
#include "EffectManager.h"

/*---------------------------------------------

				Effect Manager

---------------------------------------------*/

EffectManager::EffectManager() : _count(0), _firstHandle(nullptr), _currentHandle(nullptr), _lastHandle(nullptr), _toggleHandle(nullptr),
	_NextInLine(nullptr)
{};

// EffectManager::EffectManager(NeoPixelBus ** strip, NeoPixelAnimator ** animator) : _count(0), _firstHandle(nullptr), _currentHandle(nullptr), _lastHandle(nullptr),
// 	timeoutvar(0), effectposition(0), _NextInLine(nullptr), _strip(strip), _animator(animator)
// {};




bool EffectManager::Add(const char * name, EffectHandler* handle)
{
	_count++;

	if (!_lastHandle) {
		_firstHandle = handle; //  set the first handle
		_firstHandle->name(name); // set its name in the handler... abstract it out so user doesn't have to
		_lastHandle = handle;  // set this so we know first slot gone.

	} else {
		_lastHandle->next(handle); // give the last handler address of next
		_lastHandle = handle;  // move on..
		_lastHandle->name(name);  // give it name...
	}
	Serial.printf("ADDED EFFECT %u: %s\n", _count, _lastHandle->name());
}

bool EffectManager::Start() {
	if (_toggleHandle) Start(_toggleHandle->name()); 
}

bool EffectManager::Start(const char * name)
{
	//  end current effect...
	//  need to store address of next... and handle changeover in the loop...
	// this function should signal to current to END & store

	// actually.. maybe use return values to signal... with timeouts to prevent getting stuck..
	// manager sends... stop()....  until that returns true.. it can't --- NOPE not going to work... Start and stop should only get called once...
	if (_currentHandle) _currentHandle->Stop();

	EffectHandler* handler;
	bool found = false;
	for (handler = _firstHandle; handler; handler = handler->next()) {
		if ( strcmp( handler->name(), name) == 0) {
			found = true;
			break;
		}
	}
	if (found) {
		_NextInLine = handler;
		if( strcmp(handler->name(), "Off") != 0) { _toggleHandle = handler; } 
		return true;
	} else return false;

};

bool EffectManager::Next()
{
	_currentHandle = _currentHandle->next();
};


bool EffectManager::Stop()
{
	if (_currentHandle)  _currentHandle->Stop();
};

bool EffectManager::Pause()
{
	if (_currentHandle)  _currentHandle->Pause();
};

void EffectManager::Refresh()
{
	if (_currentHandle) _currentHandle->Refresh();

}

void EffectManager::Loop()
{

	bool waiting = false;

	if (_waitFn)  {
		waiting = _waitFn();
	}

	//  This flips over to next effect asyncstyle....
	if (!waiting && _NextInLine) {
		Serial.println("Next effect STARTED");
		_currentHandle = _NextInLine;
		_NextInLine = nullptr;
		_currentHandle->Start();
		return;
	}

	if (!waiting && _currentHandle)  _currentHandle->Run();
};

void EffectManager::SetTimeout(uint32_t time)
{
	if (_currentHandle) _currentHandle->SetTimeout(time);
}

void EffectManager::SetTimeout(const char * name, uint32_t time)
{

	EffectHandler* handler;
	for (handler = _firstHandle; handler; handler = handler->next()) {
		if ( strcmp( handler->name(), name) == 0)
			break;
	}

	if (handler) handler->SetTimeout(time);

}

const char * EffectManager::getName()
{
	if (_NextInLine) {
		return _NextInLine->name(); //  allows webgui to display the current selected instead of the ending one.
	} else 	if (_currentHandle) {
		return _currentHandle->name();
	} else {
		return "";
	}
}

const char * EffectManager::getName(uint8_t i)
{
	if (i > _count) return "";

	EffectHandler* handler;
	uint8_t count = 0;
	for (handler = _firstHandle; handler; handler = handler->next()) {
		if ( i == count ) break;
		count++;
	}
	return handler->name();
}
/*---------------------------------------------

				Generic Class

---------------------------------------------*/



// Moved objec stuff to newer class..



// bool EffectObject::StartBasicAnimations(uint16_t time) {

// 	Serial.println("Start Andimations");

// 	if (!_bus || !_animator) return false;

// 	for (uint16_t i = 0; i < _total; i++) {

// 		Details_s * current = &_details[i];

// 		current->original = _bus->GetPixelColor(current->pixel);

// 		Serial.printf(" I=> %u->%u original(%u,%u,%u) target(%u,%u,%u)\n", i, current->pixel,
// 		              current->original.R, current->original.G, current->original.B,
// 		              current->target.R, current->target.G, current->target.B );

// 		AnimUpdateCallback  animUpdate = [this, i, &current](float progress)
// 		{
// 			RgbColor updatedColor = RgbColor::LinearBlend(current->original, current->target, progress );
// 			_bus->SetPixelColor(current->pixel, updatedColor);
// 		};

// 		_animator->StartAnimation(current->pixel, time, animUpdate);
// 	}
// 	return true;
// }



/* ------------------------------------------------------------------------
				My own animator class
--------------------------------------------------------------------------*/


// NeoPixelObject::NeoPixelObject(NeoPixelBus* bus, uint16_t count) :
//     _bus(bus),
//     _animationLastTick(0),
//     _activeAnimations(0),
//     _isRunning(true)
// {
//     _animations = new AnimationContext[count];
// }

// NeoPixelObject::~NeoPixelObject()
// {
//     _bus = NULL;
//     if (_animations)
//     {
//         delete[] _animations;
//         _animations = NULL;
//     }
// }

// void NeoPixelObject::StartAnimation(uint16_t n, uint16_t time, ObjUpdateCallback animUpdate)
// {
//     if (n >= _bus->PixelCount())
//     {
//         return;
//     }

//     if (_activeAnimations == 0)
//     {
//         _animationLastTick = millis();
//     }

//     StopAnimation(n);

//     if (time == 0)
//     {
//         time = 1;
//     }

//     _animations[n].time = time;
//     _animations[n].remaining = time;
//     _animations[n].fnUpdate = animUpdate;

//     _activeAnimations++;
// }

// void NeoPixelObject::StopAnimation(uint16_t n)
// {
//     if (IsAnimating(n))
//     {
//         _activeAnimations--;
//         _animations[n].time = 0;
//         _animations[n].remaining = 0;
//         _animations[n].fnUpdate = NULL;
//     }
// }

// void NeoPixelObject::FadeTo(uint16_t time, RgbColor color)
// {
//     for (uint16_t n = 0; n < _bus->PixelCount(); n++)
//     {
//         RgbColor original = _bus->GetPixelColor(n);
//         ObjUpdateCallback animUpdate = [=](float progress)
//         {
//             RgbColor updatedColor = RgbColor::LinearBlend(original, color, progress);
//             _bus->SetPixelColor(n, updatedColor);
//         };
//         StartAnimation(n, time, animUpdate);
//     }
// }

// void NeoPixelObject::UpdateAnimations(uint32_t maxDeltaMs)
// {
//     if (_isRunning)
//     {
//         uint32_t currentTick = millis();
//         uint32_t delta = currentTick - _animationLastTick;

//         if (delta > maxDeltaMs)
//         {
//             delta = maxDeltaMs;
//         }

//         if (delta > 0)
//         {
//             uint16_t countAnimations = _activeAnimations;

//             AnimationContext* pAnim;

//             for (uint16_t iAnim = 0; iAnim < _bus->PixelCount() && countAnimations > 0; iAnim++)
//             {
//                 pAnim = &_animations[iAnim];

//                 if (pAnim->remaining > delta)
//                 {
//                     pAnim->remaining -= delta;

//                     float progress = (float)(pAnim->time - pAnim->remaining) / (float)pAnim->time;

//                     pAnim->fnUpdate(progress);
//                     countAnimations--;
//                 }
//                 else if (pAnim->remaining > 0)
//                 {
//                     pAnim->fnUpdate(1.0f);
//                     StopAnimation(iAnim);
//                     countAnimations--;
//                 }
//             }

//             _animationLastTick = currentTick;
//         }
//     }
// }

