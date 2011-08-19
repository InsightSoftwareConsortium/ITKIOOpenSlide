project(ITKIOOpenSlide)
set(ITKIOOpenSlide_LIBRARIES ITKIOOpenSlide)

option( USE_OPENSLIDE "Build openslide.org file reader.  Requires the library to
be installed." OFF )
mark_as_advanced( USE_OPENSLIDE )

if( USE_OPENSLIDE )
  find_package( OpenSlide REQUIRED )
  itk_module_impl()
endif()
