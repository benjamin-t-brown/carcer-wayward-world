cmake_minimum_required(VERSION 3.28)

project(carcer_wayward_world LANGUAGES CXX)

include(CTest)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(CARCER_ALLOW_DIRTY_DEPS "Allow tracked changes in pinned dependency checkouts" OFF)
option(CARCER_BUILD_TESTS "Build the non-UI C++ test suite" ON)
option(CARCER_BUILD_UI_TESTS "Provide compile-only UI test targets" ON)
set(CARCER_DEPS_ROOT "${CMAKE_SOURCE_DIR}/.deps" CACHE PATH
    "Directory containing pinned SDL2W and BMIN source checkouts")
set(CARCER_DEPS_LOCK "${CMAKE_SOURCE_DIR}/deps.lock" CACHE FILEPATH
    "Dependency revision lock used by the bootstrap validator")

find_program(CARCER_BASH_EXECUTABLE NAMES bash REQUIRED)

set(_carcer_dependency_check_args --check)
if(CARCER_ALLOW_DIRTY_DEPS)
  list(APPEND _carcer_dependency_check_args --allow-dirty)
endif()
set(_carcer_dependency_check_command
    "${CMAKE_COMMAND}" -E env
    "CARCER_DEPS_ROOT=${CARCER_DEPS_ROOT}"
    "CARCER_DEPS_LOCK=${CARCER_DEPS_LOCK}"
    "${CARCER_BASH_EXECUTABLE}"
    "${CMAKE_SOURCE_DIR}/scripts/bootstrap-deps.sh"
    ${_carcer_dependency_check_args})

# Configure and every default build validate, but never fetch or move a checkout.
execute_process(
  COMMAND ${_carcer_dependency_check_command}
  RESULT_VARIABLE _carcer_dependency_check_result
  COMMAND_ECHO STDOUT)
if(NOT _carcer_dependency_check_result EQUAL 0)
  message(FATAL_ERROR
          "Pinned dependencies are unavailable. Run scripts/bootstrap-deps.sh first.")
endif()
add_custom_target(validate-deps ALL
  COMMAND ${_carcer_dependency_check_command}
  COMMENT "Validating pinned SDL2W and BMIN revisions"
  VERBATIM)

if(EMSCRIPTEN)
  set(_carcer_dependency_target wasm)
else()
  set(_carcer_dependency_target native)
endif()

file(READ "${CARCER_DEPS_LOCK}" _carcer_lock_contents)
string(TOUPPER "${CMAKE_BUILD_TYPE}" _carcer_build_type_upper)
set(_carcer_config_flags "${CMAKE_CXX_FLAGS_${_carcer_build_type_upper}}")
string(CONCAT _carcer_consumer_flags
       "${CMAKE_CXX_COMPILER}|${CMAKE_CXX_COMPILER_VERSION}|${CMAKE_SYSTEM_NAME}|"
       "${CMAKE_SYSTEM_PROCESSOR}|${CMAKE_BUILD_TYPE}|${CMAKE_CXX_FLAGS}|"
       "${_carcer_config_flags}")
string(CONCAT _carcer_consumer_identity
       "${_carcer_consumer_flags}|${_carcer_lock_contents}")
string(SHA256 _carcer_consumer_hash "${_carcer_consumer_identity}")
string(SUBSTRING "${_carcer_consumer_hash}" 0 12 _carcer_consumer_key)
string(MAKE_C_IDENTIFIER
       "${CMAKE_CXX_COMPILER_ID}-${CMAKE_CXX_COMPILER_VERSION}-${_carcer_dependency_target}"
       _carcer_toolchain_name)
set(CARCER_DEPENDENCY_BUNDLE
    "${CMAKE_SOURCE_DIR}/build/compat/${_carcer_toolchain_name}-${_carcer_consumer_key}")
set(_carcer_dependency_stamp
    "${CARCER_DEPENDENCY_BUNDLE}/.carcer-headers-${_carcer_dependency_target}-build-key")
file(MAKE_DIRECTORY
     "${CARCER_DEPENDENCY_BUNDLE}/include"
     "${CARCER_DEPENDENCY_BUNDLE}/include/sdl2w"
     "${CARCER_DEPENDENCY_BUNDLE}/include/bmin"
     "${CARCER_DEPENDENCY_BUNDLE}/lib")

