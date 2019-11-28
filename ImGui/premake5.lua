project "ImGui"
    kind "StaticLib"
    language "C++"

	-- Directories
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	-- All Files
    files
    {
        "imconfig.h",
		"imgui-SFML.h",
		"imgui-SFML.cpp",
        "imgui.h",
        "imgui.cpp",
        "imgui_draw.cpp",
        "imgui_internal.h",
        "imgui_widgets.cpp",
        "imstb_rectpack.h",
        "imstb_textedit.h",
        "imstb_truetype.h",
        "imgui_demo.cpp"
    }
    
	-- Windows specific
    filter "system:windows"
        systemversion "latest"
        cppdialect "C++17"
        staticruntime "Off"
		
	-- Windows specific
    filter "system:macosx"
        systemversion "latest"
        cppdialect "C++17"
		
project "*"