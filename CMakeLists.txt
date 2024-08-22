cmake_minimum_required(VERSION 3.5.0)

# Nom et version du projet
project(ROMHACK_B3313_CARTOGRAPHY VERSION 0.1.0 LANGUAGES CXX)

# Set the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Trouver les modules nécessaires de Qt
find_package(Qt6 6.7 REQUIRED COMPONENTS Widgets Gui Core OpenGLWidgets Quick Qml)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Ajouter les répertoires d'inclusion
set(ALL_INCLUDE_DIR
    "${CMAKE_SOURCE_DIR}/ThirdPartyLibs/nlohmann-json"
    "${CMAKE_SOURCE_DIR}/includes"
)

# Créer l'exécutable principal
file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp ${PROJECT_SOURCE_DIR}/src/*.hpp  ${PROJECT_SOURCE_DIR}/src/*.h)
add_executable(ROMHACK_B3313_CARTOGRAPHY ${SOURCES})

# Ajouter les répertoires d'inclusion pour Qt
target_include_directories(${PROJECT_NAME} PUBLIC
    ${ALL_INCLUDE_DIR}
)
qt_add_resources(RESOURCES resources.qrc)
# Lier Qt aux cibles
target_link_libraries(ROMHACK_B3313_CARTOGRAPHY
    Qt6::Widgets
    Qt6::Gui
    Qt6::Core 
    Qt6::OpenGLWidgets
    Qt6::Quick
    Qt6::Qml
)

# Définir les options de compilation pour le mode Debug
target_compile_definitions(ROMHACK_B3313_CARTOGRAPHY PRIVATE $<$<CONFIG:Debug>:DEBUG>)

# Copier les ressources dans le répertoire de construction
add_custom_command(TARGET ROMHACK_B3313_CARTOGRAPHY POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E remove_directory "$<TARGET_FILE_DIR:ROMHACK_B3313_CARTOGRAPHY>/resources"
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${CMAKE_SOURCE_DIR}/resources"
  "$<TARGET_FILE_DIR:ROMHACK_B3313_CARTOGRAPHY>/resources"
  COMMENT "Updating resources in the build directory"
)

# Find windeployqt
find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${Qt6_DIR}/../../../bin")

# Custom command to run windeployqt
add_custom_command(TARGET ROMHACK_B3313_CARTOGRAPHY POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Running windeployqt..."
    COMMAND ${WINDEPLOYQT_EXECUTABLE} $<TARGET_FILE:ROMHACK_B3313_CARTOGRAPHY>
)

# Inclure les tests et le packaging
include(CTest)
enable_testing()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
