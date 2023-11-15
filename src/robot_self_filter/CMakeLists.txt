cmake_minimum_required(VERSION 2.8.3)
project(robot_self_filter)

find_package(catkin REQUIRED COMPONENTS cmake_modules roscpp tf tf_conversions filters sensor_msgs visualization_msgs geometric_shapes urdf resource_retriever pcl_conversions  shape_msgs nodelet )
find_package(Boost REQUIRED)
find_package(PCL REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(OpenMP REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(ASSIMP QUIET)
if (NOT ASSIMP_FOUND)
  pkg_check_modules(ASSIMP assimp)
endif()
if (ASSIMP_FOUND)
  if( ${ASSIMP_VERSION} STRGREATER "2.0.0" )
    set(IS_ASSIMP3 1)
    add_definitions(-DIS_ASSIMP3)
    message(STATUS "Found assimp v3")
  else()
    message(STATUS "Found assimp v2")
  endif()
  include_directories(${ASSIMP_INCLUDE_DIRS})
  link_directories(${ASSIMP_LIBRARY_DIRS})
else()
  message(STATUS "could not find assimp (perhaps available thorugh ROS package?), so assuming assimp v2")
  set(ASSIMP_LIBRARIES assimp)
  set(ASSIMP_LIBRARY_DIRS)
  set(ASSIMP_CXX_FLAGS)
  set(ASSIMP_CFLAGS_OTHER)
  set(ASSIMP_LINK_FLAGS)
  set(ASSIMP_INCLUDE_DIRS)
  set(IS_ASSIMP3 0) # most likely not
endif()


## Declare things to be passed to other projects
catkin_package(
    INCLUDE_DIRS   include
    LIBRARIES      ${PROJECT_NAME}
    CATKIN_DEPENDS roscpp tf tf_conversions filters sensor_msgs visualization_msgs geometric_shapes urdf resource_retriever pcl_conversions  shape_msgs nodelet
    #DEPENDS        pcl assimp
)

include_directories(include SYSTEM ${EIGEN_INCLUDE_DIRS} ${PCL_INCLUDE_DIRS} ${catkin_INCLUDE_DIRS})

set(HEADERS
    include/${PROJECT_NAME}/self_mask.h
    include/${PROJECT_NAME}/self_see_filter.h
)

set(SOURCES
    src/self_filter.cpp
    src/self_filter_nodelet.cpp
    #src/self_mask.cpp
    
)

#common commands for building c++ executables and libraries
add_library(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_link_libraries(${PROJECT_NAME} ${PCL_LIBRARIES} ${catkin_LIBRARIES} assimp ${Boost_LIBRARIES})
add_dependencies(${PROJECT_NAME} ${catkin_EXPORTED_TARGETS})

add_executable(test_filter src/test_filter.cpp ${HEADERS})
target_link_libraries(test_filter ${PROJECT_NAME})
add_dependencies(test_filter ${catkin_EXPORTED_TARGETS} )

add_executable(self_filter src/self_filter_node.cpp ${HEADERS})
target_link_libraries(self_filter ${PROJECT_NAME})
add_dependencies(self_filter ${catkin_EXPORTED_TARGETS} )

install(DIRECTORY include/${PROJECT_NAME}/
  DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION}
  FILES_MATCHING PATTERN "*.h"
  PATTERN ".svn" EXCLUDE
)

install(TARGETS ${PROJECT_NAME} self_filter
   LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
   RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)

#install the launch files.
install(DIRECTORY launch DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION})

install(FILES nodelets.xml DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION})
