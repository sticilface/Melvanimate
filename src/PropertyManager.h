#pragma once

#include <ArduinoJson.h>
#include <internal/RgbColor.h>
#include <internal/HslColor.h>

#include "Palette.h"
#include <IPAddress.h>
#include "Melvtrix.h"

//#define DebugPropertyManager

#if defined(DEBUG_ESP_PORT) && defined(DebugPropertyManager)
//#define PropertyManagerf(...) DEBUG_ESP_PORT.printf(__VA_ARGS__)
#define PropertyManagerf(_1, ...) DEBUG_ESP_PORT.printf_P( PSTR(_1), ##__VA_ARGS__) //  this saves around 5K RAM...

#else
#define PropertyManagerf(...) {}
#endif

//bool rtcUserMemoryRead(uint32_t offset, uint32_t *data, size_t size);
//bool rtcUserMemoryWrite(uint32_t offset, uint32_t *data, size_t size);

class RTC {
public:
	static uint16_t addr_counter;
	static void reset() { addr_counter = 0; }
	RTC(uint16_t size = 4) : _size(size) {
		_addr = addr_counter;
		size_t rounded = (size + 3) & (~3);
		addr_counter += (rounded / 4);
		////Serial.printf("RTC initialised _addr = %u\n", _addr);
	}

	bool write(uint32_t *data) {
		////Serial.printf("RTC write [%u] uint32_t = %u\n", _addr, *data);
		ESP.rtcUserMemoryWrite( _addr, data, _size);
	}

	bool read(uint32_t *data){
		////Serial.printf("RTC read [%u] uint32_t = %u\n", _addr ,*data);
		ESP.rtcUserMemoryRead( _addr, data, _size);
	}

	uint16_t address() { return _addr; }

private:
	uint16_t _addr{0};
	uint16_t _size{0};

};



class AbstractPropertyHandler
{
public:
	virtual ~AbstractPropertyHandler() {}
	virtual bool addJsonProperty(JsonObject & root, bool onlychanged = false) {return false; };
	virtual bool parseJsonProperty(JsonObject & root) { return false; } ;
	//virtual uint16_t setAddr(uint16_t addr) { return addr; } //  if override is not defined then it is not stored so return original address...
	virtual void LoadRTC() {};
	virtual void SaveRTC() {};


	void setChanged(bool changed) {
		_changed = changed;
		PropertyManagerf("[Variable::setChanged]  (%s)_changed  = %s\n", _name, (_changed)? "true": "false") ; }
	const char * name() { return _name; }
	AbstractPropertyHandler* next() { return _next; }
	void next (AbstractPropertyHandler* next) { _next = next; }
	AbstractPropertyHandler* p() { return this; }



private:
	AbstractPropertyHandler* _next = nullptr;

protected:
	const char * _name = nullptr;
	bool _changed{false};
	//uint16_t _rtcAddr{0};

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
		_var = value;
	};

	~Variable() override {}

	T get() { return _var; }

	void set(T value) {
		_var = value;

		SaveRTC();

		// ESP.rtcUserMemoryWrite(_rtcAddr, &val,  1);
		 //Serial.printf("Saving var %s :", _name);
		 //Serial.println(value);
	}



	bool addJsonProperty(JsonObject & root, bool onlychanged = false) override
	{
		PropertyManagerf("[Variable::addJsonProperty] (%s) _changed = %s, onlychanged = %s\n", name() ,(_changed)? "true" : "false",(onlychanged)? "true" : "false"  );
		if (onlychanged && !_changed) { PropertyManagerf("[Variable::addJsonProperty] returning \n"); return false; }
		root[_name] = _var;
		return true;
	}

	bool parseJsonProperty(JsonObject & root) override
	{
		if (root.containsKey(_name)) {

			if (_var != root[_name] ) {
				set(root[_name]);
				_changed = true;
				PropertyManagerf("[Variable::parseJsonProperty] _changed = true\n");
				return true;
			}
		}
		return false;
	}

	// uint16_t setAddr(uint16_t addr) override {
	// 	_rtcAddr = addr;
	// 	return addr + 1;
	// }

	void LoadRTC() override {
		uint32_t data;
		_rtc.read(&data);

		//ESP.rtcUserMemoryRead(_rtcAddr,  &data,  1);
		T var = static_cast<T>(data);
		 //Serial.printf("Retrieved from RTC: %s ", _name);
		 //Serial.println(var);
		_var = var;
	}

	void SaveRTC() override {
		uint32_t temp = static_cast<uint32_t>(_var);
		_rtc.write(&temp);
	}

