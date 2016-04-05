if(NOT DUNE_USING_OPENCV)
file(GLOB DUNE_CVBLOB_FILES
  vendor/libraries/cvblob/*.cpp)
  
set(CVBLOB_CXX_FLAGS "-Wreturn-type -Wunused-but-set-variable -Wsign-compare -Wtype-limits ")

set_source_files_properties(${DUNE_CVBLOB_FILES}
  PROPERTIES COMPILE_FLAGS "${DUNE_CXX_FLAGS} ${CVBLOB_CXX_FLAGS}")

list(APPEND DUNE_VENDOR_FILES ${DUNE_CVBLOB_FILES})
endif(NOT DUNE_USING_OPENCV)
