# This script is meant to be used
# Set the path to your Mingw installation folder
$mingw  = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath("$env:APPDATA\..\Local\Programs\WinLibs\mingw64")

# Function to download, extract, and copy SDL development files
function Install-SDL {
    param (
        [Parameter(Mandatory)]
        [string]$Url,
        [Parameter(Mandatory)]
        [string]$ArchiveName
    )

    Write-Output "Downloading $Url..."
    Invoke-WebRequest -Uri $Url -OutFile $ArchiveName

    # Create a temporary directory for extraction
    $tempDir = Join-Path $env:TEMP ([System.IO.Path]::GetRandomFileName())
    New-Item -ItemType Directory -Path $tempDir | Out-Null

    Write-Output "Extracting $ArchiveName to $tempDir..."
    tar -xzf $ArchiveName -C $tempDir

    # Get the first subfolder from the extraction (it should be something like SDL2-devel-2.0.22-mingw)
    $extractedFolder = (Get-ChildItem $tempDir | Where-Object { $_.PSIsContainer } | Select-Object -First 1).FullName

    # Check if the folder "x86_64-w64-mingw32" exists inside the extracted folder
    $baseFolder = Join-Path $extractedFolder 'x86_64-w64-mingw32'
    if (Test-Path $baseFolder) {
        $sourceFolder = $baseFolder
    }
    else {
        $sourceFolder = $extractedFolder
    }

    # Copy include files
    $includePath = Join-Path $sourceFolder 'include'
    if (Test-Path $includePath) {
        Write-Output "Copying include files from $includePath to $mingw\include ..."
        Copy-Item (Join-Path $includePath '*') -Destination (Join-Path $mingw 'include') -Recurse -Force
    }

    # Copy library files
    $libPath = Join-Path $sourceFolder 'lib'
    if (Test-Path $libPath) {
        Write-Output "Copying library files from $libPath to $mingw\lib ..."
        Copy-Item (Join-Path $libPath '*') -Destination (Join-Path $mingw 'lib') -Recurse -Force
    }

    # Optionally, also copy bin files if required
    $binPath = Join-Path $sourceFolder 'bin'
    if (Test-Path $binPath) {
        Write-Output "Copying binary files from $binPath to $mingw\bin ..."
        Copy-Item (Join-Path $binPath '*') -Destination (Join-Path $mingw 'bin') -Recurse -Force
    }

    # Clean up the downloaded archive and temporary directory
    Write-Output "Cleaning up $ArchiveName and temporary files..."
    Remove-Item $ArchiveName -Force
    Remove-Item $tempDir -Recurse -Force
}

function Install-MSYS {
    param (
        [Parameter(Mandatory)]
        [string]$Url,
        [Parameter(Mandatory)]
        [string]$ArchiveName
    )

    Write-Output "Downloading $Url..."
    Invoke-WebRequest -Uri $Url -OutFile $ArchiveName

    # Create a temporary directory for extraction
    $tempDir = Join-Path $env:TEMP ([System.IO.Path]::GetRandomFileName())
    New-Item -ItemType Directory -Path $tempDir | Out-Null

    Write-Output "Extracting $ArchiveName to $tempDir..."
    tar -xzf $ArchiveName -C $tempDir

    # Get the first subfolder from the extraction (it should be something like SDL2-devel-2.0.22-mingw)
    $extractedFolder = (Get-ChildItem $tempDir | Where-Object { $_.PSIsContainer } | Select-Object -First 1).FullName

    $baseFolder = Join-Path $extractedFolder 'mingw64'
    if (Test-Path $baseFolder) {
        $sourceFolder = $baseFolder
    }
    else {
        $sourceFolder = $extractedFolder
    }

    # Copy include files
    $includePath = Join-Path $sourceFolder 'include'
    if (Test-Path $includePath) {
        Write-Output "Copying include files from $includePath to $mingw\include ..."
        Copy-Item (Join-Path $includePath '*') -Destination (Join-Path $mingw 'include') -Recurse -Force
    }

    # Copy library files
    $libPath = Join-Path $sourceFolder 'lib'
    if (Test-Path $libPath) {
        Write-Output "Copying library files from $libPath to $mingw\lib ..."
        Copy-Item (Join-Path $libPath '*') -Destination (Join-Path $mingw 'lib') -Recurse -Force
    }

    # Optionally, also copy bin files if required
    $binPath = Join-Path $sourceFolder 'bin'
    if (Test-Path $binPath) {
        Write-Output "Copying binary files from $binPath to $mingw\bin ..."
        Copy-Item (Join-Path $binPath '*') -Destination (Join-Path $mingw 'bin') -Recurse -Force
    }

    # Clean up the downloaded archive and temporary directory
    Write-Output "Cleaning up $ArchiveName and temporary files..."
    Remove-Item $ArchiveName -Force
    Remove-Item $tempDir -Recurse -Force
}

# Download and install SDL2 and related libraries

Install-SDL 'https://www.libsdl.org/release/SDL2-devel-2.0.22-mingw.tar.gz' 'SDL2-devel.tar.gz'
Install-SDL 'https://www.libsdl.org/projects/SDL_image/release/SDL2_image-devel-2.6.3-mingw.tar.gz' 'SDL2_image-devel.tar.gz'
Install-SDL 'https://www.libsdl.org/projects/SDL_mixer/release/SDL2_mixer-devel-2.6.2-mingw.tar.gz' 'SDL2_mixer-devel.tar.gz'
Install-SDL 'https://www.libsdl.org/projects/SDL_ttf/release/SDL2_ttf-devel-2.20.2-mingw.tar.gz' 'SDL2_ttf-devel.tar.gz'
Install-MSYS 'https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-SDL2_gfx-1.0.4-2-any.pkg.tar.zst' 'SDL2-gfx.tar.gz'
Install-MSYS 'https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-zlib-1.3.1-1-any.pkg.tar.zst' 'zlib.tar.gz'
Install-MSYS 'https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-boost-1.87.0-3-any.pkg.tar.zst' 'boost.tar.gz'
Install-MSYS 'https://repo.msys2.org/mingw/mingw64/mingw-w64-x86_64-freeglut-3.6.0-1-any.pkg.tar.zst' 'freeglut.tar.gz'
Install-MSYS 'https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-glew-2.2.0-3-any.pkg.tar.zst' 'glew.tar.gz'
Install-MSYS 'https://repo.msys2.org/mingw/mingw64/mingw-w64-x86_64-boost-1.85.0-4-any.pkg.tar.zst' 'boost-1.85.tar.gz'
Install-MSYS 'https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-gcc-libs-14.2.0-3-any.pkg.tar.zst' 'gcc-libs.tar.gz'
Install-MSYS 'https://repo.msys2.org/mingw/mingw64/mingw-w64-x86_64-libwinpthread-git-12.0.0.r640.g75ed7d2d1-1-any.pkg.tar.zst' 'winpthread.tar.gz'
Install-MSYS 'https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-ffmpeg-7.1.1-2-any.pkg.tar.zst' 'ffmpeg.tar.gz'

Write-Output "Installation complete."``