set(_carcer_dependency_build_env
    "CARCER_DEPS_ROOT=${CARCER_DEPS_ROOT}"
    "CARCER_DEPS_LOCK=${CARCER_DEPS_LOCK}"
    "CARCER_SDL2W_BUNDLE=${CARCER_DEPENDENCY_BUNDLE}"
    "CARCER_DEPS_CXX=${CMAKE_CXX_COMPILER}"
    "CARCER_DEPS_TARGET=${_carcer_dependency_target}"
    "CARCER_DEPS_CONSUMER_FLAGS=${_carcer_consumer_flags}")
if(CARCER_ALLOW_DIRTY_DEPS)
  list(APPEND _carcer_dependency_build_env "CARCER_ALLOW_DIRTY_DEPS=1")
endif()

add_custom_command(
  OUTPUT "${_carcer_dependency_stamp}"
  BYPRODUCTS
    "${CARCER_DEPENDENCY_BUNDLE}/lib/libsdl2w.a"
    "${CARCER_DEPENDENCY_BUNDLE}/lib/libbmin.a"
  COMMAND "${CMAKE_COMMAND}" -E env ${_carcer_dependency_build_env}
          "${CARCER_BASH_EXECUTABLE}"
          "${CMAKE_SOURCE_DIR}/scripts/build-deps.sh" headers
  DEPENDS
    "${CMAKE_SOURCE_DIR}/deps.lock"
    "${CMAKE_SOURCE_DIR}/scripts/bootstrap-deps.sh"
    "${CMAKE_SOURCE_DIR}/scripts/build-deps.sh"
  COMMENT "Building pinned SDL2W/BMIN classic-header bundle"
  VERBATIM)
add_custom_target(carcer_dependency_bundle ALL DEPENDS "${_carcer_dependency_stamp}")
add_dependencies(carcer_dependency_bundle validate-deps)

add_library(carcer_bmin STATIC IMPORTED GLOBAL)
set_target_properties(carcer_bmin PROPERTIES
  IMPORTED_LOCATION "${CARCER_DEPENDENCY_BUNDLE}/lib/libbmin.a"
  INTERFACE_INCLUDE_DIRECTORIES "${CARCER_DEPENDENCY_BUNDLE}/include")
add_dependencies(carcer_bmin carcer_dependency_bundle)

add_library(carcer_sdl2w STATIC IMPORTED GLOBAL)
set_target_properties(carcer_sdl2w PROPERTIES
  IMPORTED_LOCATION "${CARCER_DEPENDENCY_BUNDLE}/lib/libsdl2w.a"
  INTERFACE_INCLUDE_DIRECTORIES "${CARCER_DEPENDENCY_BUNDLE}/include")
add_dependencies(carcer_sdl2w carcer_dependency_bundle)
target_link_libraries(carcer_sdl2w INTERFACE carcer_bmin)

if(EMSCRIPTEN)
  set(_carcer_emscripten_sdl_options
      -sUSE_SDL=2
      -sUSE_SDL_IMAGE=2
      -sUSE_SDL_MIXER=2
      "-sSDL2_IMAGE_FORMATS=[\"png\"]"
      -sUSE_SDL_TTF=2
      -sUSE_SDL_GFX=2)
  target_compile_options(carcer_sdl2w INTERFACE ${_carcer_emscripten_sdl_options})
else()
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(SDL2_STACK REQUIRED IMPORTED_TARGET
    sdl2 SDL2_image SDL2_ttf SDL2_mixer SDL2_gfx)

  if(WIN32)
    target_compile_definitions(carcer_sdl2w INTERFACE SDL_MAIN_HANDLED)
    get_target_property(_sdl2_opts PkgConfig::SDL2_STACK INTERFACE_COMPILE_OPTIONS)
    if(_sdl2_opts)
      list(REMOVE_ITEM _sdl2_opts "-Dmain=SDL_main")
      set_property(TARGET PkgConfig::SDL2_STACK PROPERTY
        INTERFACE_COMPILE_OPTIONS "${_sdl2_opts}")
    endif()
    get_target_property(_sdl2_libs PkgConfig::SDL2_STACK INTERFACE_LINK_LIBRARIES)
    if(_sdl2_libs)
      list(FILTER _sdl2_libs EXCLUDE REGEX "SDL2main")
      set_property(TARGET PkgConfig::SDL2_STACK PROPERTY
        INTERFACE_LINK_LIBRARIES "${_sdl2_libs}")
    endif()
    set_property(TARGET PkgConfig::SDL2_STACK PROPERTY INTERFACE_LINK_OPTIONS "")
  endif()
  target_link_libraries(carcer_sdl2w INTERFACE PkgConfig::SDL2_STACK)
endif()

