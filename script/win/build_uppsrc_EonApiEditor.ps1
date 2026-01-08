# PowerShell script to build EonApiEditor
$PSScriptRoot = Split-Path -Parent $MyInvocation.MyCommand.Definition
& "$PSScriptRoot\build_upptst_eon_generic.ps1" EonApiEditor "" @args
