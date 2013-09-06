# Add external project bullet
ExternalProject_Add(
	bullet
	URL ${CMAKE_SOURCE_DIR}/external/bullet-2.81-rev2613.tar.gz
	CMAKE_GENERATOR ${OSGWORKS_GENERATOR}
	CMAKE_ARGS 
		-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
		-DBUILD_AMD_OPENCL_DEMOS=OFF
		-DBUILD_CPU_DEMOS=OFF
		-DBUILD_DEMOS=OFF
		INSTALL_COMMAND ""
	)
set_target_properties(bullet PROPERTIES FOLDER "3rdparty")

include_directories(${CMAKE_BINARY_DIR}/src/bullet-prefix/src/bullet/src)

set(OSGBULLET_BASE_DIR ${CMAKE_BINARY_DIR}/src/bullet-2.81-rev2613-prefix/src)
