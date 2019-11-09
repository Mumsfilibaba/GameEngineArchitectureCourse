workspace "Game Engine Architecture"
    architecture "x64"
    startproject "Assign1_Allocator"
    warnings "extra"

    outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

    configurations
    {
        "Debug",
        "Release"
    }

    filter "configurations:Debug"
        symbols "On"
        runtime "Debug"
        defines
        {
            "DEBUG"
        }

    filter "configurations:Release"
        symbols "On"
        runtime "Release"
        optimize "Full"
        defines
        {
            "NDEBUG"
        }

    project "Assign1_Allocator"
        kind "ConsoleApp"
        language "C++"
        location "Assignment1"
        cppdialect "C++17"
        systemversion "latest"
        files
        {
            "Assignment1/**.cpp",
            "Assignment1/**.h"
        }
