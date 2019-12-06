workspace "Game Engine Architecture"
    architecture "x64"
    startproject "Assign2_ResourceManager"
    warnings "extra"
	staticruntime "off"

	-- Setup output dirs
    outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

	-- Setup configurations for all projects
    configurations
    {
        "Debug",
        "Release",
	}
	defines 
	{ 
		"SFML_STATIC",
		"_CRT_SECURE_NO_WARNINGS",
	}

	-- Defines for when using visual studio
	filter "action:vs*"
		defines 
		{ 
			"VISUAL_STUDIO"
		}

	-- Setup configurations for Debug for all projects
	filter "configurations:Debug"
        symbols "On"
        runtime "Debug"
        defines
        {
			"DEBUG",
			"_CRTDBG_MAP_ALLOC",
		}

	-- Setup configurations for Release for all projects
	filter "configurations:Release" 
		symbols "On"
		runtime "Release"
		optimize "Full"
		defines
		{
			"NDEBUG"
		}

	-- Setup configurations for macos
	filter "system:macosx"
		sysincludedirs
		{
			"/usr/local/include",
			"Dependencies/ImGui",
			"Dependencies/glm",
			"Dependencies/Glad/Include",
			"Dependencies/tinyxml2/",
		}
		libdirs
		{
			"/usr/local/Cellar/sfml/2.5.1/lib/"
		}
		
	-- Setup configurations for windows
	filter "system:windows"
		sysincludedirs
		{
			"Dependencies/SFML-2.5.1/include",
			"Dependencies/zlib/include",
			"Dependencies/ImGui",
			"Dependencies/glm",
			"Dependencies/Glad/Include",
			"Dependencies/tinyxml2/",
		}
		libdirs
		{
			"Dependencies/SFML-2.5.1/lib",
			"Dependencies/zlib/lib",
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
			"Assignment1/**.h",
			"Base/**.cpp",
			"Base/**.h",
        }
		links
		{
			"ImGui",
			"Glad",
		}
		includedirs
		{
			"Base"
		}
		
		--[[configurations
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
		--]]

		-- Setup configurations for different tests
		filter "configurations:Stack_Test or Pool_Test or Stack_Custom_Test or Pool_Custom_Test_8192_Chunk or Pool_Custom_Test_4096_Chunk or Pool_Custom_Test_16384_Chunk or Pool_MT_Test or Stack_MT_Test or Stack_MT_Custom_Test or Pool_MT_Custom_Test_8192_Chunk or Pool_MT_Custom_Test_4096_Chunk or Pool_MT_Custom_Test_16384_Chunk" 
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

		-- Run script to setup SFML (To avoid duplication of code)
		dofile("premake5_setupSFML.lua")
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
			"Assignment2/**.h",
			"Base/**.cpp",
			"Base/**.h",
        }
		filter { "configurations:Release", "system:windows" }
			links
			{
				"zlibstatic"
			}
		filter { "configurations:Debug", "system:windows" }
			links
			{
				"zlibstaticd"
			}
		filter "system:macosx"
			links
			{
				"z"
			}
			defines
			{
				"MACOS"
			}
		filter {}
		links
		{
			"ImGui",
			"Glad",
			"tinyxml2",
		}
		includedirs
		{
			"Base"
		}

		-- Run script to setup SFML (To avoid duplication of code)
		dofile("premake5_setupSFML.lua")
	project "*"

	-- Setup Dependency projects
	group "Dependencies"
		include "Dependencies/ImGui"
		include "Dependencies/Glad"
		include "Dependencies/tinyxml2"
	group ""