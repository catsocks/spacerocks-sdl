cmake_minimum_required(VERSION 3.9)
project(spacerocks C)

# Prevent the assets being overwritten or being copied onto themselves.
file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)
if(EXISTS "${LOC_PATH}")
  message(
    FATAL_ERROR
      "You cannot build in a source directory (or any \
directory with a CMakeLists.txt file). Please make a build subdirectory. Feel \
free to remove CMakeCache.txt and CMakeFiles.")
endif()

include(CTest)
include(CheckLibraryExists)

# As of version 3.19 CMake doesn't come with modules for finding SDL2 libraries.
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Generate a compilation database for use with tools like Clang-Tidy.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# SR is short for Spacerocks.
option(SR_DEBUG "Define macro DEBUG_SPACEROCKS to 1")
option(SR_DEBUG_BOUNDING_BOXES "Define macro DEBUG_GRAPHICS_BOUNDS to 1")
option(SR_DEBUG_RENDERED_TEXT_SIZE "Define macro DEBUG_GRAPHICS_BOUNDS to 1")
option(SR_DEBUG_TOUCH_BUTTONS "Define macro DEBUG_GRAPHICS_BOUNDS to 1")

if(MSVC)
  add_compile_options(/W4)
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# TODO: The Emscripten toolchain file sets NODE_JS_EXECUTABLE, should it be used
# instead of NODE below when it's available?
find_program(NODE node nodejs)

# Sometimes it is not necessary to link libm, when using MSVC for example.
check_library_exists(m pow "" HAVE_LIBM)

# When using Emscripten, SDL2 will be installed during compilation and OpenAL
# just has to be linked.
if(NOT EMSCRIPTEN)
  find_package(SDL2 REQUIRED)
  find_package(OpenAL REQUIRED)
endif()

# PhysFS isn't in the Emscripten Ports collection so it cannot be installed
# during compilation by emcc. You may want to pass -DCMAKE_PREFIX_PATH="/local"
# to CMake when configuring if you've installed PhysicsFS to the Emscripten
# system/local or MinGW sys-root/mingw/local folder.
find_package(PhysFS REQUIRED)

set(SOURCES
    src/assets.c
    src/audio.c
    src/bullet.c
    src/debris.c
    src/font.c
    src/game.c
    src/graphics.c
    src/input.c
    src/language.c
    src/lines.c
    src/math.c
    src/physics.c
    src/renderer.c
    src/rock.c
    src/saucer.c
    src/savefile.c
    src/scoreboard.c
    src/ship.c
    src/transition.c
    src/ui.c)

# Create a library that includes all source files except for main so that it may
# be used by the tests executables.
add_library(spacerocks_lib STATIC ${SOURCES})

