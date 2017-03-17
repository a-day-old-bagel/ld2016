cmake_minimum_required(VERSION 3.3)
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
project(ld2016)
set(CMAKE_BUILD_TYPE debug)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

option(EMSCRIPTEN_ENABLED "Build using Emscripten" OFF)

if(DEFINED ENV{EMSCRIPTEN} AND EMSCRIPTEN_ENABLED)
  if(CMAKE_BUILD_TYPE MATCHES debug)
    set(EMSCRIPTEN_FLAGS
        "-Oz"
#        "-s ASSERTIONS=2"
        "-s SAFE_HEAP=1"
        "-s STACK_OVERFLOW_CHECK=2"
        "-s DISABLE_EXCEPTION_CATCHING=0"
        "-s ALLOW_MEMORY_GROWTH=1"
       )
    string (REPLACE ";" " " EMSCRIPTEN_FLAGS "${EMSCRIPTEN_FLAGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EMSCRIPTEN_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EMSCRIPTEN_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${EMSCRIPTEN_FLAGS}")
  elseif(CMAKE_BUILD_TYPE MATCHES release)
    set(EMSCRIPTEN_FLAGS
        "-O3"
        "-s ASSERTIONS=0"
        "-s SAFE_HEAP=0"
        "-s STACK_OVERFLOW_CHECK=0"
        "-s DISABLE_EXCEPTION_CATCHING=1"
        "-s ALLOW_MEMORY_GROWTH=0"
       )
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EMSCRIPTEN_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EMSCRIPTEN_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${EMSCRIPTEN_FLAGS}")
  else()
    message(FATAL_ERROR "Unknown build type: ${CMAKE_BUILD_TYPE}")
  endif()
  # Use C++11 everywhere with Emscripten
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  # C++ demangle support with Emscripten
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s DEMANGLE_SUPPORT=1")

  # Temporarily disable the default Emscripten behavior of ignoring system
  # include directories so that we can include the header-only GLM library
  set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)

  set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

  # Asset Import library for loading meshes
  set(ASSIMP_LIBRARIES "${CMAKE_SOURCE_DIR}/extern/assimp-install-js/lib/libassimp.so")
  include_directories( SYSTEM ./extern/assimp-install-js/include/ )

  # Bullet physics libary
  set(BULLET_LIBRARIES "${CMAKE_SOURCE_DIR}/extern/bullet3/install-js/lib/libbullet.so")
  include_directories( SYSTEM ${CMAKE_SOURCE_DIR}/extern/bullet3/install-js/include/ )
  include_directories( SYSTEM ${CMAKE_SOURCE_DIR}/extern/bullet3/src)

  # GLM - Graphics Library Math
  include_directories( SYSTEM ./extern/glm/ )

else()
  find_package( Bullet REQUIRED )
  include_directories( SYSTEM ${BULLET_INCLUDE_DIRS} )

  include_directories( SYSTEM extern/glm )

  find_package( SDL2 REQUIRED )
  include_directories( SYSTEM ${SDL2_INCLUDE_DIR} )

  find_package( GLEW REQUIRED )
  include_directories( SYSTEM ${GLEW_INCLUDE_DIR} )

  find_package( OpenGL REQUIRED )
  include_directories( SYSTEM ${OPENGL_INCLUDE_DIR} )

  find_package( ASSIMP REQUIRED )
  include_directories( SYSTEM ${ASSIMP_INCLUDE_DIR} )

  get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
  foreach(dir ${dirs})
    message(STATUS "dir='${dir}'")
  endforeach()
endif()
include_directories( SYSTEM extern/stb)

add_subdirectory("./src")
