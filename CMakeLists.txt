PROJECT(ApplicationCore-LiveDemo)
cmake_minimum_required(VERSION 2.8)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)

set(${PROJECT_NAME}_MAJOR_VERSION 01)
set(${PROJECT_NAME}_MINOR_VERSION 00)
set(${PROJECT_NAME}_PATCH_VERSION 00)
include(cmake/set_version_numbers.cmake)

# Moderate version of the compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fPIC -Wextra -ansi -Wuninitialized -O3 -fomit-frame-pointer")

# linker flags
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--export-dynamic -Wl,--no-as-needed")

include(cmake/add_dependency.cmake)
add_dependency(ChimeraTK-ApplicationCore 00.09 REQUIRED)

add_dependency(ChimeraTK-ControlSystemAdapter-OPCUAAdapter REQUIRED)

FIND_PACKAGE(Boost COMPONENTS thread system unit_test_framework REQUIRED)

# extend compiler flags with flags from dependencies
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ChimeraTK-ApplicationCore_CXX_FLAGS}" )
set(CMAKE_LINK_FLAGS "-Wl,--no-as-needed ${ChimeraTK-ControlSystemAdapter-OPCUAAdapter_LINK_FLAGS} ${ChimeraTK-ApplicationCore_LINK_FLAGS}")

# set include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# create lists with source files
aux_source_directory(${CMAKE_SOURCE_DIR}/src appSources)

# main executable
add_executable(${PROJECT_NAME} ${appSources})
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "${CMAKE_LINK_FLAGS}")
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES} ${ChimeraTK-ApplicationCore_LIBRARIES} ${ChimeraTK-ControlSystemAdapter-OPCUAAdapter_LIBRARIES})

# XML file generation
add_executable(${PROJECT_NAME}-xmlGenerator ${appSources})
set_target_properties(${PROJECT_NAME}-xmlGenerator PROPERTIES COMPILE_FLAGS "-DGENERATE_XML")
target_link_libraries(${PROJECT_NAME}-xmlGenerator ${Boost_LIBRARIES} ${ChimeraTK-ApplicationCore_LIBRARIES} ${ChimeraTK-ControlSystemAdapter-OPCUAAdapter_LIBRARIES})

add_custom_target(${PROJECT_NAME}-xml ALL DEPENDS ${PROJECT_NAME}-xmlGenerator COMMAND ${PROJECT_BINARY_DIR}/${PROJECT_NAME}-xmlGenerator WORKING_DIRECTORY ${PROJECT_BINARY_DIR})

# copy map files etc.
configure_file(${CMAKE_SOURCE_DIR}/oven.map ${PROJECT_BINARY_DIR}/oven.map COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/devices.dmap ${PROJECT_BINARY_DIR}/devices.dmap COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/qthm.dmap ${PROJECT_BINARY_DIR}/qthm.dmap COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/exampleApp_mapping.xml ${PROJECT_BINARY_DIR}/exampleApp_mapping.xml COPYONLY)

