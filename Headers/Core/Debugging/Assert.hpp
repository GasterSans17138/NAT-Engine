#pragma once

#include <cassert>
#include <stdlib.h>

#ifndef NDEBUG
#define Assert(x) assert(x)
#else
#define Assert(x) ((void)0);
#endif