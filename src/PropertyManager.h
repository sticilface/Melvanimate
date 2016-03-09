#pragma once

#include <arduinojson.h>
#include <internal/RgbColor.h>
#include <internal/HslColor.h>

#include "palette.h"
#include <IPAddress.h>
#include "melvtrix.h"

//#define DebugPropertyManager

#ifdef DebugMelvanimate
#define PropertyManagerf(...) Serial.printf(__VA_ARGS__)
#else
#define PropertyManagerf(...) {}
#endif


class AbstractPropertyHandler
{
public:
	virtual ~AbstractPropertyHandler() {}
	virtual bool addJsonProperty(JsonObject & root) {return false; };
	virtual bool parseJsonProperty(JsonObject & root) { return false; } ;

	const char * name() { return _name; }
	AbstractPropertyHandler* next() { return _next; }
	void next (AbstractPropertyHandler* next) { _next = next; }
	AbstractPropertyHandler* p() { return this; }

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
	Variable(const char * name, T value)
	{
		_name = name;
		set(value);
	};

	~Variable() override {}

	//T get() { return _var; }
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
	Variable(const char * name, RgbColor value)
	{
		_name = name;
		set(value);
	};
	~Variable() override { }

	RgbColor& get() { return _var; }
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
	Variable(const char * name, const char * value)
	{
		_name = name;
		set(value);
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
	Variable(const char * name, Palette* value)
	{
		_name = name;
		set(value);
	};
	~Variable() override {}

	Palette * get() { return &_var; }
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
class Variable<MelvtrixMan*>: public AbstractPropertyHandler
{
public:
	Variable(const char * name)
	{
		_name = name;
		_var = new MelvtrixMan; 
	};
	Variable(const char * name, MelvtrixMan* value): _var(value)
	{
		_name = name;
		//set(value);
	};
	~Variable() override {
		if (_var) 
		{
			delete _var;
			_var = nullptr; 
		}
	}

	MelvtrixMan * get() { return _var; }

	void set(MelvtrixMan * value) { _var = value; }

	bool addJsonProperty(JsonObject & root) override
	{
		return _var->addJson(root);
	}

	bool parseJsonProperty(JsonObject & root) override
	{
		if (root.containsKey(_name)) {
			return _var->parseJson(root);
		} else {
			return false;
		}
	}

private:
	MelvtrixMan * _var{nullptr};// = MelvtrixMan(1,2,3); 
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
	Variable(const char * name, IPAddress value)
	{
		_name = name;
		set(value);
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
		if (root.containsKey(_name) ) {

			if ( root[_name].is<JsonArray&>()) {
				JsonArray & IP = root[_name];
				IPAddress ret;
				for (uint8_t i = 0; i < 4; i++) {
					ret[i] = IP[i];
				}

				if (_var != ret) {
					_var = ret;
					return true;
				}
			} else {
				const char * input = root[_name];
//				Serial.printf("[Variable<IPAddress>::parseJsonProperty] IP as String %s\n", input);
				return _var.fromString(input);
			}
		}

		return false;
	}

private:
	IPAddress _var;
};




template <class T>
class Array: public AbstractPropertyHandler
{
public:
	Array(const char * name, T value, uint16_t number)
	{
		_number = number;
		_var = new T[_number];
		_name = name;
		if (_var) {
			for (uint16_t i = 0; i < _number; i++) {
				_var[i] = value;
			}
		}
	};

	~Array() override
	{
		if (_var) {
			delete[] _var;
		}
	}

	operator uint16_t() const
	{
		return _number;
	}

	T operator[] (uint16_t i) const
	{
		return _var[i];
	}

	T& operator[] (uint16_t i)
	{
		return _var[i];
	}

	bool addJsonProperty(JsonObject & root) override
	{
		JsonArray& array = root.createNestedArray(_name);

		for (uint16_t i = 0; i < _number; i++) {
			array.add(_var[i]);
		}
		return true;
	}

	bool parseJsonProperty(JsonObject & root) override
	{
		if (root.containsKey(_name) ) {
			if ( root[_name].is<JsonArray&>()) {

				JsonArray& array = root[_name];

				uint16_t count = 0;

				 for (uint16_t i = 0; i < _number; i++) {
				 	_var[i] = (array[i]); 
				}
				return true;
			}
		}
		return false;
	}

private:
	T* _var;
	uint16_t _number;
};


// template <>
// class Array<Array<class T>*>: public AbstractPropertyHandler
// {
// public:
// 	Array(const char * name, T * value, uint16_t number)
// 	{
// 		_number = number;
// 		_var = new T *[_number];
// 		_name = name;
// 		if (_var) {
// 			for (uint16_t i = 0; i < _number; i++) {
// 				_var[i] = value;
// 			}
// 		}
// 	};

// 	~Array() override
// 	{
// 		if (_var) {
// 			delete[] _var;
// 		}
// 	}

// 	operator uint16_t() const
// 	{
// 		return _number;
// 	}

// 	T * operator[] (uint16_t i) const
// 	{
// 		return _var[i];
// 	}

// 	T *& operator[] (uint16_t i)
// 	{
// 		return _var[i];
// 	}

// 	bool addJsonProperty(JsonObject & root) override
// 	{
// 		JsonArray& array = root.createNestedArray(_name);

// 		for (uint16_t i = 0; i < _number; i++) {
// 			array.add(_var[i]);
// 		}
// 		return true;
// 	}

// 	bool parseJsonProperty(JsonObject & root) override
// 	{
// 		if (root.containsKey(_name) ) {
// 			if ( root[_name].is<JsonArray&>()) {

// 				JsonArray& array = root[_name];

// 				uint16_t count = 0;

// 				 for (uint16_t i = 0; i < _number; i++) {
// 				 	_var[i] = array[i]; 
// 				}
// 				return true;
// 			}
// 		}
// 		return false;
// 	}

// private:
// 	T ** _var;
// 	uint16_t _number;
// };


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

		return T{};
	}
	template<class T> void setVar(const char * property, T value)
	{
		AbstractPropertyHandler* handle = nullptr;

		for (handle = _firsthandle; handle; handle = handle->next()) {
			if (!strcmp(handle->name(), property)) {
				((Variable<T>*)(handle))->set(value);
			}
		}
	}

