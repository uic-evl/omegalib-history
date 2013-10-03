include(ExternalProject)
#include(OmegaAddTutorial)
include(UsePch)

# Set virtual memory size for precompiled header
if(WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zm500")
endif()


include(${OmegaLib_SOURCE_DIR}/external/UseOmicron.cmake)

include_directories(../modules)


###################################################################################################
# Visual Leak Detector Support
if(OMEGA_OS_WIN)
	set(OMEGA_ENABLE_VLD false CACHE BOOL "Enables Visual Leak Detector support for this build.")
endif()

###################################################################################################
# See if we can setup Python support for this build
set(OMEGA_ENABLE_PYTHON true CACHE BOOL "Enables python support if available.")
if(OMEGA_ENABLE_PYTHON)
	if(APPLE)
		set(OMEGA_USE_PYTHON true)
		include(${CMAKE_SOURCE_DIR}/external/CompilePython.cmake)
		include_directories(${PYTHON_INCLUDE_DIRS})
	elseif(WIN32)
		# On windows use the included python distribution
		set(OMEGA_USE_PYTHON true)
		include(${OmegaLib_SOURCE_DIR}/external/UsePython.cmake)
		set(PYTHON_INCLUDE_DIRS ${OmegaLib_BINARY_DIR}/python/include CACHE PATH "fdf")
		set(PYTHON_LIBRARIES ${OmegaLib_BINARY_DIR}/python/libs/python27.lib CACHE PATH "fd")
		include_directories(${PYTHON_INCLUDE_DIRS})
		# copy the python dll to the output folders
		configure_file(${CMAKE_SOURCE_DIR}/external/libs32/python27.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG} COPYONLY)
		configure_file(${CMAKE_SOURCE_DIR}/external/libs32/python27.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE} COPYONLY)
	elseif(OMEGA_OS_LINUX)
		find_package(PythonLibs)
		if(PYTHONLIBS_FOUND)
			set(OMEGA_USE_PYTHON true)
			include_directories(${PYTHON_INCLUDE_DIRS})
			include(${OmegaLib_SOURCE_DIR}/external/UsePython.cmake)
		else()
			message("Python libraries not found on this system. Omegalib Python support will be disabled")
		endif()
	endif(APPLE)
else()
	set(OMEGA_USE_PYTHON false)
endif()

###################################################################################################
# Options
set(OMEGA_BUILD_OSG_LIB true CACHE BOOL "Enable build of OpenSceneGraph integration library")
set(OMEGA_USE_OSGBULLET true CACHE BOOL "Enable osgBullet support")
set(OMEGA_BUILD_VTK_LIB false CACHE BOOL "Enable build of vtk integration library (Vtk has to be compiled with Python support)")
if(WIN32)
	set(OMEGA_ENABLE_AUTO_UPDATE true CACHE BOOL "Enables the omegalib binaries auto updater (WinSparkle) for a windows build. Supported by 32bit builds only.")
endif()

###################################################################################################
# Set include paths
if(APPLE)
	include_directories(
	  ${CMAKE_BINARY_DIR}/include/
	  ${CMAKE_SOURCE_DIR}/include/
	  ${CMAKE_SOURCE_DIR}/external/include
	  ${OMICRON_INCLUDE_DIRS}
	  )
else(APPLE)
	include_directories(
	  ${CMAKE_BINARY_DIR}/include/
	  ${CMAKE_SOURCE_DIR}/include/
  	  ${CMAKE_SOURCE_DIR}/src/3rdparty/glew  
	  ${CMAKE_SOURCE_DIR}/external/include
	  ${OMICRON_INCLUDE_DIRS}
	  )
endif(APPLE)

###################################################################################################
# Add libraries and applications to the project.

# Add some libraries used for display support.
include(${OmegaLib_SOURCE_DIR}/external/UseFreeImage.cmake)

include(${OmegaLib_SOURCE_DIR}/external/UseFreeType.cmake)
include(${OmegaLib_SOURCE_DIR}/external/UseFtgl.cmake)
include_directories(
  ${OmegaLib_BINARY_DIR}/ftgl/src/
  ${OmegaLib_BINARY_DIR}/freetype/include/
  ${OmegaLib_BINARY_DIR}/FreeImage/Source/
)

if(NOT APPLE)
	add_subdirectory(3rdparty/glew)
