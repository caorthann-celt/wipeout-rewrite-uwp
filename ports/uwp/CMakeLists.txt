cmake_minimum_required(VERSION 3.14)

set(CMAKE_SYSTEM_NAME WindowsStore)
set(CMAKE_SYSTEM_VERSION 10.0.19041.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(wipeout-uwp LANGUAGES CXX)
set_directory_properties(PROPERTIES VS_STARTUP_PROJECT ${PROJECT_NAME})

set(WIPEOUT_UWP_SDL_ROOT "C:/Users/Caorthann/vcpkg/installed/x64-uwp" CACHE PATH "Root of the SDL2 UWP dependency install")
set(WIPEOUT_UWP_ANGLE_ROOT "C:/Users/Caorthann/vcpkg/installed/x64-uwp" CACHE PATH "Root of the ANGLE UWP dependency install")
set(WIPEOUT_UWP_D3DCOMPILER_DLL "C:/Program Files (x86)/Windows Kits/10/Redist/D3D/x64/d3dcompiler_47.dll" CACHE FILEPATH "UWP-packageable D3D compiler runtime required by ANGLE")
set(WIPEOUT_DATA_ROOT "" CACHE PATH "Directory containing the real wipEout game data folders such as textures, music and common")
set(WIPEOUT_DLL_DIR "" CACHE PATH "Directory containing the UWP-built wipeout DLL and import library")
set(WIPEOUT_DLL "" CACHE FILEPATH "Optional explicit path to the UWP-built wipeout DLL")
set(WIPEOUT_IMPORT_LIB "" CACHE FILEPATH "Optional explicit path to the UWP-built wipeout import library")
set(WIPEOUT_UWP_RUNTIME_DLLS "" CACHE STRING "Additional runtime DLLs required by wipEout, separated by semicolons")

set(WIPEOUT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../..")

set(WIPEOUT_RESOLVED_DATA_ROOT "${WIPEOUT_DATA_ROOT}")
if(NOT WIPEOUT_RESOLVED_DATA_ROOT)
    set(WIPEOUT_RESOLVED_DATA_ROOT "${WIPEOUT_ROOT}/wipeout")
elseif(NOT IS_ABSOLUTE "${WIPEOUT_RESOLVED_DATA_ROOT}")
    get_filename_component(WIPEOUT_RESOLVED_DATA_ROOT "${WIPEOUT_ROOT}/${WIPEOUT_RESOLVED_DATA_ROOT}" ABSOLUTE)
endif()

if(EXISTS "${WIPEOUT_RESOLVED_DATA_ROOT}/textures/drfonts.cmp")
    set(WIPEOUT_PACKAGE_GAME_DATA TRUE)
else()
    set(WIPEOUT_PACKAGE_GAME_DATA FALSE)
    message(STATUS "No packaged wipEout game data found. The app will load user-provided data from LocalState at runtime.")
endif()

set(WIPEOUT_RESOLVED_DLL_DIR "${WIPEOUT_DLL_DIR}")
if(WIPEOUT_RESOLVED_DLL_DIR AND NOT IS_ABSOLUTE "${WIPEOUT_RESOLVED_DLL_DIR}")
    get_filename_component(WIPEOUT_RESOLVED_DLL_DIR "${WIPEOUT_ROOT}/${WIPEOUT_RESOLVED_DLL_DIR}" ABSOLUTE)
endif()

set(WIPEOUT_RESOLVED_DLL "${WIPEOUT_DLL}")
if(NOT WIPEOUT_RESOLVED_DLL AND WIPEOUT_RESOLVED_DLL_DIR)
    set(WIPEOUT_RESOLVED_DLL "${WIPEOUT_RESOLVED_DLL_DIR}/wipeout.dll")
elseif(WIPEOUT_RESOLVED_DLL AND NOT IS_ABSOLUTE "${WIPEOUT_RESOLVED_DLL}")
    get_filename_component(WIPEOUT_RESOLVED_DLL "${WIPEOUT_ROOT}/${WIPEOUT_RESOLVED_DLL}" ABSOLUTE)
endif()

set(WIPEOUT_RESOLVED_IMPORT_LIB "${WIPEOUT_IMPORT_LIB}")
if(NOT WIPEOUT_RESOLVED_IMPORT_LIB AND WIPEOUT_RESOLVED_DLL_DIR)
    set(WIPEOUT_RESOLVED_IMPORT_LIB "${WIPEOUT_RESOLVED_DLL_DIR}/wipeout.lib")
elseif(WIPEOUT_RESOLVED_IMPORT_LIB AND NOT IS_ABSOLUTE "${WIPEOUT_RESOLVED_IMPORT_LIB}")
    get_filename_component(WIPEOUT_RESOLVED_IMPORT_LIB "${WIPEOUT_ROOT}/${WIPEOUT_RESOLVED_IMPORT_LIB}" ABSOLUTE)
endif()

if(NOT EXISTS "${WIPEOUT_RESOLVED_DLL}")
    message(FATAL_ERROR "Set WIPEOUT_DLL_DIR or WIPEOUT_DLL to the UWP-built wipeout.dll.")
endif()

if(NOT EXISTS "${WIPEOUT_RESOLVED_IMPORT_LIB}")
    message(FATAL_ERROR "Set WIPEOUT_IMPORT_LIB to the UWP-built wipeout import library.")
endif()

set(WIPEOUT_UWP_SDL_BIN "${WIPEOUT_UWP_SDL_ROOT}/bin")
set(WIPEOUT_UWP_SDL_LIB "${WIPEOUT_UWP_SDL_ROOT}/lib")
set(WIPEOUT_UWP_ANGLE_BIN "${WIPEOUT_UWP_ANGLE_ROOT}/bin")
set(WIPEOUT_UWP_STAGED_D3DCOMPILER "${CMAKE_CURRENT_BINARY_DIR}/D3DCOMPILER_47.dll")

add_executable(${PROJECT_NAME} WIN32
    main.cpp
)

if(NOT EXISTS "${WIPEOUT_UWP_D3DCOMPILER_DLL}")
    message(FATAL_ERROR "Missing UWP package dependency: ${WIPEOUT_UWP_D3DCOMPILER_DLL}")
endif()

add_custom_command(
    OUTPUT "${WIPEOUT_UWP_STAGED_D3DCOMPILER}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${WIPEOUT_UWP_D3DCOMPILER_DLL}"
        "${WIPEOUT_UWP_STAGED_D3DCOMPILER}"
    DEPENDS "${WIPEOUT_UWP_D3DCOMPILER_DLL}"
    COMMENT "Stage D3DCOMPILER_47.dll with the exact import name used by ANGLE"
)

set(BinDeps
    "${WIPEOUT_UWP_SDL_BIN}/SDL2.dll"
    "${WIPEOUT_UWP_SDL_BIN}/z.dll"
    "${WIPEOUT_UWP_ANGLE_BIN}/libEGL.dll"
    "${WIPEOUT_UWP_ANGLE_BIN}/libGLESv2.dll"
    "${WIPEOUT_UWP_STAGED_D3DCOMPILER}"
    "${WIPEOUT_RESOLVED_DLL}"
    ${WIPEOUT_UWP_RUNTIME_DLLS}
    "${CMAKE_CURRENT_SOURCE_DIR}/Package.appxmanifest"
)

foreach(BinDep ${BinDeps})
    if(NOT BinDep STREQUAL "${WIPEOUT_UWP_STAGED_D3DCOMPILER}" AND NOT EXISTS "${BinDep}")
        message(FATAL_ERROR "Missing UWP package dependency: ${BinDep}")
    endif()
endforeach()

set_source_files_properties("${WIPEOUT_UWP_STAGED_D3DCOMPILER}" PROPERTIES GENERATED TRUE)
source_group("BinDeps" FILES ${BinDeps})
set_source_files_properties(${BinDeps} PROPERTIES
    VS_COPY_TO_OUT_DIR Always
    VS_DEPLOYMENT_CONTENT TRUE
    VS_DEPLOYMENT_LOCATION "."
)

set(PkgAssets
    "${CMAKE_CURRENT_SOURCE_DIR}/Assets/LargeTile.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/Assets/SmallTile.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/Assets/SplashScreen.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/Assets/Square44x44Logo.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/Assets/Square150x150Logo.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/Assets/StoreLogo.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/Assets/WideTile.png"
)

source_group("PkgAssets" FILES ${PkgAssets})
set_source_files_properties(${PkgAssets} PROPERTIES
    VS_DEPLOYMENT_CONTENT TRUE
    VS_DEPLOYMENT_LOCATION "Assets"
)

if(WIPEOUT_PACKAGE_GAME_DATA)
    file(GLOB_RECURSE DATA_FILES
        RELATIVE "${WIPEOUT_RESOLVED_DATA_ROOT}"
        "${WIPEOUT_RESOLVED_DATA_ROOT}/*"
    )

    foreach(ASSET ${DATA_FILES})
        set(FULL_PATH "${WIPEOUT_RESOLVED_DATA_ROOT}/${ASSET}")
        get_filename_component(DEST_DIR "${ASSET}" DIRECTORY)

        set_source_files_properties("${FULL_PATH}" PROPERTIES
            VS_COPY_TO_OUT_DIR Always
            VS_DEPLOYMENT_CONTENT TRUE
            VS_DEPLOYMENT_LOCATION "wipeout/${DEST_DIR}"
        )

        target_sources(${PROJECT_NAME} PRIVATE "${FULL_PATH}")
        source_group("PackageData" FILES "${FULL_PATH}")
    endforeach()
endif()

target_sources(${PROJECT_NAME} PRIVATE
    ${BinDeps}
    ${PkgAssets}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    "${WIPEOUT_UWP_SDL_ROOT}/include"
    "${WIPEOUT_UWP_SDL_ROOT}/include/SDL2"
)

target_link_directories(${PROJECT_NAME} PRIVATE
    "${WIPEOUT_UWP_SDL_LIB}"
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    SDL2.lib
    "${WIPEOUT_RESOLVED_IMPORT_LIB}"
    WindowsApp.lib
    OneCore.lib
)
