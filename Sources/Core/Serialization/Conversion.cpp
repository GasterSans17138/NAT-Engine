#include "Core/Serialization/Conversion.hpp"

#ifdef _WIN32
#pragma comment(lib, "Ws2_32.lib")
#define NOMINMAX
#include <WinSock2.h>
#else
#include <arpa/inet.h>
#endif

namespace Core::Serialization::Conversion
{
	void ToNetwork(u16 from, u16& to)
	{
		to = htons(from);
	}
	void ToNetwork(u32 from, u32& to)
	{
		to = htonl(from);
	}
	void ToNetwork(u64 from, u64& to)
	{
#ifdef _WIN32
		to = htonll(from);
#else
		to = htobe64(from);
#endif
	}
	void ToNetwork(f32 from, u32& to)
	{
#ifdef _WIN32
		to = htonf(from);
#else
    	union {
        	u32 l;
        	f32 d;
    	} tmp;
    	tmp.d = from;
    	ToNetwork(tmp.l, to);
#endif
	}
	void ToNetwork(f64 from, u64& to)
	{
#ifdef _WIN32
		to = htond(from);
#else
    	union {
        	u64 l;
        	f64 d;
    	} tmp;
    	tmp.d = from;
    	ToNetwork(tmp.l, to);
#endif
	}

	void ToLocal(u16 from, u16& to)
	{
		to = ntohs(from);
	}
	void ToLocal(u32 from, u32& to)
	{
		to = ntohl(from);
	}
	void ToLocal(u64 from, u64& to)
	{
#ifdef _WIN32
		to = ntohll(from);
#else
		to = be64toh(from);
#endif
	}
	void ToLocal(u32 from, f32& to)
	{
#ifdef _WIN32
		to = ntohf(from);
#else
    	union {
        	u32 l;
        	f32 d;
    	} tmp;
    	ToLocal(from, tmp.l);
    	to = tmp.d;
#endif
	}
	void ToLocal(u64 from, f64& to)
	{
#ifdef _WIN32
		to = ntohd(from);
#else
    	union {
        	u64 l;
        	f64 d;
    	} tmp;
    	ToLocal(from, tmp.l);
    	to = tmp.d;
#endif
	}
}