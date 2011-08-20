itk_module_test()
set(ITKIOOpenSlideTests
itkOpenSlideImageIOTest.cxx
)

CreateTestDriver(ITKIOOpenSlide  "${ITKIOOpenSlide-Test_LIBRARIES}" "${ITKIOOpenSlideTests}")

itk_add_test(NAME itkOpenSlideAperioLZWSmall
      COMMAND ITKIOOpenSlideTestDriver
      --compare DATA{Input/50232_005p.tif} ${ITK_TEST_OUTPUT_DIR}/itkOpenSlideAperioLZWSmall.mha
      itkOpenSlideImageIOTest DATA{Input/50232_005p.tif} itkOpenSlideAperioLZWSmall.mha )

itk_add_test(NAME itkOpenSlideAperioLZWLarge
      COMMAND ITKIOOpenSlideTestDriver
      --compare DATA{Input/50232_100p.svs} ${ITK_TEST_OUTPUT_DIR}/itkOpenSlideAperioLZWLarge.mha
      itkOpenSlideImageIOTest DATA{Input/50232_100p.svs} itkOpenSlideAperioLZWLarge.mha )

itk_add_test(NAME itkOpenSlideAperioLZWMultidataset
      COMMAND ITKIOOpenSlideTestDriver
      --compare DATA{Input/50232_multidataset_025p.tif} ${ITK_TEST_OUTPUT_DIR}/itkOpenSlideAperioLZWMultidataset.mha
      itkOpenSlideImageIOTest DATA{Input/50232_multidataset_025p.tif} itkOpenSlideAperioLZWMultidataset.mha )

itk_add_test(NAME itkOpenSlideAperioJPEG2000
      COMMAND ITKIOOpenSlideTestDriver
      --compare DATA{Input/15711_tiled.tif} ${ITK_TEST_OUTPUT_DIR}/itkOpenSlideAperioJPEG2000.mha
      itkOpenSlideImageIOTest DATA{Input/15711_tiled.tif} itkOpenSlideAperioJPEG2000.mha )
