/*=========================================================================
 *
 *  Copyright NumFOCUS
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         https://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

#include <cstring>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <vector>
#include "itkOpenSlideImageIO.h"
#include "itkOpenSlideImageIOFactory.h"
#include "itkImage.h"
#include "itkMetaDataObject.h"

#define SPECIFIC_IMAGEIO_MODULE_TEST

namespace
{

class ReplaceStream
{
public:
  ReplaceStream(std::ios & stream, const std::ios & newStream)
    : m_Stream(stream)
    , m_OriginalBuf(stream.rdbuf(newStream.rdbuf()))
  {}

  ~ReplaceStream() { m_Stream.rdbuf(m_OriginalBuf); }

private:
  std::ios &             m_Stream;
  std::streambuf * const m_OriginalBuf;
};

bool
ReadFileStripCR(const char * p_cFileName, std::vector<char> & vBuffer)
{
  vBuffer.clear();

  std::ifstream fileStream(p_cFileName);
  if (!fileStream)
    return false;

  fileStream.seekg(0, std::ifstream::end);
  const size_t length = fileStream.tellg();
  fileStream.seekg(0, std::ifstream::beg);

  vBuffer.resize(length);

  if (!fileStream.read(&vBuffer[0], vBuffer.size()))
    return false;

  size_t j = 0;
  for (size_t i = 0; i < vBuffer.size(); ++i)
  {
    if (vBuffer[i] != '\r')
    {
      vBuffer[j] = vBuffer[i];
      ++j;
    }
  }

  vBuffer.resize(j);

  return true;
}

} // End anonymous namespace

int
itkOpenSlideTestMetaData(int argc, char * argv[])
{
  using ImageIOType = itk::OpenSlideImageIO;
  using PixelType = itk::RGBAPixel<unsigned char>;
  using ImageType = itk::Image<PixelType, 2>;
  using SizeType = ImageType::SizeType;
  using SpacingType = ImageType::SpacingType;

  if (argc < 2 || argc > 4)
  {
    std::cerr << "Usage: " << argv[0] << " slideFile [outputLog] [comparisonLog]" << std::endl;
    return EXIT_FAILURE;
  }

  const char * const p_cSlideFile = argv[1];
  const char * const p_cOutputLog = argc > 2 ? argv[2] : "stdout";
  const char * const p_cCompareLog = argc > 3 ? argv[3] : NULL;

  std::ofstream  logFileStream;
  std::ostream * p_outStream = &std::cout;

  if (strcmp(p_cOutputLog, "stdout") != 0)
  {
    logFileStream.open(p_cOutputLog, std::ofstream::out | std::ofstream::trunc);
    if (!logFileStream)
    {
      std::cerr << "Error: Could not open output log '" << p_cOutputLog << "'." << std::endl;
      return EXIT_FAILURE;
    }

    p_outStream = &logFileStream;
  }

  // RAII trick
  ReplaceStream clStreamGuard(std::cout, *p_outStream);

  ImageIOType::Pointer p_clImageIO = ImageIOType::New();

  p_clImageIO->SetFileName(p_cSlideFile);

  try
  {
    p_clImageIO->ReadImageInformation();
  }
  catch (itk::ExceptionObject & e)
  {
    std::cerr << "Error: " << e << std::endl;
    return EXIT_FAILURE;
  }

  const std::string strComponentType = itk::ImageIOBase::GetComponentTypeAsString(p_clImageIO->GetComponentType());
  const std::string strPixelType = itk::ImageIOBase::GetPixelTypeAsString(p_clImageIO->GetPixelType());

  std::cout << "\nImage Information:\n" << std::endl;
  std::cout << "Dimensions: " << p_clImageIO->GetNumberOfDimensions() << std::endl;
  std::cout << "Component type: " << strComponentType << std::endl;
  std::cout << "Pixel type: " << strPixelType << std::endl;
  std::cout << "Vendor: " << p_clImageIO->GetVendor() << std::endl;

  // Some sanity checks

  // Check dimensions
  if (p_clImageIO->GetNumberOfDimensions() != ImageType::GetImageDimension())
  {
    std::cerr << "Error: ImageIO should report dimension " << ImageType::GetImageDimension() << " but reports "
              << p_clImageIO->GetNumberOfDimensions() << '.' << std::endl;
    return EXIT_FAILURE;
  }

  // Check pixel type
  {
    // Let's use an ImageIOType to form the expected pixel information
    ImageIOType::Pointer p_clTmpIO = ImageIOType::New();

    PixelType clTmpPixel;
    p_clTmpIO->SetPixelTypeInfo(&clTmpPixel);

    const std::string strExpectedComponentType =
      itk::ImageIOBase::GetComponentTypeAsString(p_clTmpIO->GetComponentType());
    const std::string strExpectedPixelType = itk::ImageIOBase::GetPixelTypeAsString(p_clTmpIO->GetPixelType());

    if (strExpectedComponentType != strComponentType)
    {
      std::cerr << "Error: ImageIO should report a component type of " << strExpectedComponentType << " but reports "
                << strComponentType << '.' << std::endl;
      return EXIT_FAILURE;
    }

    if (strExpectedPixelType != strPixelType)
    {
      std::cerr << "Error: ImageIO should report a pixel type of " << strExpectedPixelType << " but reports "
                << strPixelType << '.' << std::endl;
      return EXIT_FAILURE;
    }
  }

  // Sanity checks passed

  std::cout << "\nMeta Data:\n" << std::endl;

  itk::MetaDataDictionary & clTags = p_clImageIO->GetMetaDataDictionary();
  std::vector<std::string>  vKeys = clTags.GetKeys();

  std::cout << "Number of keys: " << vKeys.size() << std::endl;
  std::cout << "Entries:" << std::endl;
  for (size_t i = 0; i < vKeys.size(); ++i)
  {
    const std::string & strKey = vKeys[i];
    std::string         strValue;

    if (itk::ExposeMetaData(clTags, strKey, strValue))
      std::cout << strKey << " = " << strValue << std::endl;
  }

  std::cout << "\nLevel Information:\n" << std::endl;

  const int iLevelCount = p_clImageIO->GetLevelCount();
  std::cout << "Level count: " << iLevelCount << std::endl;

  std::cout << "Levels:" << std::endl;
  for (int iLevel = 0; iLevel < iLevelCount; ++iLevel)
  {
    p_clImageIO->SetLevel(iLevel);

    try
    {
      p_clImageIO->ReadImageInformation();
    }
    catch (itk::ExceptionObject & e)
    {
      std::cerr << "Error: " << e << std::endl;
      return EXIT_FAILURE;
    }

    SizeType clSize;
    clSize[0] = p_clImageIO->GetDimensions(0);
    clSize[1] = p_clImageIO->GetDimensions(1);

    SpacingType clSpacing;
    clSpacing[0] = p_clImageIO->GetSpacing(0);
    clSpacing[1] = p_clImageIO->GetSpacing(1);

    const size_t sizeInBytes = p_clImageIO->GetImageSizeInBytes();
    const int    iCurrentLevel = p_clImageIO->GetLevel();

    std::cout << "Level " << iCurrentLevel << ": dimensions = " << clSize << ", spacing = " << clSpacing
              << ", size in bytes = " << sizeInBytes << std::endl;
  }

  std::cout << "\nAssociated image information:\n" << std::endl;

  ImageIOType::AssociatedImageNameContainer vAssociatedImages = p_clImageIO->GetAssociatedImageNames();

  std::cout << "Number of associated images: " << vAssociatedImages.size() << std::endl;
  std::cout << "Associated image names:" << std::endl;

  const size_t numWordsPerLine = 3;

  for (size_t i = 0; i < vAssociatedImages.size(); i += numWordsPerLine)
  {
    const size_t jBegin = i;
    const size_t jEnd = std::min(vAssociatedImages.size(), jBegin + numWordsPerLine);

    std::cout << '\'' << vAssociatedImages[jBegin] << '\'';

    for (size_t j = jBegin + 1; j < jEnd; ++j)
      std::cout << ", '" << vAssociatedImages[j] << '\'';

    if (i + numWordsPerLine < vAssociatedImages.size())
      std::cout << ',';

    std::cout << std::endl;
  }

  std::cout << "\nAssociated images:" << std::endl;

  for (size_t i = 0; i < vAssociatedImages.size(); ++i)
  {
    const std::string & strAssociatedImage = vAssociatedImages[i];

    p_clImageIO->SetAssociatedImageName(strAssociatedImage);

    try
    {
      p_clImageIO->ReadImageInformation();
    }
    catch (itk::ExceptionObject & e)
    {
      std::cerr << "Error: " << e << std::endl;
      return EXIT_FAILURE;
    }

    SizeType clSize;
    clSize[0] = p_clImageIO->GetDimensions(0);
    clSize[1] = p_clImageIO->GetDimensions(1);

    SpacingType clSpacing;
    clSpacing[0] = p_clImageIO->GetSpacing(0);
    clSpacing[1] = p_clImageIO->GetSpacing(1);

    const size_t      sizeInBytes = p_clImageIO->GetImageSizeInBytes();
    const std::string strCurrentAssociatedImage = p_clImageIO->GetAssociatedImageName();

    std::cout << strCurrentAssociatedImage << ": dimensions = " << clSize << ", spacing = " << clSpacing
              << ", size in bytes = " << sizeInBytes << std::endl;
  }

  if (p_cCompareLog != NULL)
  {
    logFileStream.close();

    std::vector<char> vBuffer1, vBuffer2;

    if (!ReadFileStripCR(p_cOutputLog, vBuffer1))
    {
      std::cerr << "Error: Could not read output log file '" << p_cOutputLog << "'." << std::endl;
      return EXIT_FAILURE;
    }

    if (!ReadFileStripCR(p_cCompareLog, vBuffer2))
    {
      std::cerr << "Error: Could not read comparison log file '" << p_cCompareLog << "'." << std::endl;
      return EXIT_FAILURE;
    }

    if (vBuffer1.size() != vBuffer2.size() || std::memcmp(&vBuffer1[0], &vBuffer2[0], vBuffer1.size()) != 0)
      return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
