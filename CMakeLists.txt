cmake_minimum_required(VERSION 3.10.2)
project(IOOpenSlide)
set(IOOpenSlide_LIBRARIES IOOpenSlide)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
find_package( OpenSlide REQUIRED )
if(NOT OPENSLIDE_FOUND)
  message(FATAL_ERROR "Please specify OPENSLIDE_LIBRARY and OPENSLIDE_INCLUDE_DIR.")
endif()

if(NOT ITK_SOURCE_DIR)
  find_package(ITK REQUIRED)
  list(APPEND CMAKE_MODULE_PATH ${ITK_CMAKE_DIR})
  include(ITKModuleExternal)
else()
  itk_module_impl()
endif()

itk_module_examples()
