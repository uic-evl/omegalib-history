set(MODULES_ENABLE_RIFT false CACHE BOOL "Enable support for Oculus Rift")

# Set the module name here
SET(MODULE_NAME rift)

# Set module name and source files here
add_library(${MODULE_NAME} SHARED 
	rift.cpp)

# Set the module library dependencies here
target_link_libraries(${MODULE_NAME}
	omega)

#------------------------------------------------------------------------------
# DO NOT MODIFY ANYTHING BELOW AFTER THIS LINE
set_target_properties(${MODULE_NAME} PROPERTIES PREFIX "")
if(WIN32)
	set_target_properties(${MODULE_NAME} PROPERTIES FOLDER modules SUFFIX ".pyd")
endif()
