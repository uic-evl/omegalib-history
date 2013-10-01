# List native modules here
add_subdirectory(native/rift)
add_subdirectory(native/porthole)
add_subdirectory(native/sage)
add_subdirectory(native/mvi)

# If a module depends on OpenSceneGraph (or cyclops), add it to this section
if(OMEGA_BUILD_OSG_LIB)
	add_subdirectory(native/templateModule)
	add_subdirectory(native/pointCloud)
endif()
