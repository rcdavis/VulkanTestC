
VULKAN_SDK = os.getenv("VULKAN_SDK")

IncludeDir = {}
IncludeDir["Spdlog"] = "%{wks.location}/Vendor/Spdlog/src"
IncludeDir["glfw"] = "%{wks.location}/Vendor/glfw/src"
IncludeDir["glm"] = "%{wks.location}/Vendor/glm/src"
IncludeDir["stb"] = "%{wks.location}/Vendor/stb/src"
IncludeDir["assimp"] = "%{wks.location}/Vendor/assimp/src"
IncludeDir["Vulkan"] = "%{VULKAN_SDK}/Include"

Library = {}
Library["glfw"] = "%{wks.location}/Vendor/glfw/lib/%{cfg.platform}/glfw3.lib"
Library["assimp"] = "%{wks.location}/Vendor/assimp/lib/%{cfg.platform}-%{cfg.buildcfg}/assimp-vc143-mtd.lib"
Library["assimp_dll"] = "%{wks.location}/Vendor/assimp/lib/%{cfg.platform}-%{cfg.buildcfg}/assimp-vc143-mtd.dll"
Library["Vulkan"] = "%{VULKAN_SDK}/Lib/vulkan-1.lib"

-- Program Vars
GLSLC = "%{VULKAN_SDK}/Bin/glslc.exe"
