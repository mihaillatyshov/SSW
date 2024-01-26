cmake_minimum_required(VERSION 3.12)
cmake_policy(SET CMP0025 NEW)
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.15")
    cmake_policy(SET CMP0093 NEW)
endif()

project(App VERSION 1.0)

set(SOURCES     
    src/main.cpp
    
    src/EditorLayer.cpp                             src/EditorLayer.h   

    src/Calculations/Steps.cpp                      src/Calculations/Steps.h 
    src/Calculations/Calculations.cpp               src/Calculations/Calculations.h

    src/Graphics/GraphicsUtils.cpp                  src/Graphics/GraphicsUtils.h 
    src/Graphics/SimpleRenderable2D.cpp             src/Graphics/SimpleRenderable2D.h

    src/Gui/CustomGui.cpp                           src/Gui/CustomGui.h

    src/Math/Angle.cpp                              src/Math/Angle.h 
    src/Math/Intersections.cpp                      src/Math/Intersections.h 
    src/Math/Length.cpp                             src/Math/Length.h
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(${PROJECT_NAME} PRIVATE Engine)
# add_subdirectory(tests)

# if(MSVC)
#     set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "App")
# endif()
