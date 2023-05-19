-- premake5.lua
workspace "8599RayTracerGUI"
   architecture "x64"
   configurations { "Debug", "Release", "Dist" }
   startproject "8599RayTracerGUI"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
include "Walnut/WalnutExternal.lua"

include "8599RayTracerGUI"