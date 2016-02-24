#pragma once

#include <arduinojson.h>
#include <RgbColor.h>
#include "palette.h"


class AbstractPropertyHandler
{
public:
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
	T _var{}; 
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

		return 0; 
	}

	//  these functions should 'overridde from the effectHandler'
	bool parseJsonEffect(JsonObject & root) ;  // use json so it can be used with MQTT etc...
	bool addEffectJson(JsonObject& root) ;
	void EndVars();
	virtual bool InitVars() { return false; }


private:
	AbstractPropertyHandler* _firsthandle;
};




// specialisations for Variables 
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
	RgbColor _var = RgbColor(0,0,0); 
};

template <>
class Variable<const char *>: public AbstractPropertyHandler
{
public:
	Variable(const char * name): _var(nullptr)
	{
		_name = name;
	};

	const char * get() { return _var; }
	bool addJsonProperty(JsonObject & root) override ;
	bool parseJsonProperty(JsonObject & root) override ;

private:
	const char * _var;
};

template <>
class Variable<Palette*>: public AbstractPropertyHandler
{
public:
	Variable(const char * name): _var(name)
	{
		_name = name;
	};

	Palette* get() { return &_var; }

	bool addJsonProperty(JsonObject & root) override
	{
		return _var.addJson(root);
	}

	bool parseJsonProperty(JsonObject & root) override
	{
		if (root.containsKey(_name)) {
			return _var.parseJson(root);
		} else {
			return false;
		}
	}

private:
	Palette _var;
};











