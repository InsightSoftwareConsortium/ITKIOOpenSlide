project(ITKIOOpenSlide)
set(ITKIOOpenSlide_LIBRARIES ITKIOOpenSlide)

set( CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH} )
find_package( OpenSlide REQUIRED )

itk_module_impl()