	//  these functions should 'overridde from the effectHandler'
	bool parseJsonEffect(JsonObject & root) ;  // use json so it can be used with MQTT etc...
	bool addEffectJson(JsonObject& root) ;
	void EndVars();

	virtual bool InitVars() { return false; }

	template<class T>
	bool hasProperty(const char * name) const
	{
		AbstractPropertyHandler* handle = nullptr;

		for (handle = _firsthandle; handle; handle = handle->next()) {
			if (!strcmp(handle->name(), name)) {
				return true;
			}
		}
		return false;
	}

	template<class T>
	Array<T>& variable(const char * name)
	{

		AbstractPropertyHandler* handle = nullptr;

		for (handle = _firsthandle; handle; handle = handle->next()) {
			if (!strcmp(handle->name(), name)) {
				return *((Variable<T>*)handle);
			}
		}
	}

	template<class T>
	Array<T>& array(const char * name)
	{

		AbstractPropertyHandler* handle = nullptr;

		for (handle = _firsthandle; handle; handle = handle->next()) {
			if (!strcmp(handle->name(), name)) {
				return *((Array<T>*)handle);
			}
		}
	}


	// overloaded index to allow getting and settings using index...
	// template<class T>
	// T operator[] (const char * name) const
	// {

	// 	AbstractPropertyHandler* handle = nullptr;

	// 	for (handle = _firsthandle; handle; handle = handle->next()) {
	// 		if (!strcmp(handle->name(), name)) {
	// 			return ((Variable<T>*)(handle))->get();
	// 		}
	// 	}
	// 	return T{};
	// }

	// template<class T>
	// T& operator[] (const char * name)
	// {
	// 	AbstractPropertyHandler* handle = nullptr;

	// 	for (handle = _firsthandle; handle; handle = handle->next()) {
	// 		if (!strcmp(handle->name(), name)) {

	// 			return (  (Variable<T>*)(handle))->get();
	// 		}
	// 	}
	// 	return T{};
	// }

	// template<class T>
	// T* operator* (const char * name) const
	// {
	// 	AbstractPropertyHandler* handle = nullptr;

	// 	for (handle = _firsthandle; handle; handle = handle->next()) {
	// 		if (!strcmp(handle->name(), name)) {
	// 			return ((Variable<T>*)(handle));
	// 		}
	// 	}
	// 	return nullptr;
	// }




private:
	AbstractPropertyHandler* _firsthandle;

};



