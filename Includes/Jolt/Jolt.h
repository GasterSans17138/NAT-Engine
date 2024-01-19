// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#ifdef _DEBUG 
	#define JPH_PROFILE_ENABLED
#endif // _DEBUG 

// Project includes
#include <Jolt/Core/Core.h>

JPH_SUPPRESS_WARNINGS

#pragma warning(disable:4251)
#pragma warning(disable:4275)

#include <Jolt/Core/ARMNeon.h>
#include <Jolt/Core/Memory.h>
#include <Jolt/Core/STLAllocator.h>
#include <Jolt/Core/IssueReporting.h>
#include <Jolt/Math/Math.h>
#include <Jolt/Math/Vec4.h>
#include <Jolt/Math/Mat44.h>
