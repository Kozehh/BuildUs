##
## Usual cmake for tp2
##
add_library(Lib STATIC
    buildussystem.cpp)
if (UNIX)
target_link_libraries(Lib 
    PUBLIC 
		${Boost_LIBRARIES}
		${ZLIB_LIBRARIES}
		yaml-cpp
		
)
else ()
target_link_libraries(Lib
    PUBLIC 
		${Boost_LIBRARIES}
)
endif ()	
target_include_directories(Lib
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
		${Boost_INCLUDE_DIRS}
${yaml-cpp_SOURCE_DIR}

)
