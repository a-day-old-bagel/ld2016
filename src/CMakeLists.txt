add_executable(pyramid
    main.cpp
    )
target_link_libraries(pyramid
    common
    ecs
    ${ASSIMP_LIBRARIES}
    )

if(DEFINED ENV{EMSCRIPTEN} AND EMSCRIPTEN_ENABLED)
  set(EMSCRIPTEN_FLAGS
      "-s USE_SDL=2"
      "-s USE_BULLET=1"
     )
  string (REPLACE ";" " " EMSCRIPTEN_FLAGS "${EMSCRIPTEN_FLAGS}")
  set(EMSCRIPTEN_LINK_FLAGS
      "--preload-file ${CMAKE_CURRENT_SOURCE_DIR}/assets@/assets"
      "--preload-file ${CMAKE_CURRENT_SOURCE_DIR}/../../common/assets/shaders@/assets/shaders"
     )
  string (REPLACE ";" " " EMSCRIPTEN_LINK_FLAGS "${EMSCRIPTEN_LINK_FLAGS}")
  set_target_properties(pyramid PROPERTIES
      SUFFIX ".html"
      COMPILE_FLAGS "${EMSCRIPTEN_FLAGS}"
      LINK_FLAGS "${EMSCRIPTEN_FLAGS} ${EMSCRIPTEN_LINK_FLAGS}"
      )
  install(TARGETS pyramid
      RUNTIME DESTINATION html
      )
  install(FILES
      "${CMAKE_CURRENT_BINARY_DIR}/pyramid.js"
      "${CMAKE_CURRENT_BINARY_DIR}/pyramid.data"
      DESTINATION html
      )
else()
target_link_libraries(pyramid
      ${BULLET_LIBRARIES}
      ${SDL2_LIBRARY}
      ${SDL2_IMAGE_LIBRARY}
      ${GLEW_LIBRARY}
      ${OPENGL_LIBRARIES}
      ${ASSIMP_LIBRARY}
    )

#Copy assets to the build folder. 
add_custom_command(TARGET pyramid PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_CURRENT_SOURCE_DIR}/assets $<TARGET_FILE_DIR:pyramid>/assets)
endif()

add_subdirectory("./common")
add_subdirectory("./sandbox")
