#include <arduinojson.h>


class PropertyManager 
{
public:
	PropertyManager(): _firsthandle(nullptr) {}
	add(handler* ptr);
private:
	handelr* _firsthandle; 
};

class handler
{
public:
	handler* next() { return _next; }
	void next (handler* next) { _next = next; }
private:
	handler* _next = nullptr;
};





class effect
{
public:
	effect() {}
private:
};



