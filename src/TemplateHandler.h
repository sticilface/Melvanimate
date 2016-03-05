#pragma once

#include "EffectHandler.h"

template<class T>
class TemplateHandler {
public:
	TemplateHandler(const char * name)
	{

	}
	~TemplateHandler()
	{
		if (_handle) {
			delete _handle; 
		}
	}

	bool init() {
		_handle = new T; 
	}

	EffectHandler * get()
	{
		return _handle; 
	}

private:
	EffectHandler * _handle{nullptr};
	const char * _name{nullptr};
};


