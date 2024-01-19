#pragma once

#include <atomic>

#ifdef NAT_EngineDLL
	#define NAT_API __declspec(dllexport)
#else
	#define NAT_API __declspec(dllimport)
#endif // NAT_EngineDLL

namespace Core
{
	class NAT_API Signal
	{
	public:
		Signal();
		Signal(bool defaultValue);
		~Signal();

		bool Store(bool valueIn);
		bool Load() const;

		bool operator()();
		bool operator=(bool newValue);
	private:
		std::atomic<bool> value;
	};
}