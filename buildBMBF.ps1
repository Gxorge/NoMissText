# Builds a .zip file for loading with BMBF
& $PSScriptRoot/build.ps1

if ($?) {
    Compress-Archive -Path "./libs/arm64-v8a/libnomisstext.so", "./bmbfmod.json", "./cover.jpg" -DestinationPath "./nomisstext_v2.0.0.zip" -Update
}
