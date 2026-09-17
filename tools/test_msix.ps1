#
# Packs the installed build into an MSIX package and installs it for a test run, see tools/make_msix.ps1.
# The identity defaults to test values, pass the partner center values to test the package that gets submitted.
#
# The install needs no certificate, windows 11 takes an unsigned package with -AllowUnsigned. It is a sideload
# of your own build, a signature from the store is what smart app control asks for, so this test cannot cover it.
#
# Examples:
#   tools/test_msix.ps1
#   tools/test_msix.ps1 -SkipPack
#   tools/test_msix.ps1 -Uninstall
#
[CmdletBinding()]
param(
  # Identity name of the app, any name works for a test install.
  [string] $IdentityName = "Test.VerseLens",
  # Publisher of the app, any subject works for a test install.
  [string] $Publisher = "CN=VerseLensTest",
  # Publisher name shown to the user.
  [string] $PublisherDisplayName = "Test",
  # Installed build that shall be packed, the folder 'cmake --install' wrote.
  [string] $InstallDir = "build/install",
  # Package that is packed and installed.
  [string] $Package = "msix/VerseLens.msix",
  # Install the package that is there already instead of packing it again.
  [switch] $SkipPack,
  # Remove the installed test package and exit.
  [switch] $Uninstall
)

$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot
$packagePath = [System.IO.Path]::IsPathRooted($Package) ? $Package : (Join-Path $repoRoot $Package)

#
# An install of an earlier run has to go, windows keeps one version of an identity only.
#
$installed = Get-AppxPackage -Name $IdentityName -ErrorAction SilentlyContinue
if($null -ne $installed)
{
  Write-Host "removing $($installed.PackageFullName)"
  Remove-AppxPackage $installed.PackageFullName
}

if($Uninstall)
{
  if($null -eq $installed)
  {
    Write-Host "no package $IdentityName installed"
  }
  return
}

#
# Pack and install, the package replaces whatever the build folder holds.
#
if(-not $SkipPack)
{
  & (Join-Path $PSScriptRoot "make_msix.ps1") `
    -IdentityName $IdentityName `
    -Publisher $Publisher `
    -PublisherDisplayName $PublisherDisplayName `
    -InstallDir $InstallDir `
    -Out $Package
}

if(-not (Test-Path $packagePath))
{
  throw "No package at $packagePath. Run without -SkipPack to pack it."
}

try
{
  Add-AppxPackage -Path $packagePath -AllowUnsigned
}
catch
{
  Write-Host "The install failed. Windows 11 takes an unsigned package, older builds need a signed one:"
  Write-Host "  1. New-SelfSignedCertificate -Type Custom -Subject `"$Publisher`" -KeyUsage DigitalSignature ``"
  Write-Host "       -CertStoreLocation Cert:\CurrentUser\My -TextExtension @(`"2.5.29.37={text}1.3.6.1.5.5.7.3.3`")"
  Write-Host "  2. Import the exported certificate into 'Local Machine\Trusted People', this needs admin rights."
  Write-Host "  3. signtool sign /fd SHA256 /sha1 <thumbprint> $packagePath"
  Write-Host "  4. Add-AppxPackage -Path $packagePath"
  throw
}

#
# Report where the app writes, the package redirects the local data folder.
#
$cmakeLists = Get-Content (Join-Path $repoRoot "verselens/CMakeLists.txt") -Raw
$dataFolder = $cmakeLists -match 'set\(APP_DATA_FOLDER_NAME "([^"]+)"\)' ? $Matches[1] : "verselens"

$package = Get-AppxPackage -Name $IdentityName
Write-Host ""
Write-Host "installed $($package.PackageFullName)"
Write-Host "start it from the start menu, the log tells whether the updater stayed off:"
Write-Host "  $env:LOCALAPPDATA\Packages\$($package.PackageFamilyName)\LocalCache\Local\$dataFolder\logs\latest.log"
Write-Host "the startup entry sits in settings under apps, startup"
Write-Host "remove the test install again with: tools/test_msix.ps1 -Uninstall"
