set(ITKIOOpenSlide_SRC
itkOpenSlideImageIOFactory.cxx
itkOpenSlideImageIO.cxx
)

include_directories( ${OPENSLIDE_INCLUDE_DIRS} )

add_library(ITKIOOpenSlide ${ITKIOOpenSlide_SRC})
target_link_libraries( ITKIOOpenSlide ${ITKIOBase_LIBRARIES}
  ${OPENSLIDE_LIBRARIES} )
itk_module_target(ITKIOOpenSlide)
