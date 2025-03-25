itk_python_expression_add_test(
  NAME itkOpenSlideImageIOPythonInstantiationTest
  EXPRESSION "io = itk.OpenSlideImageIO.New()"
)

set(test_input_dir ${CMAKE_CURRENT_SOURCE_DIR}/../../test/Input)
# cmake_path(CONVERT ${test_input_dir} TO_CMAKE_PATH_LIST test_input_dir NORMALIZE)  # CMake 3.20+

list(FIND ITK_WRAP_IMAGE_DIMS 2 wrap_2_index)
if(wrap_2_index GREATER -1)
  itk_python_add_test(NAME itkOpenSlideTestBasicIOPython
    TEST_DRIVER_ARGS
    --compare DATA{${test_input_dir}/CMU-1-Small-Region.mha} ${ITK_TEST_OUTPUT_DIR}/CMU-1-Small-Region-Python.mha
    COMMAND convert_test.py
      DATA{${test_input_dir}/CMU-1-Small-Region.svs}
      ${ITK_TEST_OUTPUT_DIR}/CMU-1-Small-Region-Python.mha
    )
endif()
