project "Glad"
    kind "StaticLib"
    language "C"

	-- Directories
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "Include/glad/glad.h",
        "Include/KHR/khrplatform.h",
        "Src/glad.c"
    }
    includedirs
    {
        "Include"
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