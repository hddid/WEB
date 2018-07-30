cmake_minimum_required(VERSION 2.8)

find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
        MESSAGE("CCACHE_FOUND! " )
endif(CCACHE_FOUND)
if(POLICY CMP0058) 
		cmake_policy(SET CMP0058 NEW) 
endif() 
project(web)

SET(CMAKE_BUILD_TYPE Release)

set(ROOTFS_DIR /home/will/pack/staging_dir/target/rootfs)


IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Debug)
ENDIF()
MESSAGE("CMAKE_HOST_SYSTEM_PROCESSOR: " ${CMAKE_HOST_SYSTEM_PROCESSOR})
MESSAGE("Build type: " ${CMAKE_BUILD_TYPE})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib/)

# Check C++11 or C++0x support
add_definitions(-DCOMPILEDWITHC11)
add_definitions(-DEIGEN_DONT_VECTORIZE -DEIGEN_DISABLE_UNALIGNED_ARRAY_ASSERT)


set (WebSrc
    Src/web.cc
    Src/share_memory_api.cc
    Src/direction.cc
)


set  (MINIVERSION  true)
if (MINIVERSION)
    add_definitions(-DMINIVERSION)
else(MINIVERSION)
    find_package(Pangolin REQUIRED)
endif(MINIVERSION)
MESSAGE("MINIVERSION: " ${MINIVERSION})

if( CMAKE_BUILD_TYPE MATCHES "Debug" )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -Wall  -std=c++11 -pthread -g")
else( CMAKE_BUILD_TYPE MATCHES "Release" )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall  -std=c++11 -mfpu=neon")
endif( CMAKE_BUILD_TYPE MATCHES "Debug" )

LIST(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_modules)
include(cotire)


find_package(OpenCV 3.0 QUIET)
# find_package(Eigen3 3.1.0 REQUIRED)
# find_package(Cholmod REQUIRED)


include_directories(

    ${PROJECT_SOURCE_DIR} 
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/Examples/
    ${ROOTFS_DIR}/usr/include
    ${ROOTFS_DIR}/usr/include/opencv2
    )
link_directories(
        ${PROJECT_SOURCE_DIR}/lib
        ${ROOTFS_DIR}/usr/lib/
)





add_library(Web SHARED
        ${WebSrc}
)
# target_link_libraries(Web  )
cotire(Web)
    
add_executable(web2ui
    Examples/web2ui.cc)
target_link_libraries(web2ui Web  pthread)
cotire(web2ui)

