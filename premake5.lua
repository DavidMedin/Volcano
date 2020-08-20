workspace"Volcano"
	configurations {"Debug","Release"}
	architecture "x86_64"
project "Volcano"
	location "Volcano"
	objdir "%{prj.name}/tmp"
	kind "ConsoleApp"
	language "C"
	targetdir "bin/%{cfg.buildcfg}"
	
	files {"Volcano/src/*.*"}--includes all the c/h files
	includedirs{"Volcano/src/","Volcano/libs/**","$(VULKAN_SDK)/include"}
	libdirs {"Volcano/libs/**","$(VULKAN_SDK)/lib"}
	links{"glfw3dll","vulkan-1"}

	filter "Debug"
		defines {"_DEBUG","_CONSOLE"}
		symbols "On"
	filter "Release"
		defines{"NDEBUG"}
		optimize "On"