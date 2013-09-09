# Shi
# Source files
set(SRCS 
		osgbBCC.cpp)
    
###################################################################################################
# Setup compile info
add_executable(osgbBCC ${SRCS})
set_target_properties(osgbBCC PROPERTIES FOLDER osgBulletExamples)
target_link_libraries(osgbBCC
	omega
	omegaToolkit
	omegaOsg
	${BULLET_LIBS}
	${OSGBULLET_LIBS})
# Yin