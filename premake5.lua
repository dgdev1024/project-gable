-- @file premake5.lua

-- Workspace Settings
workspace "project-gable"
    language "C"
    cdialect "C17"
    location "./generated"
    configurations { "debug", "release", "distribute" }
    filter { "configurations:debug" }
        defines { "GABLE_DEBUG" }
        symbols "On"
    filter { "configurations:release" }
        defines { "GABLE_RELEASE" }
        optimize "On"
    filter { "configurations:distribute" }
        defines { "GABLE_DISTRIBUTE" }
        optimize "On"
    filter { "system:linux" }
        defines { "GABLE_LINUX" }
        cdialect "gnu17"
    filter {}

    -- GABLE (GAmeBoy-Like Engine) Library
    project "gable"
        kind "SharedLib"
        location "./generated/gable"
        targetdir "./build/bin/gable/%{cfg.buildcfg}"
        objdir "./build/obj/gable/%{cfg.buildcfg}"
        includedirs {
            "./projects/gable/include"
        }
        files {
            "./projects/gable/src/**.c"
        }
        
    -- Hello, World! Example
    project "hello"
        kind "ConsoleApp"
        location "./generated/hello"
        targetdir "./build/bin/hello/%{cfg.buildcfg}"
        objdir "./build/obj/hello/%{cfg.buildcfg}"
        includedirs {
            "./projects/gable/include",
            "./projects/hello/include"
        }
        files {
            "./projects/hello/src/**.c"
        }
        libdirs {
            "./build/bin/gable/%{cfg.buildcfg}"
        }
        links {
            "gable", "SDL2", "m"
        }
        