if("$ENV{SAGE_DIRECTORY}" STREQUAL "")
		message("SAGE_DIRECTORY not found: SAGE support DISABLED")
else()
	set(MODULES_ENABLE_SAGE true CACHE BOOL "Enable SAGE support")
	if(MODULES_ENABLE_SAGE)
		# Set the module name here
		set(MODULE_NAME sage)

		set(SAGE_DIR $ENV{SAGE_DIRECTORY})
		set(SAGE_INCLUDE_DIR ${SAGE_DIR}/include)
		if(WIN32)
			set(SAGE_LIBS 
				${SAGE_DIR}/lib/sail.lib 
				${SAGE_DIR}/win32/lib/pthread.lib 
				${SAGE_DIR}/win32/lib/quanta.lib
				Userenv.lib # needed for GetUserProfileDirectory
				)
			include_directories(${SAGE_DIR}/win32/include)
		else()
			set(SAGE_LIBS 
				${SAGE_DIR}/lib64/libsail.so
				${SAGE_DIR}/lib64/libquanta.so
				)
		endif()
		
		include_directories(${SAGE_INCLUDE_DIR})
	
		# Set module name and source files here
		add_library(${MODULE_NAME} SHARED 
			SageManager.h
			SageManager.cpp
			sage.cpp)

		# Set the module library dependencies here
		target_link_libraries(${MODULE_NAME}
			omega 
			${SAGE_LIBS})

		#------------------------------------------------------------------------------
		# DO NOT MODIFY ANYTHING BELOW AFTER THIS LINE
		set_target_properties(${MODULE_NAME} PROPERTIES PREFIX "")
		if(WIN32)
			set_target_properties(${MODULE_NAME} PROPERTIES FOLDER modules SUFFIX ".pyd")
		endif()
	endif()
endif()