endif(NOT APPLE)

# Add several example applications
add_subdirectory(${CMAKE_SOURCE_DIR}/examples/ohello ${CMAKE_BINARY_DIR}/examples/ohello)
add_subdirectory(${CMAKE_SOURCE_DIR}/examples/ohello2 ${CMAKE_BINARY_DIR}/examples/ohello2)
add_subdirectory(${CMAKE_SOURCE_DIR}/examples/ohelloWidgets ${CMAKE_BINARY_DIR}/examples/ohelloWidgets)
add_subdirectory(${CMAKE_SOURCE_DIR}/examples/omegaSound ${CMAKE_BINARY_DIR}/examples/omegaSound)

# Add the omegaToolkit library
add_subdirectory(omegaToolkit)

# OSX uses its own glut, do not compile it internally (Should we do the same under linux?)
set(OMEGA_USE_DISPLAY_GLUT false CACHE BOOL "Enable GLUT display system support")
if(OMEGA_USE_DISPLAY_GLUT)
	if(APPLE)
		find_package(GLUT)
	endif(APPLE)
    if(NOT APPLE) 
	    add_subdirectory(3rdparty/freeglut)
    endif()
endif()

# Open Scene Graph configuration, libraries and applications
if(OMEGA_BUILD_OSG_LIB)
	include(${OmegaLib_SOURCE_DIR}/external/UseOpenSceneGraph.cmake)
	include_directories(${OSG_INCLUDES})
	add_subdirectory(omegaOsg)
	add_subdirectory(apps/osgviewer)
	if(OMEGA_USE_OSGBULLET)
    	#add osgBullet into include
    	include(${CMAKE_SOURCE_DIR}/external/UseOsgBullet.cmake)
		include_directories(${OSGBULLET_INCLUDES})

		add_subdirectory(apps/osgbCollision)
		add_subdirectory(apps/osgbBasicDemo)
		add_subdirectory(apps/osgbDice)
		add_subdirectory(apps/osgbSlider)
	endif()
	if(OMEGA_USE_PORTHOLE)
		add_subdirectory(${CMAKE_SOURCE_DIR}/examples/portholeCustomDraw ${CMAKE_BINARY_DIR}/examples/portholeCustomDraw)
	endif()
endif()

# vtk configuration, libraries and applications
if(OMEGA_BUILD_VTK_LIB)
	find_package(VTK)
	if(NOT VTK_DIR)
	  message(FATAL_ERROR "Please set VTK_DIR.")
	endif(NOT VTK_DIR)
	include(${VTK_USE_FILE})
	
	add_subdirectory(omegaVtk)
	add_subdirectory(${CMAKE_SOURCE_DIR}/examples/vtkhello ${CMAKE_BINARY_DIR}/examples/vtkhello)
endif(OMEGA_BUILD_VTK_LIB)

# Add the core omega project
add_subdirectory(omega)

if(OMEGA_USE_PYTHON)
	# We need to add orun here because its config file uses build options defined above
	add_subdirectory(apps/orun)
	add_subdirectory(apps/mcsend)
	add_subdirectory(apps/mcserver)
endif()

# Generate the Omegalib config fule used by external applications to find omegalib.
# The @ONLY option is to substitute only variables in form @VAR@. This is because
# OmegalibConfig.cmake uses ${VAR} variables that will be substituted by the
# external application.
configure_file(${CMAKE_SOURCE_DIR}/CMakeModules/OmegalibConfig.cmake.in ${CMAKE_BINARY_DIR}/OmegalibConfig.cmake @ONLY)

# Windows-specific config operations
if(WIN32)
	# Set up the MakeRelease script. It will be used to generate release packages on windows
	configure_file(${CMAKE_SOURCE_DIR}/MakeRelease.bat.in ${CMAKE_BINARY_DIR}/MakeRelease.bat)
	# Copy The WinSparkle dll to output folder. WInsparkle is used to notify users of omegalib updates.
	if(OMEGA_ENABLE_AUTO_UPDATE)
		configure_file(${CMAKE_SOURCE_DIR}/external/libs32/WinSparkle.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG} COPYONLY)
		configure_file(${CMAKE_SOURCE_DIR}/external/libs32/WinSparkle.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE} COPYONLY)
	endif()
endif()

