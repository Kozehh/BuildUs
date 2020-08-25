add_executable(buildtests 
    	tests.cpp
    	catch.hpp
)


target_link_libraries(buildtests
    PRIVATE
    Lib
)

target_include_directories(buildtests
    PRIVATE
    Lib
)
