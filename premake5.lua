workspace "Game Engine Architecture"
    architecture "x64"
    startproject "Assign2_Allocator"
    warnings "extra"

	-- Setup output dirs
    outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

	-- Setup configurations for all projects
    configurations
    {
        "Debug",
        "Release"
    }
	defines 
	{ 
		"SFML_STATIC" 
	}
	files
	{
		"Base/**.cpp",
		"Base/**.h"
	}

	-- Setup configurations for windows
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

	-- Setup configurations for macos
	filter "system:macosx"
		libdirs
		{
			"/usr/local/Cellar/sfml/2.5.1/lib/"
		}
		sysincludedirs
		{
			"/usr/local/include",
			"ImGui"
		}
		links 
		{
			"OpenGL.framework",
			"sfml-window.2.5.1",
			"sfml-system.2.5.1",
			"sfml-audio.2.5.1",
			"sfml-graphics.2.5.1",
			"sfml-network.2.5.1"
		} 

	-- Setup configurations for Debug
    filter "configurations:Debug"
        symbols "On"
        runtime "Debug"
        defines
        {
            "DEBUG"
        }

	-- Setup configurations for Debug and Windows
	filter { "configurations:Debug", "system:windows" }
		links
		{	
			"sfml-graphics-s-d",
			"sfml-window-s-d",
			"sfml-system-s-d",
			"sfml-audio-s-d",
			"sfml-network-s-d"
		}

	-- Setup configurations for Release
    filter "configurations:Release or Stack_Test or Pool_Test or Stack_Custom_Test or Pool_Custom_Test_8192_Chunk or Pool_Custom_Test_4096_Chunk or Pool_Custom_Test_16384_Chunk or Pool_MT_Test or Stack_MT_Test or Stack_MT_Custom_Test or Pool_MT_Custom_Test_8192_Chunk or Pool_MT_Custom_Test_4096_Chunk or Pool_MT_Custom_Test_16384_Chunk" 
        symbols "On"
        runtime "Release"
        optimize "Full"
        defines
        {
            "NDEBUG"
        }
		
	filter "configurations:Stack_Test or Stack_Custom_Test or Stack_MT_Test or Stack_MT_Custom_Test"
		defines
        {
			"TEST_STACK_ALLOCATOR"
        }
		
	filter "configurations:Pool_Test or Pool_Custom_Test_8192_Chunk or Pool_Custom_Test_4096_Chunk or Pool_Custom_Test_16384_Chunk or Pool_MT_Test or Pool_MT_Custom_Test_8192_Chunk or Pool_MT_Custom_Test_4096_Chunk or Pool_MT_Custom_Test_16384_Chunk"
        defines
		{
			"TEST_POOL_ALLOCATOR"
        }
		
	filter "configurations:Stack_Custom_Test or Pool_Custom_Test_8192_Chunk or Pool_Custom_Test_4096_Chunk or Pool_Custom_Test_16384_Chunk or Stack_MT_Custom_Test or Pool_MT_Custom_Test_8192_Chunk or Pool_MT_Custom_Test_4096_Chunk or Pool_MT_Custom_Test_16384_Chunk"
        defines
        {
			"USE_CUSTOM_ALLOCATOR"
        }
		
	filter "configurations:Stack_MT_Test or Pool_MT_Test or Stack_MT_Custom_Test or Pool_MT_Custom_Test_8192_Chunk or Pool_MT_Custom_Test_4096_Chunk or Pool_MT_Custom_Test_16384_Chunk"
        defines
        {
			"MULTI_THREADED"
        }
			
	filter "configurations:Pool_Custom_Test_4096_Chunk or Pool_MT_Custom_Test_4096_Chunk"
        defines
        {
			"CONFIG_CHUNK_SIZE_4096"
        }
		
	filter "configurations:Pool_Custom_Test_8192_Chunk or Pool_MT_Custom_Test_8192_Chunk"
        defines
        {
			"CONFIG_CHUNK_SIZE_8192"
        }
		
	filter "configurations:Pool_Custom_Test_16384_Chunk or Pool_MT_Custom_Test_16384_Chunk"
        defines
        {
			"CONFIG_CHUNK_SIZE_16384"
        }

	-- Setup configurations for Release and windows
	filter { "configurations:Release or Stack_Test or Pool_Test or Stack_Custom_Test or Pool_Custom_Test_8192_Chunk or Pool_Custom_Test_4096_Chunk or Pool_Custom_Test_16384_Chunk or Stack_MT_Test or Pool_MT_Test or Stack_MT_Custom_Test or Pool_MT_Custom_Test_8192_Chunk or Pool_MT_Custom_Test_4096_Chunk or Pool_MT_Custom_Test_16384_Chunk" ,
		"system:windows" }
		links
		{	
			"sfml-graphics-s",
			"sfml-window-s",
			"sfml-system-s",
			"sfml-audio-s",
			"sfml-network-s"
		}

	-- Setup Assignment 1
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
		configurations
		{
			"Stack_Test",
			"Pool_Test",
			"Stack_Custom_Test",
			"Pool_Custom_Test_4096_Chunk",
			"Pool_Custom_Test_8192_Chunk",
			"Pool_Custom_Test_16384_Chunk",
			"Stack_MT_Test",
			"Pool_MT_Test",
			"Stack_MT_Custom_Test",
			"Pool_MT_Custom_Test_4096_Chunk",
			"Pool_MT_Custom_Test_8192_Chunk",
			"Pool_MT_Custom_Test_16384_Chunk",
		}
	project "*"

	-- Setup Assignment 2
	project "Assign2_ResourceManager"
		kind "ConsoleApp"
		language "C++"
		location "Assignment2"
		cppdialect "C++17"
		systemversion "latest"
		files
        {
            "Assignment2/**.cpp",
            "Assignment2/**.h"
        }
		links
		{
			"ImGui"
		}
	project "*"

	group "Dependencies"
		include "ImGui"
	group ""
