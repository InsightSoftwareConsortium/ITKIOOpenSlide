project(ITKIOOpenSlide)
set(ITKIOOpenSlide_LIBRARIES ITKIOOpenSlide)

set( CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH} )
find_package( OpenSlide REQUIRED )
if( NOT OPENSLIDE_FOUND )
  message( FATAL_ERROR
    "Please specify OPENSLIDE_LIBRARY and OPENSLIDE_INCLUDE_DIR.")
endif()

itk_module_impl()
