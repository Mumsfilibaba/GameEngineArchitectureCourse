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
    links 
    {
        "OpenGL.framework",
        "sfml-window.2.5.1",
        "sfml-system.2.5.1",
        "sfml-audio.2.5.1",
        "sfml-graphics.2.5.1",
        "sfml-network.2.5.1"
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

-- Setup configurations for Release and windows
filter { "configurations:Release", "system:windows" }
    links
    {	
        "sfml-graphics-s",
        "sfml-window-s",
        "sfml-system-s",
        "sfml-audio-s",
        "sfml-network-s"
    }