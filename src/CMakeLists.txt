set(IOOpenSlide_SRCS
  itkOpenSlideImageIOFactory.cxx
  itkOpenSlideImageIO.cxx
  )

itk_module_add_library(IOOpenSlide ${IOOpenSlide_SRCS})
target_include_directories(IOOpenSlide PRIVATE ${OPENSLIDE_INCLUDE_DIRS})
target_link_libraries(IOOpenSlide LINK_PRIVATE ${OPENSLIDE_LIBRARIES})
