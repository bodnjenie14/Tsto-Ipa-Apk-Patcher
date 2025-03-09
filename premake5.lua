dependencies = {
	basePath = "./deps"
}

function dependencies.load()
	dir = path.join(dependencies.basePath, "premake/*.lua")
	deps = os.matchfiles(dir)
	for i, dep in pairs(deps) do
		dep = dep:gsub(".lua", "")
		require(dep)
	end
end

function dependencies.imports()
	for i, proj in pairs(dependencies) do
		if type(i) == 'number' then
			proj.import()
		end
	end
end

function dependencies.projects()
    for i, proj in pairs(dependencies) do
        if type(i) == 'number' and proj.project then  -- Check if 'project' function exists
            proj.project()  -- Only call if the project function exists
        end
    end
end


newoption {
	trigger = "copy-to",
	description = "Optional, copy the EXE to a custom folder after build, define the path here if wanted.",
	value = "PATH"
}

newoption {
	trigger = "dev-build",
	description = "Enable development builds of the client."
}

dependencies.load()

-- Define the main project 

workspace "Tsto_patcher"

	startproject "patcher"
	location "./build"
	objdir "%{wks.location}/obj"
	targetdir "%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}"

	configurations {"Debug", "Release"}

	language "C++"
	cppdialect "C++20"

	architecture "x86_64"
	platforms "x64"
	staticruntime "off"
    runtime "Release"

	systemversion "latest"
	symbols "On"
	editandcontinue "Off"
	warnings "Extra"
	characterset "ASCII"

	if _OPTIONS["dev-build"] then
		defines {"DEV_BUILD"}
	end

	if os.getenv("CI") then
		defines {"CI"}
	end

    flags { "NoIncrementalLink", "NoMinimalRebuild", "MultiProcessorCompile", "No64BitChecks" }

	filter "platforms:x64"
		defines {"_WINDOWS", "WIN32"}
	filter {}

	filter "configurations:Release"
		optimize "Size"
		buildoptions {"/GL", "/Zc:__cplusplus"}
		linkoptions { "/IGNORE:4702", "/LTCG" }
		defines {"NDEBUG"}
	filter {}

	filter "configurations:Debug"
		--optimize "Debug"
		--defines {"DEBUG", "_DEBUG"}
                runtime "Debug"
		defines { "_DEBUG" }
                buildoptions { "/Zc:__cplusplus" }
   	        symbols "On"
	filter {}

		
-- Define the utlis project
project "utilities"
	kind "StaticLib"
	language "C++"

	files {"./source/utilities/**.hpp", "./source/utilities/**.cpp"}

	includedirs {"./source/utilities", "%{prj.location}/source"}

	dependencies.imports()
	
	
-- Define the server project
project "patcher"
    kind "WindowedApp"
    language "C++"
    targetname "tsto_patcher"
    cppdialect "C++20"

    pchheader "std_include.hpp"
    pchsource "source/patcher/std_include.cpp"
    
    includedirs
    {
        "source/patcher",
        "./source/utilities", 
	"build/src/", -- for pre generated qt moc files [NEEDED]
        "%{prj.location}/source"
    }

    files 
    {
        "./source/patcher/resources.rc",
        "./source/patcher/**.hpp",
        "./source/patcher/**.cpp",
        "./source/patcher/**.h",
        "./source/patcher/**.ui",
        "./source/patcher/**.qrc",
        "./source/patcher/**.ico",
        "./source/patcher/**.manifest",
        "build/src/**.cpp"
    }

    links {
        "utilities",  -- Links with utilities

    }
	

    -- Apply precompiled headers to moc files
    filter "files:src/patcher/patcher/**.cpp"
    filter "files:src/patcher/main/**.cpp"
    -- pchheader "std_include.hpp"
    -- pchsource "src/server/std_include.cpp"

    filter {}
	
	
	qt6.run_moc()  -- Func to run moc NEEDED this builds the moc file (moc is in qt.lua)

	
    dependencies.imports()

group "Dependencies"
    dependencies.projects()