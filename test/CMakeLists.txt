itk_module_test()
set(ITKIOOpenSlideTests
itkOpenSlideImageIOTest.cxx
)

CreateTestDriver(ITKIOOpenSlide  "${ITKIOOpenSlide-Test_LIBRARIES}" "${ITKIOOpenSlideTests}")

if( ITK_USE_BIG_DATA )
  itk_add_test(NAME itkOpenSlideImageIOTest
        COMMAND ITKIOOpenSlideTestDriver
        itkOpenSlideImageIOTest DATA{Input/aperio_histology_sample_image.svs})
endif()
