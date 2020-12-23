workspace"Volcano"
	configurations {"Debug","Release"}
	architecture "x86_64"
project "Volcano"
	location "Volcano"
	objdir "%{prj.name}/tmp"
	kind "ConsoleApp"
	language "C++"
	targetdir "bin/%{cfg.buildcfg}"
	
	files {"Volcano/src/*.*","Volcano/libs/SPIRV-Reflect/spirv_reflect.c","Volcano/libs/SPIRV-Reflect/spirv_reflect.h"}--includes all the c/h files

	-- group "Spirv-Reflect"
	-- 	files {"Volcano/src/libs/SPIRV-Reflect/*.*"}
	-- group ""
	includedirs{"Volcano/src/","Volcano/libs/**","$(VULKAN_SDK)/include"}
	libdirs {"Volcano/libs/**","$(VULKAN_SDK)/lib"}
	links{"glfw3dll","vulkan-1","shaderc_shared"}

	filter "Debug"
		defines {"_DEBUG","_CONSOLE"}
		symbols "On"
	filter "Release"
		defines{"NDEBUG"}
		optimize "On"