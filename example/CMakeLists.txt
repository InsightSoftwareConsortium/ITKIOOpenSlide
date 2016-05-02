cmake_minimum_required(VERSION 2.8)

project(DumpSlideInformation CXX)

find_package(ITK REQUIRED
  COMPONENTS
    ITKCommon
    ITKIOImageBase
    ITKIOTIFF
    ITKIOOpenSlide
    ITKIOJPEG
    ITKIOPNG
  )
include(${ITK_USE_FILE})

add_definitions(-D_USE_MATH_DEFINES=1)

add_executable(DumpSlideInformation DumpSlideInformation.cpp)
target_link_libraries(DumpSlideInformation ${ITK_LIBRARIES})
