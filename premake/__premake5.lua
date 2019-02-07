-- premake5.lua
cppdialect "C++17"

workspace "nitro"
   configurations { "Debug", "Release" }

project "nitro"
   kind "SharedLib"
   language "C++"
   targetdir "bin/%{cfg.buildcfg}"
   includedirs { "nitro", "nitro/include" }
   files { "nitro/*.h", "nitro/*.cpp" }

   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"
      
project "test_nitro"
   kind "ConsoleApp"
   language "C++"
   targetdir "bin/%{cfg.buildcfg}"
   includedirs { "nitro/include" }
   links { "gtest", "gtest_main" }
   files { "tests/*.h", "tests/*.cpp", "lib/performatics/*.cpp"}

   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"
