
include "Dependencies.lua"

outputTargetDir = "Build/%{cfg.platform}-%{cfg.buildcfg}/Bin"
outputObjDir = "Build/%{cfg.platform}-%{cfg.buildcfg}/Obj"

workspace "VulkanTest"
    startproject "VulkanTest"
    configurations { "Debug", "Release" }
    platforms { "Win64" }
    flags { "MultiProcessorCompile" }

    filter { "platforms:Win64" }
        system "windows"
        architecture "x86_64"
        defines {
            "PLATFORM_WINDOWS",
            "_CRT_SECURE_NO_WARNINGS"
        }
        systemversion "latest"

    filter { "configurations:Debug" }
        kind "ConsoleApp"
        symbols "On"
        defines {
            "DEBUG",
            "LOGGING_ENABLED"
        }

    filter { "configurations:Release" }
        kind "ConsoleApp"
        symbols "On"
        optimize "On"
        defines { "NDEBUG" }

include "VulkanTest"
