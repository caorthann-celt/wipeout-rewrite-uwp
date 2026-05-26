cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

option(UWP_BUILD "Build wipEout as a UWP compatible DLL" OFF)
if(UWP_BUILD)
	set(CMAKE_SYSTEM_NAME WindowsStore)
	set(CMAKE_SYSTEM_VERSION 10.0.19041.0 CACHE STRING "Target Windows SDK version for UWP builds" FORCE)
endif()

project(wipeout-rewrite)

if("${PROJECT_SOURCE_DIR}" STREQUAL "${PROJECT_BINARY_DIR}")
	message(FATAL_ERROR "In-tree builds are not allowed.")
endif()

include(GNUInstallDirs)
include(CMakeDependentOption)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Emscripten")
	set(EMSCRIPTEN true)
endif()

set(platform_options "SDL2" "SOKOL" "NULL")
set(PLATFORM "SDL2" CACHE STRING "Graphics platform to handle input/output")
set_property(CACHE PLATFORM PROPERTY STRINGS "${platform_options}")
if(NOT PLATFORM IN_LIST platform_options)
	message(FATAL_ERROR "PLATFORM must be one of ${platform_options}")
endif()

set(renderer_options "GL" "GLES2" "SOFTWARE" "NULL")
set(RENDERER "GL" CACHE STRING "Graphics rendering backend")
set_property(CACHE RENDERER PROPERTY STRINGS "${renderer_options}")
if(NOT RENDERER IN_LIST renderer_options)
	message(FATAL_ERROR "RENDERER must be one of ${renderer_options}")
endif()
if("${RENDERER}" MATCHES "GL(ES2)?")
	set(using_gl true)
endif()
if("${CMAKE_SYSTEM_NAME}" MATCHES "(Linux|FreeBSD)")
	set(has_glvnd true)
endif()

cmake_dependent_option(USE_GLVND "Link against modern GLVND ABIs" ON "using_gl;has_glvnd" OFF)
cmake_dependent_option(MINIMAL_BUNDLE "Do not include music/movies for web builds" OFF "EMSCRIPTEN" OFF)
option(PATH_ASSETS "Path to where the game assets should be located.")
option(PATH_USERDATA "Path to where user data (e.g. game saves) should be located.")
option(DEV_BUILD "Set asset/userdata paths to the source directory for testing" OFF)
set(WIPEOUT_UWP_VCPKG_ROOT "$ENV{VCPKG_ROOT}" CACHE PATH "Path to vcpkg")
set(WIPEOUT_UWP_DEFAULT_DEPS_ROOT "")
if(WIPEOUT_UWP_VCPKG_ROOT)
	get_filename_component(WIPEOUT_UWP_DEFAULT_DEPS_ROOT "${WIPEOUT_UWP_VCPKG_ROOT}/installed/x64-uwp" ABSOLUTE)
endif()
set(WIPEOUT_UWP_SDL_ROOT "${WIPEOUT_UWP_DEFAULT_DEPS_ROOT}" CACHE PATH "Root of the SDL2 UWP dependency install")
set(WIPEOUT_UWP_ANGLE_ROOT "${WIPEOUT_UWP_DEFAULT_DEPS_ROOT}" CACHE PATH "Root of the ANGLE UWP dependency headers and import libraries")
if (DEV_BUILD)
	set(PATH_ASSETS "${CMAKE_SOURCE_DIR}/")
	set(PATH_USERDATA "${CMAKE_SOURCE_DIR}/")
endif()

if(NOT UWP_BUILD)
	find_package(OpenGL)
	find_package(GLEW)
	find_package(SDL2)
endif()

