
VULKAN_SDK = os.getenv("VULKAN_SDK")

IncludeDir = {}
IncludeDir["Spdlog"] = "%{wks.location}/Vendor/Spdlog/src"
IncludeDir["glfw"] = "%{wks.location}/Vendor/glfw/src"
IncludeDir["glm"] = "%{wks.location}/Vendor/glm/src"
IncludeDir["stb_image"] = "%{wks.location}/Vendor/stb_image/src"
IncludeDir["TinyObjLoader"] = "%{wks.location}/Vendor/TinyObjLoader/src"
IncludeDir["Vulkan"] = "%{VULKAN_SDK}/Include"

Library = {}
Library["glfw"] = "%{wks.location}/Vendor/glfw/lib/%{cfg.platform}/glfw3.lib"
Library["Vulkan"] = "%{VULKAN_SDK}/Lib/vulkan-1.lib"

-- Program Vars
GLSLC = "%{VULKAN_SDK}/Bin/glslc.exe"
