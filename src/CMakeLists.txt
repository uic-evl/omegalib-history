set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}")
include(ExternalProject)
#include(OmegaAddTutorial)
include(UsePch)

# Set virtual memory size for precompiled header
if(WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zm500")
endif()


include(${OmegaLib_SOURCE_DIR}/external/UseOmicron.cmake)

set(OMEGA_HARDCODE_DATA_PATHS true CACHE BOOL "When set to true, the data and application paths will be hardcoded in macros and will be used default for file lookups")

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
# Shi
set(OMEGA_USE_OSGBULLET true CACHE BOOL "Enable osgBullet support")
# Yin
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
add_subdirectory(${CMAKE_SOURCE_DIR}/examples/ohello ${CMAKE_BINARY_DIR}/examples/ohello)
add_subdirectory(${CMAKE_SOURCE_DIR}/examples/ohello2 ${CMAKE_BINARY_DIR}/examples/ohello2)
add_subdirectory(${CMAKE_SOURCE_DIR}/examples/omegaSound ${CMAKE_BINARY_DIR}/examples/omegaSound)
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
	add_subdirectory(cyclops)
	add_subdirectory(${CMAKE_SOURCE_DIR}/examples/cyhello ${CMAKE_BINARY_DIR}/examples/cyhello)
	add_subdirectory(${CMAKE_SOURCE_DIR}/examples/cyhello2 ${CMAKE_BINARY_DIR}/examples/cyhello2)
	add_subdirectory(apps/osgviewer)
	if(OMEGA_USE_OSGBULLET)
    	#add osgBullet into include
    	include(${CMAKE_SOURCE_DIR}/external/UseOsgBullet.cmake)
		include_directories(${OSGBULLET_INCLUDES})

		add_subdirectory(apps/osgbCollision)
		add_subdirectory(apps/osgbBCC)
		#add_subdirectory(apps/osgbCollisionWithoutOmega)
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

# Sets the path to the omega applications source root (if present)
set(OMEGA_APP_DIR CACHE PATH "omegalib application directory")
if(OMEGA_APP_DIR)
	add_subdirectory(${OMEGA_APP_DIR} ${CMAKE_BINARY_DIR}/apps)
	file(TO_CMAKE_PATH ${OMEGA_APP_DIR} OMEGA_FIXED_APP_DIR)
	# Set the OMEGA_APPROOT_DIRECTORY to the app directory root
	# (it will be used to set a data lookup path)
	add_definitions(-DOMEGA_APPROOT_DIRECTORY="${OMEGA_FIXED_APP_DIR}")
endif()

# Add the core omega project
add_subdirectory(omega)

if(OMEGA_USE_PYTHON)
	# We need to add orun here because its config file uses build options defined above
	add_subdirectory(apps/orun)
	add_subdirectory(apps/mcsend)
	add_subdirectory(apps/mcserver)
endif()

# Add the modules subdirectory
add_subdirectory(${CMAKE_SOURCE_DIR}/modules ${CMAKE_BINARY_DIR}/modules)

configure_file(${CMAKE_SOURCE_DIR}/CMakeModules/OmegalibConfig.cmake.in ${CMAKE_BINARY_DIR}/OmegalibConfig.cmake)
if(WIN32)
	configure_file(${CMAKE_SOURCE_DIR}/MakeRelease.bat.in ${CMAKE_BINARY_DIR}/MakeRelease.bat)
	# Copy WinSparkle dll to output folder
	if(OMEGA_ENABLE_AUTO_UPDATE)
		configure_file(${CMAKE_SOURCE_DIR}/external/libs32/WinSparkle.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG} COPYONLY)
		configure_file(${CMAKE_SOURCE_DIR}/external/libs32/WinSparkle.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE} COPYONLY)
	endif()
endif()


