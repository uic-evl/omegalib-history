set(MODULES_ENABLE_FLIPBOOK_PLAYER true CACHE BOOL "Enable the flipbook player module")
if(MODULES_ENABLE_FLIPBOOK_PLAYER)
	# Set the module name here
	set(MODULE_NAME flipbookPlayer)

	# Set module name and source files here
	add_library(${MODULE_NAME} SHARED 
		flipbookPlayer.cpp)

	# Set the module library dependencies here
	target_link_libraries(${MODULE_NAME}
		omega)

	#------------------------------------------------------------------------------
	# DO NOT MODIFY ANYTHING BELOW AFTER THIS LINE
	set_target_properties(${MODULE_NAME} PROPERTIES PREFIX "")
	if(WIN32)
		set_target_properties(${MODULE_NAME} PROPERTIES FOLDER modules SUFFIX ".pyd")
	endif()
endif()