cmake_minimum_required (VERSION 2.8)

project (sserver)

if(UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(MSVC)
  # configurations
  set(CMAKE_CONFIGURATION_TYPES "Debug;Release")

  # Cxx-flags
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} /D_DEBUG /MTd  /Zi /Ob0 /Od /RTC1")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} /GL /Oi /Ot /MT /Ox /Ob2 /D NDEBUG")

  # linker flags
  set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS} /debug")
  set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS} /LTCG /INCREMENTAL:NO")

  add_definitions(-D_SBCS)
endif()

set(Boost_USE_STATIC_LIBS ON )
set(Boost_USE_STATIC_RUNTIME ON)
set(Boost_USE_MULTITHREADED ON)

find_package( Boost COMPONENTS system date_time filesystem program_options REQUIRED )

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories( ${Boost_INCLUDE_DIRS} )
link_directories(${Boost_LIBRARY_DIR})

add_executable(sserver
  client.cpp
  server.cpp
  logger.cpp
  sserver.h
  main.cpp)

target_link_libraries(sserver ${Boost_LIBRARIES})
