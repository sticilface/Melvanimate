#pragma once

#include <arduinojson.h>
#include <RgbColor.h>
#include <functional>
//#include "EffectManager.h"


class AbstractPropertyHandler
{
public:
//   `AbstractPropertyHandler::addJsonProperty(ArduinoJson::JsonObject&)'
	virtual bool addJsonProperty(JsonObject & root) {return false; };
	virtual bool parseJsonProperty(JsonObject & root) { return false; } ;

	const char * name() { return _name; }
	AbstractPropertyHandler* next() { return _next; }
	void next (AbstractPropertyHandler* next) { _next = next; }

private:
	AbstractPropertyHandler* _next = nullptr;
protected:
	const char * _name = nullptr;

};

//  template class for everything
template <class T>
class Variable: public AbstractPropertyHandler
{
public:
	Variable(const char * name)
	{
		_name = name;
	};

	T get() { return _var; }
	bool addJsonProperty(JsonObject & root) override
	{
		root[_name] = _var;
		return true;
	}

	bool parseJsonProperty(JsonObject & root) override
	{
		if (root.containsKey(_name)) {
			if (_var != root[_name] ) {
				_var = root[_name];
				return true;
			}
		}
		return false;
	}

private:
	T _var;
};


class PropertyManager
{
public:
	PropertyManager(): _firsthandle(nullptr) {}
	AbstractPropertyHandler* addVar(AbstractPropertyHandler* ptr);

	template<class T> T getVar(const char * property)
	{
		AbstractPropertyHandler* handle = nullptr;

		for (handle = _firsthandle; handle; handle = handle->next()) {
			if (!strcmp(handle->name(), property)) {
				return (  (Variable<T>*)(handle))->get();
			}
		}

		return (T)(NULL);  // not sure about this... likely
	}

	//  these functions should 'overridde from the effectHandler'
	bool parseJsonEffect(JsonObject & root) ;  // use json so it can be used with MQTT etc...
	bool addEffectJson(JsonObject& root) ;


private:
	AbstractPropertyHandler* _firsthandle;
};






// specialisation for Rgbcolor
template <>
class Variable<RgbColor>: public AbstractPropertyHandler
{
public:
	Variable(const char * name)
	{
		_name = name;
	};

	RgbColor get() { return _var; }
	bool addJsonProperty(JsonObject & root) override ;
	bool parseJsonProperty(JsonObject & root) override ;

private:
	RgbColor _var;
};




// class effect
// {
// public:
// 	effect()
// 	{
// 		manager.add(new Variable<int>("int"));
// 	}

// 	void run() {
// 		int a = manager.get<int>("int");
// 	}
// private:
// 	PropertyManager manager;
// };









