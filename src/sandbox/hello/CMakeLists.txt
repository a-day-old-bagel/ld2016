add_executable(hello
    main.cpp
    )
target_link_libraries(hello
    common
    ${ASSIMP_LIBRARIES}
    )

if(DEFINED ENV{EMSCRIPTEN})
  set(EMSCRIPTEN_FLAGS
      "-s USE_SDL=2"
      "-s USE_BULLET=1"
     )
  string (REPLACE ";" " " EMSCRIPTEN_FLAGS "${EMSCRIPTEN_FLAGS}")
  set_target_properties(hello PROPERTIES
      SUFFIX ".html"
      COMPILE_FLAGS "${EMSCRIPTEN_FLAGS}"
      LINK_FLAGS "${EMSCRIPTEN_FLAGS}"
      )
  install(TARGETS hello
      RUNTIME DESTINATION html
      )
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/hello.js"
      DESTINATION html
      )
else()
target_link_libraries(hello
      ${BULLET_LIBRARIES}
      ${SDL2_LIBRARY}
      ${GLEW_LIBRARY}
      ${OPENGL_LIBRARIES}
    )
endif()
