#include "Wrappers/ShaderLoader.hpp"

#include <array>
#include <filesystem>

#include "Core/FileManager.hpp"
#include "Core/Debugging/Log.hpp"
#include "Wrappers/WindowManager.hpp"
#include "Core/FileManager.hpp"

using namespace Wrappers;

std::string exec(const std::string& program, const std::string& arg)
{
    Core::FileManager::WriteFile("output.txt", std::vector<u8>());
    Wrappers::WindowManager::ExecuteCommand(program, arg);
    return Core::FileManager::LoadFile("output.txt");
}

std::string Wrappers::ShaderLoader::LoadCompileShader(char const* filename)
{
    std::filesystem::path p = filename;
    if (!std::filesystem::exists(p))
    {
        LOG(DEBUG_LEVEL::LERROR, "File %s does not exist!", filename);
        return "";
    }
    std::string program;
    std::string cmd;
#ifdef _WIN32
    program = "cmd.exe";
    cmd = "/c %VK_SDK_PATH%\\Bin\\glslc.exe";
#else
    program = "/bin/glslc\"";
#endif
    std::string output = std::filesystem::current_path().string() + "/Cache/Shaders/" + p.filename().string() + ".spv";
    cmd += " \"";
    if (p.is_relative())
    {
        cmd += std::filesystem::current_path().string() + "/";
    }
    cmd += p.string();
    cmd += "\" -o \"";
    cmd += output;
    cmd += "\" > output.txt 2>&1";

    std::string result = exec(program, cmd);
    if (result.size())
    {
        std::istringstream st(result);
        std::string line;
        while (std::getline(st, line))
        {
            LOG(DEBUG_LEVEL::LERROR, line.c_str());
        }
    }

	return Core::FileManager::LoadFile(output.c_str());
}