private:
	T _var{};
	RTC _rtc;
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
		_var = value;
	};
	~Variable() override { }

	RgbColor& get() { return _var; }

	void set(RgbColor value) {
		_var = value;
		SaveRTC();

	 }

	bool addJsonProperty(JsonObject & root, bool onlychanged = false) override;
	bool parseJsonProperty(JsonObject & root) override;
	// uint16_t setAddr(uint16_t addr) override {
	// 	_rtcAddr = addr;
	// 	return addr + 1;
	// }
	void LoadRTC() override {
		uint32_t Val;
		_rtc.read(&Val);
		//ESP.rtcUserMemoryRead(_rtcAddr,  &Val,  1);

		_var.R = Val >> 16 & 0xFF;
		_var.G = Val >> 8 & 0xFF;
		_var.B = Val & 0xFF;

	////Serial.printf("Retrieved from RTC: %s RGB(%u,%u,%u)\n",_name , _var.R, _var.G, _var.B);

		// ESP.rtcUserMemoryRead(_rtcAddr,  &Var.Data32,  1);
		// //Serial.printf("Retrieved from RTC: %s RGB(%u,%u,%u)\n",_name , Var.Bytes[0], Var.Bytes[1], Var.Bytes[2]);
		// _var.R = Var.Bytes[0];
		// _var.G = Var.Bytes[1];
		// _var.B = Var.Bytes[2];
	}

	void SaveRTC() override {
		uint32_t Val = (uint32_t)( _var.R << 16 | _var.G << 8 | _var.B );
		_rtc.write(&Val);
	}

private:
	RgbColor _var = RgbColor(0, 0, 0);
	RTC _rtc;
	//
	// union
	// {
	// uint32_t Data32;
	// uint8_t Bytes[4];
	// } Var;

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

	bool addJsonProperty(JsonObject & root, bool onlychanged = false) override;
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
	Variable(const char * name, Palette::palette_type  value)
	{
		_name = name;
		//set(value);
		_var.mode(value);
	};
	~Variable() override {}

	Palette * get() { return &_var; }
	//void set(Palette * value) { _var = *value; }

	bool addJsonProperty(JsonObject & root, bool onlychanged = false) override
	{
		if (onlychanged && !_changed) { return false; }
		return _var.addJson(root);
	}

	bool parseJsonProperty(JsonObject & root) override
	{
		if (root.containsKey(_name)) {
			_changed =  _var.parseJson(root);
			return _changed;
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
		//Serial.printf("Melvtrix Size = %u\n", sizeof(MelvtrixMan));
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

	bool addJsonProperty(JsonObject & root, bool onlychanged = false) override
	{
		if (onlychanged && !_changed) { return false; }
		return _var->addJson(root);
	}

	bool parseJsonProperty(JsonObject & root) override
	{
		if (root.containsKey(_name)) {
			_changed = _var->parseJson(root);
			return _changed;
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
		_var = INADDR_NONE;
	};
	Variable(const char * name, IPAddress value)
	{
		_name = name;
		_var = value;
	};
	~Variable() override {}

	IPAddress get() { return _var; }

	void set(IPAddress value) {
		_var = value;

		SaveRTC();

	}

	bool addJsonProperty(JsonObject & root, bool onlychanged = false) override
	{
		if (onlychanged && !_changed) { return false; }
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
					set(ret);
					_changed = true;
					return true;
				}
			} else {
				const char * input = root[_name];
				IPAddress temp;
				if (temp.fromString(input)) {
					if (temp != _var) {
						set(temp);
						_changed = true;
						return true;
					}
				}
			}
		}

		return false;
	}

	// uint16_t setAddr(uint16_t addr) override {
	// 	_rtcAddr = addr;
	// 	return addr + 1;
	// }

	void LoadRTC() override {

		uint32_t data;
		//ESP.rtcUserMemoryRead(_rtcAddr,  &data,  1);
		_rtc.read(&data);

		_var[0] = data >> 24 & 0xFF;
	  _var[1] = data >> 16 & 0xFF;
		_var[2] = data >> 8 & 0xFF;
		_var[3] = data & 0xFF;

		////Serial.printf("Retrieved from RTC: %s[%u]  %u -> (%s)\n", _name,_rtc.address() ,data, _var.toString().c_str() ) ;
	}

	void SaveRTC() {
		uint32_t Val = (uint32_t)( _var[0] << 24 | _var[1] << 16 | _var[2] << 8 | _var[3] );

		////Serial.printf("Setting IP %s[%u] (%s) -> %u\n", _name,_rtc.address(), value.toString().c_str(), Val );
		//ESP.rtcUserMemoryWrite(_rtcAddr, &Val,  1);
		_rtc.write(&Val);
	}

private:
	IPAddress _var;
	RTC _rtc;
};


class PropertyManager
{
public:
	PropertyManager(): _firsthandle(nullptr) { }

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
	bool addEffectJson(JsonObject& root, bool onlychanged = false) ;
	void EndVars();
	void GetRTCdata(){
		AbstractPropertyHandler* handle = nullptr;
		for (handle = _firsthandle; handle; handle = handle->next()) {
			handle->LoadRTC();
		}
	}
	void SaveRTCdata(){
		AbstractPropertyHandler* handle = nullptr;
		for (handle = _firsthandle; handle; handle = handle->next()) {
			handle->SaveRTC();
		}
	}

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

private:
	AbstractPropertyHandler* _firsthandle;
};
