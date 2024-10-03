workspace("PremakeTest")
   configurations({ "Debug", "Release" })
   platforms({ "Win32", "Win64" })
   filter { "platforms:Win32" }
    system "Windows"
    architecture "x86"

   filter { "platforms:Win64" }
    system "Windows"
    architecture "x86_64"

   filter {  }

   includedirs({ "source" })
   includedirs({ "libs/**/include" })
   libdirs({ "libs/**" })
   location("build")

project("TestProj")
   targetname("Game")
   kind("WindowedApp")
   location("build/TestProj")
   language("C++")
   targetdir("bin/%{cfg.buildcfg}")
   objdir("intermediate/%{cfg.buildcfg}")
   debugdir("")
   
   links({ "d3d11", "dxgi", "d3dcompiler", "dxguid", "assimp-vc143-mtd", "DirectXTK", "gtest", "gtest_main" })

   files({ "source/**.h", "source/**.cpp", "source/**.hpp" })

   filter("configurations:Debug")
      defines({ "DEBUG" })
      symbols("On")

   filter("configurations:Release")
      defines({ "NDEBUG" })
      optimize("On")
