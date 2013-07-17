set(MODULES_ENABLE_RIFT false CACHE BOOL "Enable support for Oculus Rift")

if(MODULES_ENABLE_RIFT)
	# Set the module name here
	set(MODULE_NAME rift)

	set(RIFT_EMULATE false CACHE BOOL "When set to true, compiles the rift module without using the OVR library")
	if(RIFT_EMULATE)
		add_definitions(-DRIFT_EMULATE)
	endif()

	if(NOT RIFT_EMULATE)
		# Oculus SDK include and library
		set(OVR_INCLUDE_DIR CACHE PATH "Oculus SDK include directory")
		set(OVR_LIB CACHE PATH "Oculus SDK library")
		include_directories(${OVR_INCLUDE_DIR})
	endif()
	
	# Set module name and source files here
	add_library(${MODULE_NAME} SHARED 
		rift.cpp)

	# Set the module library dependencies here
	target_link_libraries(${MODULE_NAME}
		omega)

	if(NOT RIFT_EMULATE)
		target_link_libraries(${MODULE_NAME}
			${OVR_LIB})
		
		# link with libraries needed on a linux install.
		if( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )
		  set_target_properties(${MODULE_NAME} PROPERTIES LINK_FLAGS "-ludev -lXinerama")
		endif( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )
	endif()
	
	#------------------------------------------------------------------------------
	# DO NOT MODIFY ANYTHING BELOW AFTER THIS LINE
	set_target_properties(${MODULE_NAME} PROPERTIES PREFIX "")
	if(WIN32)
		set_target_properties(${MODULE_NAME} PROPERTIES FOLDER modules SUFFIX ".pyd")
	endif()
endif()