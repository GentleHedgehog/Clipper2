# link example:
# include(LIBS/Clipper2/CPP/clipper2_lib.cmake)
# target_link_libraries(some_project PRIVATE Clipper2)
# if CLIPPER2_USINGZ = ON -> Clipper2Z with additional z
# var to save user data in each vertex
# target_link_libraries(some_project PRIVATE Clipper2Z)



#cmake_minimum_required(VERSION 3.15)
#project(Clipper2 VERSION 1.5.2 LANGUAGES C CXX)

if(NOT TARGET Clipper2Lib)

#    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
    if(NOT DEFINED CMAKE_CXX_STANDARD OR CMAKE_CXX_STANDARD LESS 17)
        set(CMAKE_CXX_STANDARD 17)
    endif()
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
#    set_property(GLOBAL PROPERTY USE_FOLDERS ON)

    # CLIPPER2_HI_PRECISION: See GetIntersectPoint() in clipper.core.h
    option(CLIPPER2_HI_PRECISION "Caution: enabling this will compromise performance" OFF)

    # CLIPPER2_MAX_DECIMAL_PRECISION: maximum decimal precision when scaling PathsD to Paths64.
    # Caution: excessive scaling will increase the likelihood of integer overflow errors.
    set(CLIPPER2_MAX_DECIMAL_PRECISION 8 CACHE STRING "Maximum decimal precision range")


    option(BUILD_SHARED_LIBS "Build shared libs" OFF)
    set(CLIPPER2_USINGZ "ON" CACHE STRING "Build Clipper2Z, either \"ON\" or \"OFF\" or \"ONLY\"")

    set(CLIPPER2_INC_FOLDER ${PROJECT_SOURCE_DIR}/Clipper2Lib/include/clipper2)


    set(CLIPPER2_INC
      ${CLIPPER2_INC_FOLDER}/clipper.h
      ${CLIPPER2_INC_FOLDER}/clipper.version.h
      ${CLIPPER2_INC_FOLDER}/clipper.core.h
      ${CLIPPER2_INC_FOLDER}/clipper.engine.h
      ${CLIPPER2_INC_FOLDER}/clipper.export.h
      ${CLIPPER2_INC_FOLDER}/clipper.minkowski.h
      ${CLIPPER2_INC_FOLDER}/clipper.offset.h
      ${CLIPPER2_INC_FOLDER}/clipper.rectclip.h
    )

    set(CLIPPER2_SRC
      Clipper2Lib/src/clipper.engine.cpp
      Clipper2Lib/src/clipper.offset.cpp
      Clipper2Lib/src/clipper.rectclip.cpp
    )

    set(CLIPPER2_LIBS "") # one or both of Clipper2/Clipper2Z

    # primary Clipper2 library
    if (NOT (CLIPPER2_USINGZ STREQUAL "ONLY"))
      list(APPEND CLIPPER2_LIBS Clipper2)
      add_library(Clipper2 ${CLIPPER2_INC} ${CLIPPER2_SRC})

      target_compile_definitions(
        Clipper2 PUBLIC
          CLIPPER2_MAX_DECIMAL_PRECISION=${CLIPPER2_MAX_DECIMAL_PRECISION}
          $<$<BOOL:${CLIPPER2_HI_PRECISION}>:CLIPPER2_HI_PRECISION>
      )

      target_include_directories(
        Clipper2 PUBLIC
          $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Clipper2Lib/include>
          $<INSTALL_INTERFACE:include>
      )

      if (MSVC)
        target_compile_options(Clipper2 PRIVATE /W4 /WX)
      else()
        target_compile_options(Clipper2 PRIVATE -Wall -Wextra -Wpedantic -Werror -Wno-c++20-compat)
        target_link_libraries(Clipper2 PUBLIC -lm)
      endif()
    endif()

    # secondary Clipper2 library with USINGZ defined (if required)
    if (NOT (CLIPPER2_USINGZ STREQUAL "OFF"))
      list(APPEND CLIPPER2_LIBS Clipper2Z)
      add_library(Clipper2Z ${CLIPPER2_INC} ${CLIPPER2_SRC})

      target_compile_definitions(
        Clipper2Z PUBLIC
          USINGZ
          CLIPPER2_MAX_DECIMAL_PRECISION=${CLIPPER2_MAX_DECIMAL_PRECISION}
          $<$<BOOL:${CLIPPER2_HI_PRECISION}>:CLIPPER2_HI_PRECISION>
      )
      target_include_directories(
        Clipper2Z PUBLIC
          $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Clipper2Lib/include>
          $<INSTALL_INTERFACE:include>
      )

      if (MSVC)
        target_compile_options(Clipper2Z PRIVATE /W4 /WX)
      else()
        target_compile_options(Clipper2Z PRIVATE -Wall -Wextra -Wpedantic -Werror -Wno-c++20-compat -Wsign-conversion)
        target_link_libraries(Clipper2Z PUBLIC -lm)
      endif()
    endif()

    set_target_properties(${CLIPPER2_LIBS} PROPERTIES FOLDER Libraries
                                           VERSION 1.5.2
                                           SOVERSION 1
                                           PUBLIC_HEADER "${CLIPPER2_INC}")

#    target_include_directories(Clipper2Lib
#        PUBLIC
#        ${CMAKE_CURRENT_LIST_DIR}
#        ${CMAKE_CURRENT_LIST_DIR}/.. # add parent folder to use: #include "Clipper2/..."
#        )

endif()






# disable exceptions
#string(REGEX REPLACE "/W[3|4]" "/w" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
#add_definitions(-D_HAS_EXCEPTIONS=0) # for STL
