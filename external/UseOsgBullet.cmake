#find bullet
include(FindBulletHelper)
#include bullet
include_directories(
	${BULLET_INCLUDE_DIRS}
    ${BULLET_EXTRAS_INCLUDE_DIR}
)

set(EXTLIB_NAME osgbullet)
set(EXTLIB_TGZ ${CMAKE_SOURCE_DIR}/external/${EXTLIB_NAME}.tar.gz)
set(EXTLIB_DIR ${CMAKE_BINARY_DIR}/${EXTLIB_NAME})

if(NOT EXISTS ${EXTLIB_DIR})
  execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf
    ${EXTLIB_TGZ} WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
endif(NOT EXISTS ${EXTLIB_DIR})

set(OSGBULLET_INCLUDES ${EXTLIB_DIR}/include)

set(OSGBULLET_LIB_DIR ${CMAKE_BINARY_DIR}/osgBullet/lib)
set(OSGBULLET_COMPONENTS osgbCollision osgbDynamics osgbInteraction)
if(OMEGA_OS_WIN)
	foreach( C ${OSGBULLET_COMPONENTS})
		set(${C}_LIBRARY ${OSGBULLET_LIB_DIR}/${C}.lib)
		set(${C}_LIBRARY_DEBUG ${OSGBULLET_LIB_DIR}/${C}d.lib)
		set(OSGBULLET_LIBS ${OSGBULLET_LIBS} optimized ${${C}_LIBRARY} debug ${${C}_LIBRARY_DEBUG})
	endforeach()
elseif(OMEGA_OS_LINUX)
    foreach( C ${OSGBULLET_COMPONENTS} )
		set(${C}_LIBRARY ${OSGBULLET_LIB_DIR}/lib${C}.a)
		set(${C}_LIBRARY_DEBUG ${OSGBULLET_LIB_DIR}/lib${C}.a)
		set(OSGBULLET_LIBS ${OSGBULLET_LIBS} optimized ${${C}_LIBRARY} debug ${${C}_LIBRARY_DEBUG})
	endforeach()
elseif(APPLE)
	foreach( C ${OSGBULLET_COMPONENTS} )
		set(${C}_LIBRARY ${OSGBULLET_LIB_DIR}/lib${C}.a)
		set(${C}_LIBRARY_DEBUG ${OSGBULLET_LIB_DIR}/lib${C}.a)
		set(OSGBULLET_LIBS ${OSGBULLET_LIBS} optimized ${${C}_LIBRARY} debug ${${C}_LIBRARY_DEBUG})
	endforeach()
endif(OMEGA_OS_WIN)