workspace "spdlog"
	configurations {"release"}

project "spdlog"
	kind "StaticLib"
	language "C++"

	objdir "build"
	targetdir "lib"
	targetname "spdlog"