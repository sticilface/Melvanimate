//
//			Super experimental NOT working at all... have to figure out types.
//
//

#include <arduinojson.h>

class PropertyHandlerVariable;
class newtestabc;


class handler
{
public:
	handler* next() { return _next; }
	void next (handler* next) { _next = next; }
private:
	handler* _next = nullptr;
};

template <class T>
class Variable: public handler
{
public:
	Variable(const char * name): _name(name) {};

	T &operator[](const char * in)
	{
		if (strcmp(in, _name)) {
			return _var;
		}
	}

	const char * name() { return _name; }

	virtual T get() { return _var; }
	virtual void set(T in) { _var = in; }
	virtual bool addJsonProperty(JsonObject & root) { return false; }
	virtual bool parseJsonProperty(JsonObject & root) { return false; }

private:
	const char * _name = nullptr;
	T _var;
};

class masterHolderclass
{
public:
	void add(handler* ptr)
	{
		if (ptr) {

			if (!_firsthandle) {
				_firsthandle = ptr;
				ptr->next(nullptr);
				//Serial.printf("[EffectHandler::installProperty] installed property 1: %s\n", ptr->name());
			} else {

				handler* handle = nullptr;
				handler* lasthandle = nullptr;
				uint8_t count = 1;
				for (handle = _firsthandle; handle; handle = handle->next()) {
					lasthandle = handle;
					count++;
				}

				lasthandle->next(ptr);

				//Serial.printf("[EffectHandler::installProperty] %s property %u: %s\n", name(), count, ptr->name());

			}
		}
	}


template <class T> T get(const char * in)

	{
		Variable<T>* handle = nullptr;

		//handler * handle = nullptr; 

		for (handle = (Variable<T>*)_firsthandle; handle; handle = (Variable<T>*)handle->next()) {
			if (strcmp(handle->name(),in)) {
				return handle->get(); 
			}
		}

		return NULL; 
	}





private:
	handler * _firsthandle;

	//template <class T> Variable<T> * _handlexxx; 

};

class effect: public masterHolderclass
{
	effect()
	{
		add(new Variable<int>("int"));
		add(new Variable<const char *>("const char *"));
		add(new Variable<uint8_t>("uint8_t"));
	}

	void randomfunction()
	{
			int a = get<int>("int");
	}

};


// class PropertyHandlerVariable
// {
// public:
// 	virtual bool addJsonProperty(JsonObject & root) { return false; }
// 	virtual bool parseJsonProperty(JsonObject & root) { return false; }
// 	virtual const char * name() = 0;

// //	virtual T get() = 0;
// //	virtual void set(T) = 0;

// 	PropertyHandlerVariable* next() { return _next; }
// 	void next (PropertyHandlerVariable* next) { _next = next; }
// private:
// 	PropertyHandlerVariable* _next = nullptr;
// };


// template <class T>
// class Property : public PropertyHandlerVariable
// {

// public:
// 	Property(EffectHandler* ptr, const char * name): _name(name)
// 	{
// 		if (ptr) {
// 			ptr->installVariableProperty(this);
// 		}
// 	}
// 	const char * name() { return _name; }

// 	T get()  { return _var; }
// 	void set(T var)  { _var = var; }

// 	bool addJsonProperty(JsonObject & root) override
// 	{
// 		root[_name] = _var;
// 		return true;
// 	}
// 	bool parseJsonProperty(JsonObject & root) override
// 	{
// 		if (root.containsKey(_name)) {
// 			if (_var != root[_name] ) {
// 				_var = root[_name];
// 				return true;
// 			}
// 		}
// 		return false;
// 	}

// 	T _var;
// private:
// 	const char * _name = nullptr;
// };
