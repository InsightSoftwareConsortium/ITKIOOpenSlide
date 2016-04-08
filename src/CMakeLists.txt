set(ITKIOOpenSlide_SRCS
  itkOpenSlideImageIOFactory.cxx
  itkOpenSlideImageIO.cxx
  )

include_directories(${OPENSLIDE_INCLUDE_DIRS})

add_library(ITKIOOpenSlide ${ITKIOOpenSlide_SRCS})
target_link_libraries(ITKIOOpenSlide LINK_PRIVATE ${OPENSLIDE_LIBRARIES})
itk_module_link_dependencies()
itk_module_target(ITKIOOpenSlide)
