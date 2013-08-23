if(WIN32)
	set(EXTLIB_NAME fbxsdk_win-2014-vs10)
	set(EXTLIB_DIR ${CMAKE_BINARY_DIR}/fbxsdk_win)
elseif(UNIX)
	if(APPLE)
		set(EXTLIB_NAME fbxsdk_macosx)
		set(EXTLIB_DIR ${CMAKE_BINARY_DIR}/fbxsdk_macosx)
	else(APPLE)
		set(EXTLIB_NAME fbxsdk_linux-2014)
		set(EXTLIB_DIR ${CMAKE_BINARY_DIR}/fbxsdk_linux)
	endif(APPLE)
endif()

set(EXTLIB_TGZ ${CMAKE_BINARY_DIR}/${EXTLIB_NAME}.tar.gz)
if(NOT EXISTS ${EXTLIB_DIR})
	message(STATUS "Downloading FBX SDK binary archive...")
	file(DOWNLOAD http://omegalib.googlecode.com/files/${EXTLIB_NAME}.tar.gz ${EXTLIB_TGZ} SHOW_PROGRESS)
	execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf ${EXTLIB_TGZ} WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
endif()

set(FBX_DEFAULT_ROOT ${EXTLIB_DIR})

# Setup the Fbx Sdk
set(FBX_ROOT ${FBX_DEFAULT_ROOT} CACHE PATH "The Fbx SDK Root Directory")

set(FBX_INCLUDE_DIR ${FBX_ROOT}/include)
include_directories(${FBX_INCLUDE_DIR})

if(APPLE)
    set(FBX_LIBDIR ${FBX_ROOT}/lib/gcc4)
elseif(CMAKE_COMPILER_IS_GNUCXX)
    SET(FBX_LIBDIR ${FBX_ROOT}/lib/gcc4)
elseif(MSVC80)
	message("ERROR, UNSUPPORTED VISUAL STUDIO VERSION (2005), FBX SDK Version must be downloaded manually")
    SET(FBX_LIBDIR ${FBX_ROOT}/lib/vs2005)
elseif(MSVC90)
	message("ERROR, UNSUPPORTED VISUAL STUDIO VERSION (2008), FBX SDK Version must be downloaded manually")
    SET(FBX_LIBDIR ${FBX_ROOT}/lib/vs2008)
elseif(MSVC10 OR MSVC_VERSION>1600)
    SET(FBX_LIBDIR ${FBX_ROOT}/lib/vs2010)
endif()

IF(APPLE)
	SET(FBX_LIBDIR ${FBX_LIBDIR}/ub)
elseif(CMAKE_CL_64)
    SET(FBX_LIBDIR ${FBX_LIBDIR}/x64)
elseif(CMAKE_COMPILER_IS_GNUCXX AND CMAKE_SIZEOF_VOID_P EQUAL 8)
    SET(FBX_LIBDIR ${FBX_LIBDIR}/x64)
else()
    SET(FBX_LIBDIR ${FBX_LIBDIR}/x86)
endif()

IF(APPLE)
    SET(FBX_LIBNAME ${FBX_LIBDIR}/libfbxsdk-2012.2.dylib ${FBX_LIBDIR}/libfbxsdk-2012.2d.dylib)
	file(COPY ${FBX_LIBDIR}/libfbxsdk-2012.2.dylib DESTINATION ${CMAKE_BINARY_DIR}/bin)
	file(COPY ${FBX_LIBDIR}/libfbxsdk-2012.2d.dylib DESTINATION ${CMAKE_BINARY_DIR}/bin)
elseif(CMAKE_COMPILER_IS_GNUCXX)
    SET(FBX_LIBNAME ${FBX_LIBDIR}/release/libfbxsdk.a)
ELSE()
	set(FBX_LIBNAME debug ${FBX_LIBDIR}/debug/libfbxsdk-md.lib optimized ${FBX_LIBDIR}/release/libfbxsdk-md.lib)
endif()
