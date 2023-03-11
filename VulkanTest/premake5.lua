
ShadersDir = "assets/shaders"

project "VulkanTest"
    language "C++"
    cppdialect "C++17"
    targetdir (outputTargetDir)
    objdir (outputObjDir)

    postbuildcommands {
        "{MKDIR} %{ShadersDir}/compiled",
        "%{GLSLC} %{ShadersDir}/TriangleTest.vert -o %{ShadersDir}/compiled/TriangleTest.vert.spv",
        "%{GLSLC} %{ShadersDir}/TriangleTest.frag -o %{ShadersDir}/compiled/TriangleTest.frag.spv",
        "{COPY} %{Library.assimp_dll} %{outputTargetDir}"
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
        "%{IncludeDir.stb}",
        "%{IncludeDir.Spdlog}",
        "%{IncludeDir.TinyObjLoader}",
        "%{IncludeDir.assimp}",
        "%{IncludeDir.Vulkan}"
    }

    links {
        "%{Library.Vulkan}",
        "%{Library.assimp}",
        "%{Library.glfw}"
    }
