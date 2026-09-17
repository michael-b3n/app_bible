#
# Packs an installed build into an MSIX package for the microsoft store. The store signs the package during
# certification, so the package this script writes is unsigned and only installs on a machine with a test signature.
#
# Identity name, publisher and publisher display name are the values partner center shows for the reserved app name.
# Keep them out of the sources, the release workflow passes them from repository variables.
#
# Examples:
#   tools/make_msix.ps1 -IdentityName 12345MyName.VerseLens -Publisher "CN=..." -PublisherDisplayName "My Name"
#   tools/make_msix.ps1 -InstallDir build/install -Version 2.2 -Out msix/VerseLens.msix -IdentityName ... -Publisher ...
#
[CmdletBinding()]
param(
  # Identity name of the app, as reserved in partner center.
  [Parameter(Mandatory = $true)] [string] $IdentityName,
  # Publisher of the app, the full subject of the store certificate, e.g. "CN=ABCD1234-...".
  [Parameter(Mandatory = $true)] [string] $Publisher,
  # Publisher name shown to the user in the store listing.
  [Parameter(Mandatory = $true)] [string] $PublisherDisplayName,
  # Installed build that shall be packed, the folder 'cmake --install' wrote.
  [string] $InstallDir = "build/install",
  # Version of the package, <major>.<minor> or a full version. The store requires the revision to be zero.
  [string] $Version = "",
  # Path of the package this script writes.
  [string] $Out = "msix/VerseLens.msix"
)

$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot
$manifestTemplate = Join-Path $repoRoot "verselens/res/msix/AppxManifest.xml.in"
$assetsDir = Join-Path $repoRoot "verselens/res/msix/assets"

#
# Read the app information CMake holds, so the package never disagrees with the build.
#
$cmakeLists = Get-Content (Join-Path $repoRoot "verselens/CMakeLists.txt") -Raw
function Read-CMakeValue([string] $name)
{
  if($cmakeLists -notmatch "set\($name `"?([^`")]+)`"?\)")
  {
    throw "$name not found in verselens/CMakeLists.txt"
  }
  return $Matches[1]
}

$exeName = Read-CMakeValue "APP_EXE_NAME"
$appName = Read-CMakeValue "APP_NAME"
$description = (Read-CMakeValue "APP_FILE_DESCRIPTION").Replace('${APP_NAME}', $appName)

# The application id of the manifest allows no spaces, makeappx reports that as a schema error only.
if($appName -notmatch "^[A-Za-z][A-Za-z0-9.-]*$")
{
  throw "APP_NAME '$appName' is no valid application id, it has to start with a letter and hold letters, digits, '.' or '-'"
}

if($Version -eq "")
{
  $Version = "$(Read-CMakeValue 'APP_VERSION_MAJOR').$(Read-CMakeValue 'APP_VERSION_MINOR')"
}

# The manifest wants four numbers, the store rejects a revision other than zero.
$parts = @($Version.Split('.') | Where-Object { $_ -ne "" })
while($parts.Count -lt 3) { $parts += "0" }
$packageVersion = "$($parts[0]).$($parts[1]).$($parts[2]).0"

#
# Locate makeappx of the windows SDK, the newest version wins.
#
$makeAppx = Get-Command makeappx.exe -ErrorAction SilentlyContinue
if($null -eq $makeAppx)
{
  $makeAppx = Get-ChildItem "${env:ProgramFiles(x86)}/Windows Kits/10/bin/*/x64/makeappx.exe" -ErrorAction SilentlyContinue |
    Sort-Object FullName | Select-Object -Last 1
  if($null -eq $makeAppx)
  {
    throw "makeappx.exe not found. Install the Windows SDK or add it to the PATH."
  }
}
$makeAppxPath = if($makeAppx -is [System.Management.Automation.CommandInfo]) { $makeAppx.Source } else { $makeAppx.FullName }

#
# Stage the installed build together with manifest and assets, makeappx packs a folder as it is.
#
function Resolve-RepoPath([string] $path)
{
  return [System.IO.Path]::IsPathRooted($path) ? $path : (Join-Path $repoRoot $path)
}

$installPath = Resolve-RepoPath $InstallDir
if(-not (Test-Path (Join-Path $installPath "bin/$exeName.exe")))
{
  throw "No installed build at $installPath. Run the install of the release preset first."
}

$stagingDir = Join-Path ([System.IO.Path]::GetTempPath()) "verselens_msix_$([System.IO.Path]::GetRandomFileName())"
try
{
  New-Item -ItemType Directory -Force $stagingDir | Out-Null
  Copy-Item "$installPath/*" $stagingDir -Recurse
  Copy-Item $assetsDir (Join-Path $stagingDir "assets") -Recurse

  $replacements = @{
    "@IDENTITY_NAME@"           = $IdentityName
    "@PUBLISHER@"               = $Publisher
    "@PUBLISHER_DISPLAY_NAME@"  = $PublisherDisplayName
    "@VERSION@"                 = $packageVersion
    "@DISPLAY_NAME@"            = $description
    "@DESCRIPTION@"             = $description
    "@APPLICATION_ID@"          = $appName
    "@EXE_NAME@"                = $exeName
  }
  $manifest = Get-Content $manifestTemplate -Raw
  foreach($key in $replacements.Keys)
  {
    $manifest = $manifest.Replace($key, $replacements[$key])
  }
  Set-Content (Join-Path $stagingDir "AppxManifest.xml") $manifest -Encoding UTF8

  $outPath = Resolve-RepoPath $Out
  New-Item -ItemType Directory -Force (Split-Path -Parent $outPath) | Out-Null
  & $makeAppxPath pack /o /d $stagingDir /p $outPath
  if($LASTEXITCODE -ne 0)
  {
    throw "makeappx failed with exit code $LASTEXITCODE"
  }
  Write-Host "packed $outPath, version $packageVersion"
}
finally
{
  Remove-Item $stagingDir -Recurse -Force -ErrorAction SilentlyContinue
}
