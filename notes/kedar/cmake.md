# CMake Notes for CSCE 120 Honors Project

## Table of Contents
- [What is CMake](#what-is-cmake)
- [CMakeLists.txt Structure](#cmakelists-txt-structure)
- [Basic Commands](#basic-commands)
    - [Adding an Executable](#adding-an-executable)
    - [Adding a Library](#adding-a-library)
    - [Linking Libraries](#linking-libraries)
- [Including External Libraries](#including-external-libraries)
- [Variables](#variables)
    - [PROJECT_NAME](#project_name)
    - [CMAKE_CURRENT_SOURCE_DIR](#cmake_current_source_dir)
    - [Defining Custom Variables](#defining-custom-variables)
    - [Setting C++ Standards](#setting-c-standards)
    -   [CMAKE_CXX_STANDARD](#cmake_cxx_standard)
    -   [CMAKE_CXX_STANDARD_REQUIRED](#cmake_cxx_standard_required)
    -   [CMAKE_CXX_EXTENSIONS](#cmake_cxx_extensions)

## What is CMake
Programs are built with Makefiles. Makefiles have targets and commands instructing how to build them. CMake makes Makefiles.

CMake is built using a `CMakeLists.txt` file, which contains instructions for CMake to generate a Makefile. CMake shouldn't be built in the same directory as the source code.
Create a separate `build` directory, navigate to it, and run `cmake ..` to generate the Makefile in the `build` directory.
Running the `cmake` command will read the `CMakeLists.txt` file from the specified source directory, and generate the Makefile in the current directory.
CMake doesn't clean up each build; that needs to be done manually by deleting the `build` directory or using `make clean` if specified in the Makefile.

## CMakeLists.txt Structure
A basic `CMakeLists.txt` file has the following structure:
```cmake
cmake_minimum_required(VERSION <version>)
project(<project_name>)
<build instructions>
```
- `cmake_minimum_required(VERSION <version>)`: Specifies the minimum version of CMake required.
    - e.g. `cmake_minimum_required(VERSION 3.10)`
- `project(<project_name>)`: Defines the name of the project.
    - e.g. `project(app)`
- `<build instructions>`: Contains commands to specify source files, include directories, libraries, and build targets.

## Basic Commands
### Adding an Executable
The `add_executable` command is used to define an executable target. The syntax is:
```cmake
add_executable(<executable_name> <source_files>)
add_executable(app main.cpp utils.cpp)
```
- `<executable_name>`: The name of the executable to be created.
- `<source_files>`: A list of source files that will be compiled to create the executable.

### Adding a Library
The `add_library` command is used to define a static or dynamic library target.

Static libraries end in `.a` and are linked at compile time. The syntax is: 
```cmake
add_library(<library_name> STATIC <source_files>)
add_library(mylib STATIC utils.cpp math.cpp)
```
- `<library_name>`: The name of the library to be created.
- `<source_files>`: A list of source files that will be compiled to create the library.

Static libraries become part of the final executable. CMake defaults to static libraries, so the `STATIC` keyword is optional.

Dynamic/shared libraries end in `.so` and are linked at runtime. The syntax is:
```cmake
add_library(<library_name> SHARED <source_files>)
add_library(mylib SHARED utils.cpp math.cpp)
```
- `<library_name>`: The name of the library to be created.
- `<source_files>`: A list of source files that will be compiled to create the library.

Shared libraries are not part of the final executable and can be updated independently.

### Linking Libraries
To tell CMake to link a library to an executable, use the `target_link_libraries` command:
```cmake
target_link_libraries(<executable_name> <library_name>)
target_link_libraries(app mylib)
```
- `<executable_name>`: The name of the executable target.
- `<library_name>`: The name of the library to link to the executable.

The library and executable must be defined using `add_executable` and `add_library` before linking them.
If the library is located in a separate directory, the directory needs to be specified using the `add_subdirectory` command:
```cmake
add_subdirectory(<library_directory>)
add_subdirectory(mylib)
```
The directory needs to have a `CMakeLists.txt` file defining the library using `add_library`.
After adding the subdirectory, we need to tell CMake where to find the header files using the `target_include_directories` command:
```cmake
target_include_directories(<library_name> <ACCESS> <include_directory>)
target_include_directories(app PRIVATE ../mylib/include)
```
- `<library_name>`: The name of the library target. This can also be an executable target.
- `<ACCESS>`: The access level for the include directory (`PRIVATE`, `PUBLIC`, or `INTERFACE`).
    - `PRIVATE`: The target knows where to find the include directory, but targets that link to it do not.
    - `PUBLIC`: The target and targets that link to it know where to find the include directory.
    - `INTERFACE`: Only targets that link to it know where to find the include directory.
    Typically, this is used for libraries that are used by other targets and have header files in the same directory.
- `<include_directory>`: The path to the directory containing the header files.

This command should be specified in the library's `CMakeLists.txt` file.

## Including External Libraries
To include external, system libraries, use the `find_package` command:
```cmake
find_package(<package_name> REQUIRED)
find_package(fmt REQUIRED)
```
- `<package_name>`: The name of the package to find.
- `REQUIRED`: Specifies that the package is required for the build to succeed.
After finding the package, link it to the executable using `target_link_libraries`:
```cmake
target_link_libraries(<executable_name> <package_name>::<package_name>)
target_link_libraries(app fmt::fmt)
```
- `<executable_name>`: The name of the executable target.
- `<package_name>::<package_name>`: The target name for the package, typically in the format `<package_name>::<package_name>`.

Most packages specify their target names in their documentation.

## Variables
CMake allows the use of variables to store values that can be reused throughout the `CMakeLists.txt` file. Some variables are built-in, while others are user-defined.
Variables are accessed with `${<VARIABLE_NAME>}`.
### PROJECT_NAME
`PROJECT_NAME` is a built-in variable that holds the name of the project defined in the `project()` command.
Example usage:
```cmake
project(MyApp)
add_executable(${PROJECT_NAME} main.cpp)
```
### CMAKE_CURRENT_SOURCE_DIR
`CMAKE_CURRENT_SOURCE_DIR` is a built-in variable that holds the path to the directory containing the currently processed `CMakeLists.txt` file.
Example usage:
```cmake
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
```

### Defining Custom Variables
Custom variables can be defined using the `set` command:
```cmake
set(<VARIABLE_NAME> <value>)
set(SOURCES main.cpp utils.cpp math.cpp)
add_executable(${PROJECT_NAME} ${SOURCES})
```
- `<VARIABLE_NAME>`: The name of the variable to define.
- `<value>`: The value(s) to assign to the variable.

### Setting C++ Standards
It's good practice to set the C++ standard at the start of every CMake project.
To specify the C++ standard for the project, use the following variables:
#### CMAKE_CXX_STANDARD
Sets the C++ standard version (e.g., 11, 14, 17, 20).
Example usage:
```cmake
set(CMAKE_CXX_STANDARD 17)
```
#### CMAKE_CXX_STANDARD_REQUIRED
Specifies whether the C++ standard is required.
Example usage:
```cmake
set(CMAKE_CXX_STANDARD_REQUIRED <CONDITION>)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
```
- `<CONDITION>`: `YES` or `NO` (or `ON`/`OFF`). If set to `ON` or `YES`, CMake will generate an error if the specified standard is not supported by the compiler.
Otherwise, it will use the closest supported standard.
#### CMAKE_CXX_EXTENSIONS
Specifies whether to use compiler-specific extensions.
Example usage:
```cmake
set(CMAKE_CXX_EXTENSIONS <CONDITION>)
set(CMAKE_CXX_EXTENSIONS OFF)
```
- `<CONDITION>`: `ON` or `OFF`. If set to `ON`, compiler-specific extensions will be enabled (e.g., GNU extensions for GCC).
If set to `OFF`, only standard-compliant code will be used.

