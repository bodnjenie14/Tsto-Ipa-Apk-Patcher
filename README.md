# APK Patcher

A C++ Qt-based GUI application for patching APK files. This application allows you to modify server URLs in APK files, including both text-based modifications and binary patching.

## Prerequisites

- Qt 6.0 or later
- Premake5
- OpenJDK 11 or later
- Python 3
- Visual Studio 2019 or later (for Windows)

## Building

1. Set the QTDIR environment variable to your Qt installation directory:
```cmd
set QTDIR=C:\Qt\6.5.0\msvc2019_64
```

2. Generate the project files using Premake:
```cmd
premake5 vs2019
```

3. Open the generated solution file in Visual Studio and build the project.

## Usage

1. Launch the application
2. Click "Check Dependencies" to ensure all required tools are installed
3. Click "Browse" to select an APK file
4. Enter the new Game Server URL and DLC Server URL
5. Click "Patch APK" to process the file

## Features

- APK decompilation and recompilation using apktool
- URL replacement in text-based files (.smali, .xml, .txt)
- Binary patching of .so files
- Dependency checking and installation
- User-friendly GUI interface

## Notes

- The application requires Java for APK tool operations
- Make sure you have write permissions in the application directory
- Backup your APK files before patching
