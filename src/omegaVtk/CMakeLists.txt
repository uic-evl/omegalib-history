ADD_DEFINITIONS( -D_CRT_SECURE_NO_WARNINGS -DOVTK_EXPORTING -DVTK_LIBRARY_DIR="${VTK_LIBRARY_DIRS}" -DVTK_PYTHON_DIR="${VTK_LIBRARY_DIRS}/../Wrapping/Python")

SET( srcs 
		vtkGenericOpenGLRenderWindow.cpp
		CustomDepthPeelingPass.cpp
		VtkModule.cpp
		VtkRenderPass.cpp
		VtkAttachPoint.cpp
        )
		
if(OMEGA_USE_PYTHON)
SET( srcs ${srcs}
		PyVtk.cpp
		omegaVtkPythonApi.cpp
        )
endif()
		
SET( headers 
		# omega
		CustomDepthPeelingPass.h
		${CMAKE_SOURCE_DIR}/include/omegaVtk/ovtkbase.h
		${CMAKE_SOURCE_DIR}/include/omegaVtk/PyVtk.h
		${CMAKE_SOURCE_DIR}/include/omegaVtk/VtkModule.h
		${CMAKE_SOURCE_DIR}/include/omegaVtk/vtkGenericOpenGLRenderWindow.h
		${CMAKE_SOURCE_DIR}/include/omegaVtk/VtkRenderPass.h
		${CMAKE_SOURCE_DIR}/include/omegaVtk/VtkAttachPoint.h
		${CMAKE_SOURCE_DIR}/include/omegaVtk/omegaVtkPythonApi.h
        ) 
        
SOURCE_GROUP(omegaVtk REGULAR_EXPRESSION omegaVtk/)		

###############################################################################
# Setup compile info
ADD_LIBRARY( omegaVtk SHARED ${srcs} ${headers})

###############################################################################
# Setup link info.
TARGET_LINK_LIBRARIES( omegaVtk 
	omega 
	omegaToolkit
    vtkRendering
	)
