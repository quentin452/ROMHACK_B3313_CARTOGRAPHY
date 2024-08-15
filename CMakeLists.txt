cmake_minimum_required(VERSION 3.16)

# Nom et version du projet
project(ROMHACK_B3313_CARTOGRAPHY VERSION 0.1.0 LANGUAGES CXX)

# Trouver SFML
set(SFML_DIR "${CMAKE_SOURCE_DIR}/ThirdPartyLibs/SFML-2.6.1/lib/cmake/SFML")
find_package(SFML 2.5 COMPONENTS system window graphics REQUIRED)


set(ALL_INCLUDE_DIR
    "${CMAKE_SOURCE_DIR}/ThirdPartyLibs/nlohmann-json"
    "${CMAKE_SOURCE_DIR}/includes"
)
# Créer l'exécutable principal
file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)
add_executable(ROMHACK_B3313_CARTOGRAPHY ${SOURCES})

# Ajouter les répertoires d'inclusion et de lien pour SFML
include_directories("${CMAKE_SOURCE_DIR}/ThirdPartyLibs/SFML-2.6.1/include")
link_directories("${CMAKE_SOURCE_DIR}/ThirdPartyLibs/SFML-2.6.1/lib")

target_include_directories(${PROJECT_NAME} PUBLIC
${ALL_INCLUDE_DIR}
)
# Lier SFML aux cibles
target_link_libraries(ROMHACK_B3313_CARTOGRAPHY sfml-system sfml-window sfml-graphics)

target_compile_definitions(ROMHACK_B3313_CARTOGRAPHY PRIVATE $<$<CONFIG:Debug>:DEBUG>)

# Copier les DLL de SFML dans le répertoire de construction
if(MSVC)
  add_custom_command(TARGET ROMHACK_B3313_CARTOGRAPHY POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/ThirdPartyLibs/SFML-2.6.1/bin"
    "$<TARGET_FILE_DIR:ROMHACK_B3313_CARTOGRAPHY>"
    COMMENT "Copying SFML DLLs to the build directory"
  )
endif()

# Ajouter une commande personnalisée pour supprimer le répertoire de destination avant de copier les nouveaux fichiers
add_custom_command(TARGET ROMHACK_B3313_CARTOGRAPHY POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E remove_directory "$<TARGET_FILE_DIR:ROMHACK_B3313_CARTOGRAPHY>/resources"
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${CMAKE_SOURCE_DIR}/resources"
  "$<TARGET_FILE_DIR:ROMHACK_B3313_CARTOGRAPHY>/resources"
  COMMENT "Updating resources in the build directory"
)

# Inclure les tests et le packaging
include(CTest)
enable_testing()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)