# The header files are kept in a different folder because otherwise the tests
# wouldn't be able to include SDL due to a name conflict between Spacerocks
# math.h and the standard library math.h. It'd be nice to keep the headers in
# src/.
target_include_directories(spacerocks_lib
                           PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(EMSCRIPTEN)
  # The Emscripten EM_ASM macro requires some extensions.
  set_target_properties(spacerocks_lib PROPERTIES C_EXTENSIONS ON)

  # Supress warnings generated when using the EM_ASM macro.
  target_compile_options(
    spacerocks_lib
    PRIVATE -Wno-gnu-statement-expression
            -Wno-gnu-zero-variadic-macro-arguments
            -Wno-dollar-in-identifier-extension)

  set(USE_FLAGS -sUSE_SDL=2)
  target_compile_options(spacerocks_lib PUBLIC ${USE_FLAGS})
  # The IDBFS library is necessary for writing persistent files in the browser.
  target_link_options(spacerocks_lib PUBLIC ${USE_FLAGS} -lidbfs.js -lopenal)
else()
  target_link_libraries(spacerocks_lib PUBLIC SDL2::Main ${OPENAL_LIBRARY})

  if(HAVE_LIBM)
    target_link_libraries(spacerocks_lib PRIVATE m)
  endif()
endif()

target_compile_options(
  spacerocks_lib
  PRIVATE $<$<BOOL:${SR_DEBUG_BOUNDING_BOXES}>:-DDEBUG_BOUNDING_BOXES=1>
          $<$<BOOL:${SR_DEBUG_RENDERED_TEXT_SIZE}>:-DDEBUG_RENDERED_TEXT_SIZE=1>
          $<$<BOOL:${SR_DEBUG_TOUCH_BUTTONS}>:-DDEBUG_TOUCH_BUTTONS=1>)

target_compile_options(
  spacerocks_lib
  PUBLIC $<$<OR:$<CONFIG:Debug>,$<BOOL:${SR_DEBUG}>>:-DDEBUG_SPACEROCKS>)

target_link_libraries(spacerocks_lib PUBLIC ${PHYSFS_LIBRARY})

# TODO: Figure out why this is necessary for physfs.h to be found when compiling
# with MinGW, Emscripten, and MSVC but not with GCC (natively on GNU/Linux).
target_include_directories(spacerocks_lib PUBLIC ${PHYSFS_INCLUDE_DIR})

add_executable(spacerocks src/main.c)

target_link_libraries(spacerocks spacerocks_lib)

if(MSVC OR MINGW)
  # TODO: Test that this works with MSVC.
  target_sources(spacerocks
                 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/windows/app.rc)

  set_target_properties(spacerocks PROPERTIES OUTPUT_NAME "Spacerocks")
elseif(APPLE)
  # TODO: Test that this works.
  set_target_properties(
    spacerocks
    PROPERTIES MACOSX_BUNDLE ON
               MACOSX_BUNDLE_INFO_PLIST
               ${CMAKE_CURRENT_SOURCE_DIR}/src/apple/Info.plist)
elseif(EMSCRIPTEN)
  set(SHELL_FILE ${CMAKE_CURRENT_SOURCE_DIR}/src/emscripten/shell.html)

  set_target_properties(
    spacerocks
    PROPERTIES OUTPUT_NAME "game"
               SUFFIX ".html"
               # Always link the target when the Emscripten shell file changes.
               LINK_DEPENDS ${SHELL_FILE})

  target_link_options(
    spacerocks PRIVATE --shell-file ${SHELL_FILE} --preload-file
    ${CMAKE_CURRENT_SOURCE_DIR}/assets/game@assets)

  # Allow for the output of the program to be shown in the emrun command-line
  # tool.
  target_link_options(spacerocks PRIVATE $<$<CONFIG:Debug>:--emrun>)

  # You probably want to set the URL environment variable used by the
  # build-website.js script and configure this project in Release mode when
  # building this target. TODO: Consider implementing the build-website.js
  # functionality with CMake.
  if(NODE)
    add_custom_target(
      spacerocks_website
      DEPENDS spacerocks
      COMMAND
        ${NODE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/build-website.js
        --assets-folder ${CMAKE_CURRENT_SOURCE_DIR}/assets/web --build-folder
        ${CMAKE_CURRENT_BINARY_DIR} --output-folder
        ${CMAKE_CURRENT_BINARY_DIR}/website
      BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/website)
  endif()
endif()

if(NOT EMSCRIPTEN)
  # Copy assets files to where the executable will be.
  add_custom_target(
    spacerocks_copy_assets
    COMMAND
      ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/assets/game
      ${CMAKE_CURRENT_BINARY_DIR}/assets
    BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/assets/audio
               ${CMAKE_CURRENT_BINARY_DIR}/assets/images)

  # Create a ZIP archive of the assets files in the folder the executable will
  # be.
  # FIXME: The wildcard operator doesn't seem to work on Windows.
  add_custom_target(
    spacerocks_archive_assets
    COMMAND ${CMAKE_COMMAND} -E tar cf ${CMAKE_CURRENT_BINARY_DIR}/assets *
            --format=zip
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/assets/game
    BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/assets)

  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_dependencies(spacerocks spacerocks_copy_assets)
  else()
    add_dependencies(spacerocks spacerocks_archive_assets)
  endif()

  if(NODE)
    # This target should be built when distributing the game.
    add_custom_target(
      spacerocks_manual
      COMMAND
        ${NODE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/build-manual.js --input
        ${CMAKE_CURRENT_SOURCE_DIR}/assets/web/index.html --output-folder
        ${CMAKE_CURRENT_BINARY_DIR}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/assets/web
      BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/Spacerocks.html)
  endif()
endif()

# TODO: Figure out how to copy the DLLs of 3rd party libraries to the folder the
# executable is in when compiling for Windows. Until then if the executables
# complain about missing DLLs please copy them from the libraries' folders. The
# DLLs of x86_64 MinGW libraries installed on Fedora using the dnf package
# manager are stored in /usr/x86_64-w64-mingw32/sys-root/mingw/bin.

set(TESTS tests/lines.c tests/math.c tests/scoreboard.c)

if(BUILD_TESTING)
  # As of Emscripten v2.0.17 the path to the node executable in
  # CMAKE_CROSSCOMPILING_EMULATOR is surrounded in unnecessary double quotes
  # that cause ctest to fail running tests.
  if(EMSCRIPTEN)
    string(REPLACE "\"" "" CMAKE_CROSSCOMPILING_EMULATOR
                   ${CMAKE_CROSSCOMPILING_EMULATOR})
  endif()

  foreach(TEST ${TESTS})
    # TEST_NAME will be equal to the filename without extension.
    get_filename_component(TEST_NAME ${TEST} NAME_WE)
    string(PREPEND TEST_NAME "test_")

    add_executable(${TEST_NAME} ${TEST})
    # Statically linking the library for every test seems wasteful but I think
    # it's okay for just a few tests.
    target_link_libraries(${TEST_NAME} spacerocks_lib)

    # You can run all the tests at once using the CMake ctest tool.
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
  endforeach()
endif()
