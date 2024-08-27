project "Editor"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"
	warnings "Extra"

	targetdir ("%{wks.location}/build/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/build-int/" .. outputdir .. "/%{prj.name}")

	flags
	{
		"MultiProcessorCompile"
	}

	files
	{
		"src/**.h",
		"src/**.cpp",
	}

	dependson
	{
		
	}

	links
	{
		"Strafe",
		"imgui"
	}

    includedirs
    {
        "%{wks.location}/Strafe/src",
		"%{IncludesDir.glm}",
		"%{IncludesDir.spdlog}",
		"%{IncludesDir.entt}",
		"%{IncludesDir.imgui}",
		"%{IncludesDir.glfw}",
		"%{IncludesDir.glad}",
    }

	postbuildcommands
	{
		
	}

	filter "system:windows"
		systemversion "latest"
		
	filter "configurations:Debug"
		defines "STRAFE_EDITOR_DEBUG"
		runtime "Debug"
		optimize "off"
		symbols "on"
		kind "ConsoleApp"

	filter "configurations:Release"
		defines "STRAFE_EDITOR_RELEASE"
		runtime "Release"
		optimize "on"
		symbols "off"
		kind "WindowedApp"
		entrypoint "mainCRTStartup"