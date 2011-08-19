set(ITKIOOpenSlide_SRC
itkOpenSlideImageIOFactory.cxx
itkOpenSlideImageIO.cxx
)

add_library(ITKIOOpenSlide ${ITKIOOpenSlide_SRC})
target_link_libraries( ITKIOOpenSlide ${ITKIOBase_LIBRARIES}
  ${OPENSLIDE_LIBRARIES} )
itk_module_target(ITKIOOpenSlide)
