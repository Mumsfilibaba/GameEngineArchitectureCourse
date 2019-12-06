project "tinyxml2"
    kind "StaticLib"
    language "C++"

	-- Directories
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	-- All Files
    files
    {
        "tinyxml2.h",
		"tinyxml2.cpp",
    }
    
	-- Windows specific
    filter "system:windows"
        systemversion "latest"
        cppdialect "C++17"
        staticruntime "Off"
		
	-- macOS specific
    filter "system:macosx"
        systemversion "latest"
        cppdialect "C++17"