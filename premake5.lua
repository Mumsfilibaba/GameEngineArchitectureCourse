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
	
	group "Dependencies"
		include "ImGui"
	group ""
	
	project "*"

	filter "system:windows"
		sysincludedirs
		{
			"Dependencies/SFML-2.5.1/include",
			"ImGui"
		}
		libdirs
		{
			"Dependencies/SFML-2.5.1/lib"
		}
		links
		{
			"opengl32",
			"freetype",
			"winmm",
			"gdi32",
			"flac",
			"vorbisenc",
			"vorbisfile",
			"vorbis",
			"ogg",
			"ws2_32"
		}

	filter "configurations:*"
		defines 
		{ 
			"SFML_STATIC" 
		}

    filter "configurations:Debug"
        symbols "On"
        runtime "Debug"
        defines
        {
            "DEBUG"
        }
		links
		{	
			"sfml-graphics-s-d",
			"sfml-window-s-d",
			"sfml-system-s-d",
			"sfml-audio-s-d",
			"sfml-network-s-d"
		}

    filter "configurations:Release"
        symbols "On"
        runtime "Release"
        optimize "Full"
        defines
        {
            "NDEBUG"
        }
		links
		{	
			"sfml-graphics-s",
			"sfml-window-s",
			"sfml-system-s",
			"sfml-audio-s",
			"sfml-network-s"
		}
		
	dependson
	{
		"ImGui"
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
		
		links
		{
			"ImGui"
		}
