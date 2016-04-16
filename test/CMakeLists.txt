itk_module_test()
set(ITKIOOpenSlideTests
itkOpenSlideImageIOTest.cxx
itkOpenSlideTestMetaData.cxx
)

CreateTestDriver(ITKIOOpenSlide  "${ITKIOOpenSlide-Test_LIBRARIES}" "${ITKIOOpenSlideTests}")

itk_add_test(NAME itkOpenSlideTestMetaData
  COMMAND ITKIOOpenSlideTestDriver
  itkOpenSlideTestMetaData DATA{Input/CMU-1.svs} ${ITK_TEST_OUTPUT_DIR}/MetaDataTest.txt DATA{Input/CMU-1.svs.txt}
)

itk_add_test(NAME itkOpenSlideTestBasicIO
  COMMAND ITKIOOpenSlideTestDriver
  --compare DATA{Input/CMU-1-Small-Region.mha} ${ITK_TEST_OUTPUT_DIR}/CMU-1-Small-Region.mha
  itkOpenSlideImageIOTest DATA{Input/CMU-1-Small-Region.svs} ${ITK_TEST_OUTPUT_DIR}/CMU-1-Small-Region.mha compress
)

itk_add_test(NAME itkOpenSlideTestAssociatedImage
  COMMAND ITKIOOpenSlideTestDriver
  --compare DATA{Input/CMU-1-Small-Region-label.mha} ${ITK_TEST_OUTPUT_DIR}/CMU-1-Small-Region-label.mha
  itkOpenSlideImageIOTest DATA{Input/CMU-1-Small-Region.svs} ${ITK_TEST_OUTPUT_DIR}/CMU-1-Small-Region-label.mha associatedImage=label compress
)

itk_add_test(NAME itkOpenSlideTestLevel
  COMMAND ITKIOOpenSlideTestDriver
  --compare DATA{Input/CMU-3-level-7.mha} ${ITK_TEST_OUTPUT_DIR}/CMU-3-level-7.mha
  itkOpenSlideImageIOTest DATA{Input/CMU-3.ndpi} ${ITK_TEST_OUTPUT_DIR}/CMU-3-level-7.mha level=7 compress
)

itk_add_test(NAME itkOpenSlideTestDownsample
  COMMAND ITKIOOpenSlideTestDriver
  --compare DATA{Input/CMU-3-level-7.mha} ${ITK_TEST_OUTPUT_DIR}/CMU-3-downsample-129.mha
  itkOpenSlideImageIOTest DATA{Input/CMU-3.ndpi} ${ITK_TEST_OUTPUT_DIR}/CMU-3-downsample-129.mha downsample=129 compress
)

# Compress cannot be used here since stream writing will not be supported
itk_add_test(NAME itkOpenSlideTestApproximateStreaming
  COMMAND ITKIOOpenSlideTestDriver
  --compare DATA{Input/CMU-1-level-1-stream-200.mha} ${ITK_TEST_OUTPUT_DIR}/CMU-1-level-1-stream-200.mha
  itkOpenSlideImageIOTest DATA{Input/CMU-1.svs} ${ITK_TEST_OUTPUT_DIR}/CMU-1-level-1-stream-200.mha level=1 stream=200 approximateStreaming
)

itk_add_test(NAME itkOpenSlideTestStreaming
  COMMAND ITKIOOpenSlideTestDriver
  --compare DATA{Input/CMU-1-level-1.mha} ${ITK_TEST_OUTPUT_DIR}/CMU-1-level-1.mha
  itkOpenSlideImageIOTest DATA{Input/CMU-1.svs} ${ITK_TEST_OUTPUT_DIR}/CMU-1-level-1.mha level=1 stream=200
)

