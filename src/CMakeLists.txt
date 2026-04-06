set(IOOpenSlide_SRCS
  itkOpenSlideImageIOFactory.cxx
  itkOpenSlideImageIO.cxx
  )

add_library(IOOpenSlide ${IOOpenSlide_SRCS})
target_include_directories(IOOpenSlide PRIVATE ${OPENSLIDE_INCLUDE_DIRS})
target_include_directories(IOOpenSlide PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../include)
target_include_directories(IOOpenSlide PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/include)
target_link_libraries(IOOpenSlide PRIVATE ${OPENSLIDE_LIBRARIES})
itk_module_link_dependencies()
itk_module_target(IOOpenSlide)
