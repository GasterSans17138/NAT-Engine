#include "Core/Signal.hpp"

using namespace Core;

Signal::Signal() : value(false)
{
}

Signal::Signal(bool defaultValue) : value(defaultValue)
{
}

Signal::~Signal()
{
}

bool Signal::Store(bool valueIn)
{
	value.store(valueIn);
	return valueIn;
}

bool Signal::Load() const
{
	return value.load();
}

bool Signal::operator()()
{
	return Load();
}

bool Signal::operator=(bool newValue)
{
	value.store(newValue);
	return Load();
}