set(common_src
	src/wipeout/camera.c
	src/wipeout/camera.h
	src/wipeout/droid.c
	src/wipeout/droid.h
	src/wipeout/game.c
	src/wipeout/game.h
	src/wipeout/hud.c
	src/wipeout/hud.h
	src/wipeout/image.c
	src/wipeout/image.h
	src/wipeout/ingame_menus.c
	src/wipeout/ingame_menus.h
	src/wipeout/intro.c
	src/wipeout/intro.h
	src/wipeout/main_menu.c
	src/wipeout/main_menu.h
	src/wipeout/menu.c
	src/wipeout/menu.h
	src/wipeout/object.c
	src/wipeout/object.h
	src/wipeout/particle.c
	src/wipeout/particle.h
	src/wipeout/race.c
	src/wipeout/race.h
	src/wipeout/scene.c
	src/wipeout/scene.h
	src/wipeout/sfx.c
	src/wipeout/sfx.h
	src/wipeout/ship.c
	src/wipeout/ship.h
	src/wipeout/ship_ai.c
	src/wipeout/ship_ai.h
	src/wipeout/ship_player.c
	src/wipeout/ship_player.h
	src/wipeout/title.c
	src/wipeout/title.h
	src/wipeout/track.c
	src/wipeout/track.h
	src/wipeout/ui.c
	src/wipeout/ui.h
	src/wipeout/weapon.c
	src/wipeout/weapon.h
	src/input.c
	src/input.h
	src/mem.c
	src/mem.h
	src/platform.h
	src/render.h
	src/system.c
	src/system.h
	src/types.c
	src/types.h
	src/utils.c
	src/utils.h

)

if(UWP_BUILD)
	add_library(wipeout SHARED ${common_src})
	set_property(TARGET wipeout PROPERTY WINDOWS_EXPORT_ALL_SYMBOLS ON)
else()
	add_executable(wipeout WIN32
		${common_src}
		packaging/windows/wipeout.exe.manifest
		packaging/windows/wipeout.rc
	)
endif()
set_property(TARGET wipeout PROPERTY C_STANDARD 11)
target_include_directories(wipeout PRIVATE src)
target_include_directories(wipeout SYSTEM PRIVATE src/libs)
target_compile_options(wipeout PRIVATE
	$<$<CXX_COMPILER_ID:MSVC>:/W4>
	$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra>
)
target_compile_definitions(wipeout PRIVATE
	$<$<BOOL:${PATH_ASSETS}>:-DPATH_ASSETS=${PATH_ASSETS}>
	$<$<BOOL:${PATH_USERDATA}>:-DPATH_USERDATA=${PATH_USERDATA}>
)

if(UWP_BUILD)
	target_compile_definitions(wipeout PRIVATE
		UWP_BUILD=1
		__WINRT__=1
		"NOMINMAX"
		"_USE_MATH_DEFINES"
		"_CRT_SECURE_NO_WARNINGS"
	)
	target_include_directories(wipeout PRIVATE
		"${WIPEOUT_UWP_SDL_ROOT}/include"
		"${WIPEOUT_UWP_SDL_ROOT}/include/SDL2"
		"${WIPEOUT_UWP_ANGLE_ROOT}/include"
		"${CMAKE_CURRENT_SOURCE_DIR}/ports/uwp/helpers"
	)
	target_link_directories(wipeout PRIVATE
		"${WIPEOUT_UWP_SDL_ROOT}/lib"
		"${WIPEOUT_UWP_ANGLE_ROOT}/lib"
	)
	target_sources(wipeout PRIVATE
		ports/uwp/helpers/uwp_egl_surface_size.c
		ports/uwp/helpers/uwp_egl_surface_size.h
	)
elseif(WIN32)
	target_compile_definitions(wipeout PRIVATE
		"NOMINMAX"
		"_USE_MATH_DEFINES"
		"_CRT_SECURE_NO_WARNINGS"
	)
elseif(APPLE)
	target_compile_definitions(wipeout PRIVATE
		"_THREAD_SAFE"
		"GL_SILENCE_DEPRECATION"
	)
	target_link_libraries(wipeout PUBLIC "-framework Foundation")
	set_source_files_properties(src/platform_sokol.c PROPERTIES COMPILE_FLAGS "-x objective-c")
	if("${PLATFORM}" STREQUAL SOKOL)
		target_link_libraries(wipeout PUBLIC
			"-framework Cocoa"
			"-framework QuartzCore"
			"-framework AudioToolbox"
		)
	endif()
