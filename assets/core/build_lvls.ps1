﻿param([string]$BinPath,
      [string]$CompilerArgs)

$old_path = $env:Path

$env:Path += ";" + $BinPath

$compile_expression =  "shader_compiler --outputdir munged\ --declarationinputdir declarations\ --definitioninputdir definitions\ --hlslinputdir src\" + " $CompilerArgs"
Invoke-Expression $compile_expression | Write-Host

sp_texture_munge --outputdir "munged\" --sourcedir "textures\"
lvl_pack -i "munged\" --sourcedir ".\" --outputdir ".\"

$env:Path = $old_path