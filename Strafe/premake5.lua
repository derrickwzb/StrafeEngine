include "../dependencies.lua"

project "Strafe"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"
	warnings "Extra"
	externalwarnings "Off"

	targetdir ("%{wks.location}/build/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/build-int/" .. outputdir .. "/%{prj.name}")

	pchheader "strafepch.h"
	pchsource "src/strafepch.cpp"

	flags
	{
		"MultiProcessorCompile"
	}

	files
	{
		"src/**.h",
		"src/**.cpp"
	}

    defines
    {
        "_CRT_SECURE_NO_WARNINGS"
    }

	libdirs
	{
		
	}

	links
	{
		"GLFW",
		"Glad"
	}

    includedirs
    {
        "src",
		"%{IncludesDir.spdlog}",
		"%{IncludesDir.glfw}",
		"%{IncludesDir.glad}",
		"%{IncludesDir.glm}",
		"%{IncludesDir.entt}",
    }

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		defines "STRAFE_DEBUG"
		runtime "Debug"
		optimize "off"
		symbols "on"

	filter "configurations:Release"
		defines "STRAFE_RELEASE"
		runtime "Release"
		optimize "on"
		symbols "off"
