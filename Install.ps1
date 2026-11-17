param([switch]$Elevated)
$ErrorActionPreference='Stop'
$bundleName='DIAMOND I - HARMONIC EMULATION.vst3'
$sourceBundle=Join-Path $PSScriptRoot $bundleName
if(-not (Test-Path -LiteralPath $sourceBundle)) {
 $sourceBundle=Join-Path $PSScriptRoot ('build/DiamondI_artefacts/Release/VST3/'+$bundleName)
}
if(-not (Test-Path -LiteralPath $sourceBundle)){throw 'No se encuentra el bundle VST3 junto al instalador ni en build.'}
$isAdmin=([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
if(-not $isAdmin){
 if($Elevated){throw 'Windows no ha concedido permisos de administrador.'}
 $process=Start-Process -FilePath (Get-Process -Id $PID).Path -ArgumentList @('-NoProfile','-ExecutionPolicy','Bypass','-File',('"'+$PSCommandPath+'"'),'-Elevated') -Verb RunAs -WindowStyle Hidden -Wait -PassThru
 exit $process.ExitCode
}
$vst3Root=Join-Path ${env:CommonProgramFiles} 'VST3'
New-Item -ItemType Directory -Force -Path $vst3Root | Out-Null
$targetBundle=Join-Path $vst3Root $bundleName
$relativeDll='Contents/x86_64-win/'+$bundleName
Copy-Item -LiteralPath $sourceBundle -Destination $vst3Root -Recurse -Force
$expected=(Get-FileHash -LiteralPath (Join-Path $sourceBundle $relativeDll)).Hash
$actual=(Get-FileHash -LiteralPath (Join-Path $targetBundle $relativeDll)).Hash
if($expected -ne $actual){throw 'La copia instalada no coincide con el binario validado.'}
if(-not (Test-Path -LiteralPath (Join-Path $targetBundle 'Contents/Resources/moduleinfo.json'))){throw 'Falta el manifiesto VST3.'}
Write-Output ('Instalado: '+$targetBundle)
Write-Output ('SHA256: '+$actual)
