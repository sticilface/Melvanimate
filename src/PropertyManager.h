#pragma once

#include <arduinojson.h>
#include <RgbColor.h>
#include "palette.h"
#include "IPAddress.h"


class AbstractPropertyHandler
{
public:
	virtual ~AbstractPropertyHandler() {}
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

	~Variable() override {}

	T get() { return _var; }
	void set(T value) { _var = value; }
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


// specialisations for Variables
template <>
class Variable<RgbColor>: public AbstractPropertyHandler
{
public:
	Variable(const char * name)
	{
		_name = name;
	};

	~Variable() override { }

	RgbColor get() { return _var; }
	void set(RgbColor value) { _var = value; }

	bool addJsonProperty(JsonObject & root) override;
	bool parseJsonProperty(JsonObject & root) override; 


private:
	RgbColor _var = RgbColor(0, 0, 0);
};


template <>
class Variable<const char *>: public AbstractPropertyHandler
{
public:
	Variable(const char * name): _var(nullptr)
	{
		_name = name;
	};

	~Variable() override
	{
		if (_var) {
			free((void*)_var);
		}
	}

	const char * get() { return _var; }
	void set(const char * in)
	{

		if (_var) {
			if ( strcmp(in, _var)) {
				free( (void*)_var);  //  not good... but i really want to free that const char *
				_var = strdup(in);
			}
		} else {
			_var = strdup(in);
		}

	}

	bool addJsonProperty(JsonObject & root) override;
	bool parseJsonProperty(JsonObject & root) override;

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
	~Variable() override {}

	Palette* get() { return &_var; }
	void set(Palette * value) { _var = *value; }

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

template <>
class Variable<IPAddress>: public AbstractPropertyHandler
{
public:
	Variable(const char * name)
	{
		_name = name;
		_var = IPAddress(0, 0, 0, 0);
	};
	~Variable() override {}

	IPAddress get() { return _var; }
	void set(IPAddress value) { _var = value; }

	bool addJsonProperty(JsonObject & root) override
	{
		JsonArray & IP = root.createNestedArray(_name);
		for (uint8_t i = 0; i < 4; i++) {
			IP.add(_var[i]);
		}

		return true;
	}

	bool parseJsonProperty(JsonObject & root) override
	{
		if (root.containsKey(_name) && root[_name].is<JsonArray&>()) {
			JsonArray & IP = root[_name];
			IPAddress ret;
			for (uint8_t i = 0; i < 4; i++) {
				ret[i] = IP[i];
			}

			if (_var != ret) {
				_var = ret;
				return true;
			}
		}

		return false;
	}

private:
	IPAddress _var;
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



