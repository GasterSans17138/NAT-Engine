#pragma once

#include "Core/Types.hpp"

#include "Resources/Shader.hpp"

namespace Wrappers
{
	class NAT_API ShaderLoader
	{
	public:
		ShaderLoader() = default;
		~ShaderLoader() = default;

		static std::string LoadCompileShader(char const* filename);
	};
}
