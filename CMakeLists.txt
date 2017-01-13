cmake_minimum_required (VERSION 2.8)

project (sserver)

if(UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  set(Boost_USE_STATIC_LIBS OFF )
  set(Boost_USE_MULTITHREADED ON)
  set(Boost_USE_STATIC_RUNTIME OFF)

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
elseif(MSVC)
  # configurations
  set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
   
  # Cxx-flags
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} /D_DEBUG /MTd  /Zi /Ob0 /Od /RTC1")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} /GL /Oi /Ot /MT /Ox /Ob2 /D NDEBUG")
  
  # linker flags
  set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS} /debug")
  set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS} /LTCG /INCREMENTAL:NO")

  find_package( Boost COMPONENTS system date_time filesystem program_options REQUIRED )

  set(CMAKE_INCLUDE_CURRENT_DIR ON)

  include_directories( ${Boost_INCLUDE_DIRS} )
  link_directories(${Boost_LIBRARY_DIR})
  
  set(Boost_USE_STATIC_LIBS ON )

  add_executable(sserver
    client.cpp
    server.cpp
    logger.cpp
    sserver.h
    main.cpp)

  target_link_libraries(sserver)
endif()
