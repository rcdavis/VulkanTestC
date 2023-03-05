
ShadersDir = "assets/shaders"

project "VulkanTest"
    language "C++"
    cppdialect "C++17"
    targetdir (outputTargetDir)
    objdir (outputObjDir)

    postbuildcommands {
        "{MKDIR} %{ShadersDir}/compiled",
        "%{GLSLC} %{ShadersDir}/TriangleTest.vert -o %{ShadersDir}/compiled/TriangleTest.vert.spv",
        "%{GLSLC} %{ShadersDir}/TriangleTest.frag -o %{ShadersDir}/compiled/TriangleTest.frag.spv"
    }

    cleancommands {
        "{RMDIR} %{ShadersDir}/compiled"
    }

    files {
        "src/**.h",
        "src/**.cpp"
    }

    includedirs {
        "src",
        "%{IncludeDir.glfw}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.stb_image}",
        "%{IncludeDir.Spdlog}",
        "%{IncludeDir.Vulkan}"
    }

    links {
        "%{Library.Vulkan}",
        "%{Library.glfw}"
    }