###############################################################################
# Setup the module loader
include(ModuleUtils)
find_package(git)
if(GIT_FOUND)
	set(OMEGA_HUB_URL "http://github.com/omega-hub/hub" CACHE STRING "url to git repository containing the module index")
	
	# Delete the native modules CMakeLists file. it will be regenerated by 
	# module_def calls for native modules
	set(MODULES_CMAKE_FILE ${CMAKE_SOURCE_DIR}/modules/CMakeLists.txt)
	file(REMOVE ${MODULES_CMAKE_FILE})
	file(APPEND ${MODULES_CMAKE_FILE} "#auto-generated file\n")
	
	# delete the modulesConfig.h file, it will be regenerated by module_def
	# for native modules.
	set(MODULES_CONFIG_FILE ${CMAKE_SOURCE_DIR}/modules/modulesConfig.h)
	file(REMOVE ${MODULES_CONFIG_FILE})
	file(APPEND ${MODULES_CONFIG_FILE} "//auto-generated file\n")
	
	# download the index CMakeLists file
	file(DOWNLOAD ${OMEGA_HUB_URL}/raw/master/CMakeLists.txt ${CMAKE_BINARY_DIR}/modules/CMakeLists.txt SHOW_PROGRESS)
	if(EXISTS ${CMAKE_BINARY_DIR}/modules/CMakeLists.txt)
		# Add the CMakeLists with the module index. Need to specify a binary 
		# output path since the file is not in the source tree.
		add_subdirectory(${CMAKE_BINARY_DIR}/modules ${CMAKE_BINARY_DIR}/modules/build)
	else()
		message("Could not find module master index file")
	endif()
endif()

# Add the modules subdirectory. This will include cmake scripts for all native modules
add_subdirectory(${CMAKE_SOURCE_DIR}/modules ${CMAKE_BINARY_DIR}/modules)

###############################################################################
# Install commands
install(DIRECTORY ../include DESTINATION omegalib)
install(DIRECTORY ../CMakeModules DESTINATION omegalib)

# omicron
install(DIRECTORY ../omicron/CMakeModules DESTINATION omegalib/omicron)
install(DIRECTORY ../omicron/include DESTINATION omegalib)
install(DIRECTORY ../omicron/external/include DESTINATION omegalib/external)
install(FILES ${CMAKE_BINARY_DIR}/omicron/omicron/UseOmicron.cmake DESTINATION omegalib/omicron/omicron)
install(DIRECTORY ${CMAKE_BINARY_DIR}/omicron/omicron/include DESTINATION omegalib/omicron/omicron)

if(WIN32)
	# on windows, install only the release dir, and also copy the export libs.
	install(DIRECTORY ${CMAKE_BINARY_DIR}/bin/release/ DESTINATION omegalib/bin)
	install(DIRECTORY ${CMAKE_BINARY_DIR}/lib/release/ DESTINATION omegalib/lib FILES_MATCHING PATTERN "*.lib")
else()
	install(DIRECTORY ${CMAKE_BINARY_DIR}/bin DESTINATION omegalib)
endif()
# osgworks
install(DIRECTORY ${CMAKE_BINARY_DIR}/src/osgWorks-prefix/src/osgWorks-build/lib/ DESTINATION omegalib/lib)

# modules and examples
install(DIRECTORY ../modules DESTINATION omegalib)
install(DIRECTORY ../examples/python/ DESTINATION omegalib/examples)

# data
install(DIRECTORY ../cubemaps DESTINATION omegalib)
install(DIRECTORY ../evl DESTINATION omegalib)
install(DIRECTORY ../fonts DESTINATION omegalib)
install(DIRECTORY ../menu_sounds DESTINATION omegalib)
install(DIRECTORY ../ui DESTINATION omegalib)
install(DIRECTORY ../porthole DESTINATION omegalib)
install(DIRECTORY ../system DESTINATION omegalib)
install(FILES ../default.cfg DESTINATION omegalib)
install(FILES ../default_init.py DESTINATION omegalib)
install(FILES ../omegalib-transparent-white.png DESTINATION omegalib)
install(FILES ${CMAKE_BINARY_DIR}/OmegalibConfig.cmake DESTINATION omegalib)
install(FILES ${CMAKE_BINARY_DIR}/include/omegaConfig.h DESTINATION omegalib/include)


