project("Engine")
  targetname("Engine")
  kind("WindowedApp")  -- TODO: replace with SharedLib/StaticLib when scripting prototype is ready
  location("../build/Engine")
  language("C++")
  debugdir("")
  
  includedirs({ "source" })
  includedirs({ "libs/**/include" })
  
  filter("configurations:Debug*")
    libdirs({ "libs/**/debug" })
	links({ "assimp-vc143-mtd" })
  
  filter("configurations:Release*")
    libdirs({ "libs/**/release" })
	links({ "assimp-vc143-mt" })
  
  filter {  }
  
  links({ "d3d11", "dxgi", "d3dcompiler", "dxguid", "DirectXTK", "gtest", "gtest_main" })
  
  files({ "source/**.h", "source/**.cpp", "source/**.hpp" })
  