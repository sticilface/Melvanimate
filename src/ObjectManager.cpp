#include "ObjectManager.h"
#include "Arduino.h"


EffectGroup::~EffectGroup()
{
        //  iterate through all objects and delete them properly...
        EffectObjectHandler * holding = _firstHandle, *todelete;
        uint16_t count = 0;
        do {
                todelete = holding;
                holding = holding->next();
                if (todelete) {
                        delete todelete;
                        count++;
                }
        } while (holding);
        Debugobjf("[~EffectGroup] %u effects deleted\n", count);

}

EffectObjectHandler * EffectGroup::Add(uint16_t id, EffectObjectHandler* handle)
{
        if (!handle) { return 0; }

        handle->id(id);
        //handle->Timeout(timeout);


        if (!_lastHandle) {
                _firstHandle = handle; //  set the first handle
                _lastHandle = handle; // set this so we know first slot gone.

        } else {
                _lastHandle->next(handle); // give the last handler address of next
                _lastHandle = handle; // move on..
        }

        return handle;
        //Debugobjf("EFFECT Object ID:%u \n", _lastHandle->id());
}

EffectObjectHandler* EffectGroup::Get(uint16_t x)
{

        EffectObjectHandler* handler;;

        for (handler = _firstHandle; handler; handler = handler->next() ) {
                if ( handler->id() == x) {
                        break;
                }
        }
        return handler;
}




void EffectGroup::Update()
{

         EffectObjectHandler* handler = nullptr;

        // if (!handler) { handler = _firstHandle; }
        // if (handler) {
        //         handler->UpdateObject();
        //         handler = handler->next();
        // }


	for (handler = _firstHandle; handler; handler = handler->next() ) {

        // uint32_t lasttick = handler->Lasttick();
        // uint32_t timeout = handler->Timeout();

        // if (millis() - lasttick > timeout || lasttick == 0) {

		handler->UpdateObject();
//  handler->StartAnimations();
//  handler->Lasttick(millis());

        // }

        }
}

// void EffectGroup::Run()
// {
//  // EffectObjectHandler* handler;
//  // for (handler = _firstHandle; handler; handler = handler->next() ) {
//  //  handler->StartAnimations();
//  // }

// }

bool EffectGroup::Inuse(EffectObjectHandler* exclude,  uint16_t pixel)
{

        EffectObjectHandler* handler;

        for (handler = _firstHandle; handler; handler = handler->next() ) {

                uint16_t * data = handler->pixels();

                if (data && handler != exclude) {

                        for (uint16_t i = 0; i < handler->total(); i++) {

                                if (data[i] == pixel) {
                                        //Serial.printf("%u = %u\n", data[i] , pixel);
                                        return true;

                                }

                        }
                }

        }

        return false;
}


// bool SimpleEffectObject<typename T>::UpdateObject()
// {
//  if (millis() - Lasttick() > Timeout() ) {
//   if (_ObjUpdate) {
//    if (_ObjUpdate()) {
//     Lasttick(millis());
//     return true;
//    }
//   }
//  }
//  return false;
// }



// //  maybe not in use...
// bool EffectObject::UpdateObject()
// {
//  if (_ObjUpdate) {
//   _ObjUpdate();
//  } else {
//   return false;
//  }

// }

// void EffectObject::SetObjectUpdateCallback(ObjectUpdateCallback Fn)
// {
//  _ObjUpdate = Fn;
// }

// void EffectObject::SetPixelUpdateCallback(AnimationUpdateCallback Fn)
// {
//  _AniUpdate = Fn;
// }


// bool AnimatedEffectObject::move(Direction dir)
// {

//  if (!_constrain) {
//   switch (dir) {
//   case N: {
//    if (y++ > _matrix->height()) {
//     y = 0;
//    }
//    return true;
//    break;
//   }
//   case NE: {
//    if (x++ > _matrix->width()) {
//     x = 0;
//    }
//    if (y++ > _matrix->height()) {
//     y = 0;
//    }
//    return true;
//    break;
//   }
//   case E: {
//    if (x++ > _matrix->width()) {
//     x = 0;
//    }
//    return true;
//    break;
//   }
//   case SE: {
//    if (x++ > _matrix->width()) {
//     x = 0;
//    }
//    if (y-- < 0 ) {
//     y = _matrix->height();
//    }
//    return true;
//    break;
//   }
//   case S: {
//    if (y-- < 0 ) {
//     y = _matrix->height();
//    }
//    return true;
//    break;
//   }
//   case SW: {
//    if (x-- < 0 ) {
//     x = _matrix->width();
//    }
//    if (y-- < 0 ) {
//     y = _matrix->height();
//    }
//    return true;
//    break;
//   }
//   case W: {
//    if (x-- < 0 ) {
//     x = _matrix->width();
//    }
//    return true;
//    break;
//   }
//   case NW: {
//    if (y++ > _matrix->height()) {
//     y = 0;
//    }
//    if (x-- > 0 ) {
//     x = _matrix->width();
//    }
//    return true;
//    break;
//   }
//   }
//  } else {
//   switch (dir) {

//   case N: {

//    if (y++ > _matrix->height()) {
//     y = 0;
//    }
//    return true;
//    break;
//   }
//   case NE: {
//    if (x++ > _matrix->width()) {
//     x = 0;
//    }
//    if (y++ > _matrix->height()) {
//     y = 0;
//    }
//    return true;
//    break;
//   }
//   case E: {
//    if (x++ > _matrix->width()) {
//     x = 0;
//    }
//    return true;
//    break;
//   }
//   case SE: {
//    if (x++ > _matrix->width()) {
//     x = 0;
//    }
//    if (y-- < 0 ) {
//     y = _matrix->height();
//    }
//    return true;
//    break;
//   }
//   case S: {
//    if (y-- < 0 ) {
//     y = _matrix->height();
//    }
//    return true;
//    break;
//   }
//   case SW: {
//    if (x-- < 0 ) {
//     x = _matrix->width();
//    }
//    if (y-- < 0 ) {
//     y = _matrix->height();
//    }
//    return true;
//    break;
//   }
//   case W: {
//    if (x-- < 0 ) {
//     x = _matrix->width();
//    }
//    return true;
//    break;
//   }
//   case NW: {
//    if (y++ > _matrix->height()) {
//     y = 0;
//    }
//    if (x-- > 0 ) {
//     x = _matrix->width();
//    }
//    return true;
//    break;
//   }
//   }
//  }
// }