add_library(carcer_build_options INTERFACE)
target_compile_features(carcer_build_options INTERFACE cxx_std_23)
target_compile_options(carcer_build_options INTERFACE
  $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:-Wall;-Wextra;-Wno-unused-parameter;-Wno-missing-field-initializers>
  $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>>:-Og;-g;-fno-omit-frame-pointer>)

include(cmake/carcer_sources.cmake)
add_library(carcer_lib STATIC ${CARCER_SOURCES})
add_dependencies(carcer_lib carcer_dependency_bundle)
set_property(TARGET carcer_lib PROPERTY CXX_SCAN_FOR_MODULES OFF)
target_include_directories(carcer_lib PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(carcer_lib PUBLIC
  carcer_sdl2w carcer_bmin carcer_build_options)

add_executable(CARCER src/main.cpp)
set_property(TARGET CARCER PROPERTY CXX_SCAN_FOR_MODULES OFF)
target_link_libraries(CARCER PRIVATE carcer_lib)
if(WIN32)
  target_sources(CARCER PRIVATE cmake/win_sdl_main_ready.cpp)
endif()

if(EMSCRIPTEN)
  target_link_options(CARCER PRIVATE
    ${_carcer_emscripten_sdl_options}
    -sALLOW_MEMORY_GROWTH=1
    -sSAFE_HEAP=0
    -sASSERTIONS=1
    -sINITIAL_MEMORY=326565888
    -sENVIRONMENT=web
    -sDISABLE_EXCEPTION_CATCHING=0
    -lidbfs.js
    "-sEXPORTED_FUNCTIONS=[\"_main\",\"_enableSound\",\"_disableSound\",\"_setVolume\",\"_setKeyDown\",\"_setKeyUp\",\"_setKeyStatus\",\"_sendEvent\"]"
    "-sEXPORTED_RUNTIME_METHODS=[\"ccall\"]"
    --preload-file "${CMAKE_SOURCE_DIR}/src/assets@assets")
  set_target_properties(CARCER PROPERTIES SUFFIX ".js")
endif()

function(carcer_add_test_executable test_source is_ui)
  get_filename_component(test_name "${test_source}" NAME_WE)
  set(test_target "carcer_test_${test_name}")

  if(is_ui)
    add_executable("${test_target}" EXCLUDE_FROM_ALL "${test_source}")
  else()
    add_executable("${test_target}" "${test_source}")
  endif()
  set_property(TARGET "${test_target}" PROPERTY CXX_SCAN_FOR_MODULES OFF)
  target_link_libraries("${test_target}" PRIVATE carcer_lib)

  if(NOT is_ui)
    add_test(NAME "${test_name}" COMMAND "${test_target}")
    set_tests_properties("${test_name}" PROPERTIES
      WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/src")
  endif()

  set(CARCER_LAST_TEST_TARGET "${test_target}" PARENT_SCOPE)
endfunction()

if(BUILD_TESTING AND CARCER_BUILD_TESTS)
  file(GLOB_RECURSE CARCER_NON_UI_TEST_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/__test__/runner/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/__test__/db/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/__test__/model/*.cpp")
  add_custom_target(carcer_non_ui_tests)
  foreach(test_source IN LISTS CARCER_NON_UI_TEST_SOURCES)
    carcer_add_test_executable("${test_source}" FALSE)
    add_dependencies(carcer_non_ui_tests "${CARCER_LAST_TEST_TARGET}")
  endforeach()
endif()

if(CARCER_BUILD_UI_TESTS)
  file(GLOB_RECURSE CARCER_UI_TEST_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/__test__/ui/*.cpp")
  add_custom_target(carcer_ui_tests)
  foreach(test_source IN LISTS CARCER_UI_TEST_SOURCES)
    carcer_add_test_executable("${test_source}" TRUE)
    add_dependencies(carcer_ui_tests "${CARCER_LAST_TEST_TARGET}")
  endforeach()
endif()

if(BUILD_TESTING)
  find_package(Python3 COMPONENTS Interpreter REQUIRED)
  add_test(NAME IncludeArchitecture
    COMMAND "${Python3_EXECUTABLE}"
            "${CMAKE_SOURCE_DIR}/scripts/check-include-architecture.py"
            --root "${CMAKE_SOURCE_DIR}"
            --source-subsystem model
            --source-subsystem rules
            --source-subsystem state
            --source-subsystem actions)

  file(GLOB_RECURSE CARCER_PHASE3_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/lib/*.h"
    "${CMAKE_SOURCE_DIR}/src/data/*.h"
    "${CMAKE_SOURCE_DIR}/src/model/*.h"
    "${CMAKE_SOURCE_DIR}/src/db/*.h")
  list(APPEND CARCER_PHASE3_HEADERS
    "${CMAKE_SOURCE_DIR}/src/game/combat/CombatParty.h"
    "${CMAKE_SOURCE_DIR}/src/game/inventory/InventoryRules.h"
    "${CMAKE_SOURCE_DIR}/src/game/map/CharacterConstruction.h"
    "${CMAKE_SOURCE_DIR}/src/game/map/TileFields.h")
  set(CARCER_PHASE3_HEADER_PROBES)
  foreach(header IN LISTS CARCER_PHASE3_HEADERS)
    file(RELATIVE_PATH relative_header "${CMAKE_SOURCE_DIR}/src" "${header}")
    string(MD5 header_id "${relative_header}")
    set(probe "${CMAKE_BINARY_DIR}/header-self-containment/phase3/${header_id}.cpp")
    file(WRITE "${probe}" "#include \"${relative_header}\"\nint carcer_header_probe_${header_id} = 0;\n")
    list(APPEND CARCER_PHASE3_HEADER_PROBES "${probe}")
  endforeach()
  add_library(carcer_phase3_header_checks OBJECT EXCLUDE_FROM_ALL
    ${CARCER_PHASE3_HEADER_PROBES})
  set_property(TARGET carcer_phase3_header_checks PROPERTY CXX_SCAN_FOR_MODULES OFF)
  target_link_libraries(carcer_phase3_header_checks PRIVATE carcer_lib)
  add_test(NAME Phase3HeaderSelfContainment
    COMMAND "${CMAKE_COMMAND}" --build "${CMAKE_BINARY_DIR}"
            --target carcer_phase3_header_checks --parallel 8)

  file(GLOB_RECURSE CARCER_PHASE4_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/game/*.h"
    "${CMAKE_SOURCE_DIR}/src/in3/*.h"
    "${CMAKE_SOURCE_DIR}/src/runner/*.h"
    "${CMAKE_SOURCE_DIR}/src/state/*.h")
  set(CARCER_PHASE4_HEADER_PROBES)
  foreach(header IN LISTS CARCER_PHASE4_HEADERS)
    file(RELATIVE_PATH relative_header "${CMAKE_SOURCE_DIR}/src" "${header}")
    string(MD5 header_id "phase4-${relative_header}")
    set(probe "${CMAKE_BINARY_DIR}/header-self-containment/phase4/${header_id}.cpp")
    file(WRITE "${probe}" "#include \"${relative_header}\"\nint carcer_header_probe_${header_id} = 0;\n")
    list(APPEND CARCER_PHASE4_HEADER_PROBES "${probe}")
  endforeach()
  add_library(carcer_phase4_header_checks OBJECT EXCLUDE_FROM_ALL
    ${CARCER_PHASE4_HEADER_PROBES})
  set_property(TARGET carcer_phase4_header_checks PROPERTY CXX_SCAN_FOR_MODULES OFF)
  target_link_libraries(carcer_phase4_header_checks PRIVATE carcer_lib)
  add_test(NAME Phase4HeaderSelfContainment
    COMMAND "${CMAKE_COMMAND}" --build "${CMAKE_BINARY_DIR}"
            --target carcer_phase4_header_checks --parallel 8)

  file(GLOB_RECURSE CARCER_PHASE5_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/actions/*.hpp")
  set(CARCER_PHASE5_HEADER_PROBES)
  foreach(header IN LISTS CARCER_PHASE5_HEADERS)
    file(RELATIVE_PATH relative_header "${CMAKE_SOURCE_DIR}/src" "${header}")
    string(MD5 header_id "phase5-${relative_header}")
    set(probe "${CMAKE_BINARY_DIR}/header-self-containment/phase5/${header_id}.cpp")
    file(WRITE "${probe}" "#include \"${relative_header}\"\nint carcer_header_probe_${header_id} = 0;\n")
    list(APPEND CARCER_PHASE5_HEADER_PROBES "${probe}")
  endforeach()
  add_library(carcer_phase5_header_checks OBJECT EXCLUDE_FROM_ALL
    ${CARCER_PHASE5_HEADER_PROBES})
  set_property(TARGET carcer_phase5_header_checks PROPERTY CXX_SCAN_FOR_MODULES OFF)
  target_link_libraries(carcer_phase5_header_checks PRIVATE carcer_lib)
  add_test(NAME Phase5HeaderSelfContainment
    COMMAND "${CMAKE_COMMAND}" --build "${CMAKE_BINARY_DIR}"
            --target carcer_phase5_header_checks --parallel 8)
endif()