elseif(EMSCRIPTEN)
	# Emscripten's CMake modules don't define targets like the standard
	# ones do, so we define them ourselves here.
	add_library(GLEW::GLEW INTERFACE IMPORTED)
	add_library(OpenGL::GL INTERFACE IMPORTED)
	if (NOT TARGET SDL2::Main)
		add_library(SDL2::Main INTERFACE IMPORTED)
	endif()
	set_target_properties(OpenGL::GL PROPERTIES
		IMPORTED_LIBNAME "GL"
	)
	set_target_properties(GLEW::GLEW PROPERTIES
		IMPORTED_LIBNAME "GLEW"
	)
	set_target_properties(SDL2::Main PROPERTIES
		IMPORTED_LIBNAME "SDL2"
		INTERFACE_COMPILE_OPTIONS "SHELL:-s USE_SDL=2"
		INTERFACE_LINK_LIBRARIES  "SHELL:-s USE_SDL=2"
	)

	target_link_options(wipeout PRIVATE
		"SHELL:-s ALLOW_MEMORY_GROWTH=1"
		"SHELL:-s ENVIRONMENT=web"
		"SHELL:-s FORCE_FILESYSTEM"
		"SHELL:--preload-file ${CMAKE_SOURCE_DIR}/wipeout/@/wipeout"
	)
	if(MINIMAL_BUNDLE)
		target_link_options(wipeout PRIVATE
			"SHELL:--exclude-file ${CMAKE_SOURCE_DIR}/wipeout/music"
			"SHELL:--exclude-file ${CMAKE_SOURCE_DIR}/intro.mpeg"
		)
	endif()

	configure_file("${CMAKE_SOURCE_DIR}/src/wasm-index.html" "game.html" COPYONLY)
elseif(UNIX)
	target_link_libraries(wipeout PUBLIC m)
	if ("${PLATFORM}" STREQUAL "SOKOL" AND "${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
		find_package(Threads REQUIRED)
		find_package(X11 REQUIRED)
		find_package(ALSA REQUIRED)
		target_link_libraries(wipeout PUBLIC
			X11::X11
			X11::Xcursor
			Threads::Threads
			X11::Xi
			dl
			ALSA::ALSA
		)
	endif()
endif()

if(using_gl)
	target_compile_definitions(wipeout PRIVATE "RENDERER_GL")
	target_sources(wipeout PRIVATE src/render_gl.c)

	if ("${RENDERER}" STREQUAL "GLES2")
		target_compile_definitions(wipeout PRIVATE "USE_GLES2")
	endif()

	if(UWP_BUILD)
		target_link_libraries(wipeout PUBLIC
			libEGL.lib
			libGLESv2.lib
			WindowsApp.lib
			OneCore.lib
		)
	elseif ("${RENDERER}" STREQUAL "GLES2")
		if (TARGET OpenGL::GLES2)
			target_link_libraries(wipeout PUBLIC OpenGL::GLES2)
		endif()
	else()
		target_include_directories(wipeout PUBLIC ${OPENGL_INCLUDE_DIR})
		if(USE_GLVND AND TARGET OpenGL::OpenGL)
			target_link_libraries(wipeout PUBLIC OpenGL::OpenGL)
		else()
			target_link_libraries(wipeout PUBLIC OpenGL::GL)
		endif()

		if(NOT APPLE)
			target_include_directories(wipeout PRIVATE ${GLEW_INCLUDE_DIRS})
			target_link_libraries(wipeout PRIVATE GLEW::GLEW)
		endif()
	endif()
elseif("${RENDERER}" STREQUAL "SOFTWARE")
	target_compile_definitions(wipeout PRIVATE "RENDERER_SOFTWARE")
	target_sources(wipeout PRIVATE src/render_software.c)
elseif("${RENDERER}" STREQUAL "NULL")
	target_compile_definitions(wipeout PRIVATE "RENDERER_NULL")
	target_sources(wipeout PRIVATE src/render_null.c)
endif()

if("${PLATFORM}" STREQUAL SDL2)
	target_sources(wipeout PRIVATE src/platform_sdl.c)
	if(UWP_BUILD)
		target_link_libraries(wipeout PUBLIC SDL2.lib)
	else()
		target_link_libraries(wipeout PUBLIC SDL2::Main)
	endif()
elseif("${PLATFORM}" STREQUAL SOKOL)
	target_sources(wipeout PRIVATE src/platform_sokol.c)
elseif("${PLATFORM}" STREQUAL NULL)
	target_sources(wipeout PRIVATE src/platform_null.c)
endif()

install(TARGETS wipeout)
