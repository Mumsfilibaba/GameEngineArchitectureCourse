workspace "Game Engine Architecture"
    architecture "x64"
    startproject "Assign1_Allocator"
    warnings "extra"

    outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

    configurations
    {
        "Debug",
        "Release",
		"Stack_Test",
		"Pool_Test",
		"Stack_Custom_Test",
		"Pool_Custom_Test",
		"Stack_MT_Test",
		"Pool_MT_Test",
		"Stack_MT_Custom_Test",
		"Pool_MT_Custom_Test"
    }
	
	group "Dependencies"
		include "ImGui"
	group ""
	
	project "*"

	dependson
	{
		"ImGui"
	}

	defines 
	{ 
		"SFML_STATIC" 
	}

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

    filter "configurations:Debug"
        symbols "On"
        runtime "Debug"
        defines
        {
            "DEBUG"
        }

	filter { "configurations:Debug", "system:windows" }
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
		
	filter "configurations:Stack_Test"
        symbols "On"
        runtime "Release"
        optimize "Full"
        defines
        {
            "NDEBUG",
			"TEST_STACK_ALLOCATOR"
        }
		
	filter "configurations:Pool_Test"
        symbols "On"
        runtime "Release"
        optimize "Full"
        defines
        {
            "NDEBUG",
			"TEST_POOL_ALLOCATOR"
        }
		
	filter "configurations:Stack_Custom_Test"
        symbols "On"
        runtime "Release"
        optimize "Full"
        defines
        {
            "NDEBUG",
			"TEST_STACK_ALLOCATOR",
			"USE_CUSTOM_ALLOCATOR"
        }
		
	filter "configurations:Pool_Custom_Test"
        symbols "On"
        runtime "Release"
        optimize "Full"
        defines
        {
            "NDEBUG",
			"TEST_POOL_ALLOCATOR",
			"USE_CUSTOM_ALLOCATOR"
        }
		
	filter "configurations:Stack_MT_Test"
        symbols "On"
        runtime "Release"
        optimize "Full"
        defines
        {
            "NDEBUG",
			"TEST_STACK_ALLOCATOR",
			"MULTI_THREADED"
        }
		
	filter "configurations:Pool_MT_Test"
        symbols "On"
        runtime "Release"
        optimize "Full"
        defines
        {
            "NDEBUG",
			"TEST_POOL_ALLOCATOR",
			"MULTI_THREADED"
        }
		
	filter "configurations:Stack_MT_Custom_Test"
        symbols "On"
        runtime "Release"
        optimize "Full"
        defines
        {
            "NDEBUG",
			"TEST_STACK_ALLOCATOR",
			"USE_CUSTOM_ALLOCATOR",
			"MULTI_THREADED"
        }
		
	filter "configurations:Pool_MT_Custom_Test"
        symbols "On"
        runtime "Release"
        optimize "Full"
        defines
        {
            "NDEBUG",
			"TEST_POOL_ALLOCATOR",
			"USE_CUSTOM_ALLOCATOR",
			"MULTI_THREADED"
        }
	
	filter { "configurations:Release" ,
		"system:windows" }
		links
		{	
			"sfml-graphics-s",
			"sfml-window-s",
			"sfml-system-s",
			"sfml-audio-s",
			"sfml-network-s"
		}
		
	filter { "configurations:Stack_Test",
		"system:windows" }
		links
		{	
			"sfml-graphics-s",
			"sfml-window-s",
			"sfml-system-s",
			"sfml-audio-s",
			"sfml-network-s"
		}
		
	filter { "configurations:Pool_Test",
		"system:windows" }
		links
		{	
			"sfml-graphics-s",
			"sfml-window-s",
			"sfml-system-s",
			"sfml-audio-s",
			"sfml-network-s"
		}
		
	filter { "configurations:Stack_Custom_Test",
		"system:windows" }
		links
		{	
			"sfml-graphics-s",
			"sfml-window-s",
			"sfml-system-s",
			"sfml-audio-s",
			"sfml-network-s"
		}
		
	filter { "configurations:Pool_Custom_Test",
		"system:windows" }
		links
		{	
			"sfml-graphics-s",
			"sfml-window-s",
			"sfml-system-s",
			"sfml-audio-s",
			"sfml-network-s"
		}
		
	filter { "configurations:Stack_MT_Test",
		"system:windows" }
		links
		{	
			"sfml-graphics-s",
			"sfml-window-s",
			"sfml-system-s",
			"sfml-audio-s",
			"sfml-network-s"
		}
		
	filter { "configurations:Pool_MT_Test",
		"system:windows" }
		links
		{	
			"sfml-graphics-s",
			"sfml-window-s",
			"sfml-system-s",
			"sfml-audio-s",
			"sfml-network-s"
		}
		
	filter { "configurations:Stack_MT_Custom_Test",
		"system:windows" }
		links
		{	
			"sfml-graphics-s",
			"sfml-window-s",
			"sfml-system-s",
			"sfml-audio-s",
			"sfml-network-s"
		}
		
	filter { "configurations:Pool_MT_Custom_Test",
		"system:windows" }
		links
		{	
			"sfml-graphics-s",
			"sfml-window-s",
			"sfml-system-s",
			"sfml-audio-s",
			"sfml-network-s"
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
