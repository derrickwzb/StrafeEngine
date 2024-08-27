include "Tools/dependencies.lua"

workspace "Strafe"
	architecture "x64"
	startproject "Editor"

	configurations
	{
		"Debug",
		"Release"
	}

outputdir = "%{cfg.buildcfg}"

group ""
	include "Strafe"

group "Dependencies"
	include "Strafe/dependencies/glfw"
	include "Strafe/dependencies/glad"

group "Application"
	include "Editor"
	include "Launcher"
group "Application/Dependencies"
	include "Editor/dependencies/imgui"