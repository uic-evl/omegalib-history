# In this section, add modules that require OpenSceneGraph support
if(OMEGA_BUILD_OSG_LIB)
	#templateModule
	add_subdirectory(templateModule)
	add_subdirectory(pointCloud)
	add_subdirectory(rift)
	add_subdirectory(porthole)
	add_subdirectory(flipbookPlayer)
	add_subdirectory(sage)
	add_subdirectory(mvi)
endif()
