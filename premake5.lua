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

    -- Enable Extra Warnings, but ignore any unused warnings
    warnings "Extra"
    disablewarnings {
        "unused-parameter",
        "unused-variable",
        "unused-function",
        "unused-value",
        "unused-label",
        "unused-but-set-variable",
        "unused-const-variable",
        "unused-local-typedefs",
        "unused-macros",
        "unused-result",
        "unused-parameter",
        "unused-variable",
        "unused-function",
        "unused-value",
        "unused-label",
        "unused-but-set-variable",
        "unused-const-variable",
        "unused-local-typedefs",
        "unused-macros",
        "unused-result",
        "unused-parameter",
        "unused-variable",
        "unused-function",
        "unused-value",
        "unused-label",
        "unused-but-set-variable",
        "unused-const-variable",
        "unused-local-typedefs",
        "unused-macros",
        "unused-result"
    }

    -- Ignore warnings due to compiler bugs:
    disablewarnings {
        "stringop-truncation",
    }

    -- Treat Warnings as Errors
    flags { "FatalWarnings" }

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

    -- GABUILD (Gable Asset BUILDer) Tool
    project "gabuild"
        kind "ConsoleApp"
        location "./generated/gabuild"
        targetdir "./build/bin/gabuild/%{cfg.buildcfg}"
        objdir "./build/obj/gabuild/%{cfg.buildcfg}"
        includedirs {
            "./projects/gable/include",
            "./projects/gabuild/include"
        }
        files {
            "./projects/gabuild/src/**.c"
        }
        libdirs {
            "./build/bin/gable/%{cfg.buildcfg}"
        }
        links {
            "gable", "m"
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
        
        -- Prebuild Command: Build/Copy Assets
        prebuildcommands {
            "cd ../.. && ./scripts/assets.sh hello %{cfg.buildcfg}"
        }

    -- "Unbricked" - Breakout Clone
    project "unbricked"
        kind "ConsoleApp"
        location "./generated/unbricked"
        targetdir "./build/bin/unbricked/%{cfg.buildcfg}"
        objdir "./build/obj/unbricked/%{cfg.buildcfg}"
        includedirs {
            "./projects/gable/include",
            "./projects/unbricked/include"
        }
        files {
            "./projects/unbricked/src/**.c"
        }
        libdirs {
            "./build/bin/gable/%{cfg.buildcfg}"
        }
        links {
            "gable", "SDL2", "m"
        }
        
        -- Prebuild Command: Build/Copy Assets
        prebuildcommands {
            "cd ../.. && ./scripts/assets.sh unbricked %{cfg.buildcfg}"
        }
        