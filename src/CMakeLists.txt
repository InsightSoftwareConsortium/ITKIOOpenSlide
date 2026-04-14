set(IOOpenSlide_SRCS
  itkOpenSlideImageIOFactory.cxx
  itkOpenSlideImageIO.cxx
  )

include_directories("${IOOpenSlide_SOURCE_DIR}/include")
include_directories("${CMAKE_CURRENT_BINARY_DIR}")
include_directories(${OPENSLIDE_INCLUDE_DIRS})

add_library(IOOpenSlide ${IOOpenSlide_SRCS})
target_link_libraries(IOOpenSlide LINK_PRIVATE ${OPENSLIDE_LIBRARIES})
itk_module_link_dependencies()
itk_module_target(IOOpenSlide)
