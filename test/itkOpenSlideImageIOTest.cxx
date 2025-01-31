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

#include <cstdlib>
#include <cstring>

#include "itkOpenSlideImageIO.h"
#include "itkOpenSlideImageIOFactory.h"

#include "itkImageIOFactory.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImage.h"
#include "itkRGBAPixel.h"
#include "itkOpenSlideImageIO.h"

#define SPECIFIC_IMAGEIO_MODULE_TEST

namespace
{

bool
ParseValue(const char * p_cValue, std::string & strCommand, std::string & strValue)
{
  strCommand.clear();
  strValue.clear();

  if (p_cValue == NULL)
    return false;

  const char * p = strchr(p_cValue, '=');

  if (p == NULL)
  {
    strCommand = p_cValue;
    return true;
  }

  // is p the beginning or end of the string?
  if (p == p_cValue)
    return false;

  strCommand.assign(p_cValue, (p - p_cValue));
  strValue = p + 1;

  return true;
}

#if 0
// For generating data for tests (particularly the streaming one)
bool CompressImageFile(const char *p_cFileName) {
  using PixelType = itk::RGBAPixel<unsigned char>;
  using ImageType = itk::Image<PixelType, 2>;

  using ReaderType = itk::ImageFileReader<ImageType>;
  using WriterType = itk::ImageFileWriter<ImageType>;

  ReaderType::Pointer p_clReader = ReaderType::New();
  WriterType::Pointer p_clWriter = WriterType::New();

  p_clReader->SetFileName(p_cFileName);
  p_clWriter->SetInput(p_clReader->GetOutput());
  p_clWriter->SetFileName(p_cFileName);
  p_clWriter->UseCompressionOn();

  try {
    p_clWriter->Update();
  }
  catch (itk::ExceptionObject &e) {
    std::cerr << "Error: " << e << std::endl;
    return false;
  }

  return true;
}
#endif

} // End anonymous namespace

int
itkOpenSlideImageIOTest(int argc, char * argv[])
{
  using PixelType = itk::RGBAPixel<unsigned char>;
  using ImageType = itk::Image<PixelType, 2>;
  using ReaderIOType = itk::OpenSlideImageIO;
  using ReaderType = itk::ImageFileReader<ImageType>;
  using WriterType = itk::ImageFileWriter<ImageType>;

  if (argc < 3)
  {
    std::cerr << "Usage: " << argv[0] << " inputImage outputImage [command1 command2 ...]\n";
    return EXIT_FAILURE;
  }

  // const char * const p_cArg0 = argv[0];
  const char * const p_cInputImage = argv[1];
  const char * const p_cOutputImage = argv[2];

  argc -= 3;
  argv += 3;

  bool         bShouldFail = false;
  bool         bUseCompression = false;
  bool         bApproximateStreaming = false;
  unsigned int uiNumStreams = 0; // 0 means no streaming
  int          iLevel = 0;
  std::string  strAssociatedImageName;
  double       dDownsampleFactor = 0.0; // 0 means no down sample

  for (int i = 0; i < argc; ++i)
  {
    std::string strCommand;
    std::string strValue;

    if (!ParseValue(argv[i], strCommand, strValue))
    {
      std::cerr << "Error: Could not parse value '" << argv[i] << "'." << std::endl;
      return EXIT_FAILURE;
    }

    if (strCommand == "shouldFail")
    {
      bShouldFail = true;
    }
    else if (strCommand == "compress")
    {
      bUseCompression = true;
    }
    else if (strCommand == "approximateStreaming")
    {
      bApproximateStreaming = true;
    }
    else if (strCommand == "level")
    {
      if (strValue.empty())
      {
        std::cerr << "Error: Expected level parameter." << std::endl;
        return EXIT_FAILURE;
      }

      char * p = NULL;
      iLevel = strtol(strValue.c_str(), &p, 10);
      if (*p != '\0')
      {
        std::cerr << "Error: Could not parse level '" << strValue << "'." << std::endl;
        return EXIT_FAILURE;
      }
    }
    else if (strCommand == "associatedImage")
    {
      if (strValue.empty())
      {
        std::cerr << "Error: Expected associated image name." << std::endl;
        return EXIT_FAILURE;
      }

      strAssociatedImageName = strValue;
    }
    else if (strCommand == "downsample")
    {
      if (strValue.empty())
      {
        std::cerr << "Error: Expected downsample factor." << std::endl;
        return EXIT_FAILURE;
      }

      char * p = NULL;
      dDownsampleFactor = strtod(strValue.c_str(), &p);
      if (*p != '\0')
      {
        std::cerr << "Error: Could not parse downsample factor '" << strValue << "'." << std::endl;
        return EXIT_FAILURE;
      }
    }
    else if (strCommand == "stream")
    {
      if (strValue.empty())
      {
        std::cerr << "Error: Expected number of streams." << std::endl;
        return EXIT_FAILURE;
      }

      char * p = NULL;
      uiNumStreams = strtoul(strValue.c_str(), &p, 10);
      if (*p != '\0')
      {
        std::cerr << "Error: Could not parse number of streams '" << strValue << "'." << std::endl;
        return EXIT_FAILURE;
      }
    }
    else
    {
      std::cout << "Error: Unknown command '" << argv[i] << "'." << std::endl;
      return EXIT_FAILURE;
    }
  }

  const int iFailCode = (bShouldFail ? EXIT_SUCCESS : EXIT_FAILURE);
  const int iSuccessCode = (iFailCode == EXIT_FAILURE ? EXIT_SUCCESS : EXIT_FAILURE);

  std::cout << "Parameters:\n" << std::endl;
  std::cout << "inputImage = '" << p_cInputImage << '\'' << std::endl;
  std::cout << "outputImage = '" << p_cOutputImage << '\'' << std::endl;
  std::cout << "shouldFail = " << std::boolalpha << bShouldFail << std::endl;
  std::cout << "compress = " << std::boolalpha << bUseCompression << std::endl;
  std::cout << "approximateStreaming = " << std::boolalpha << bApproximateStreaming << std::endl;
  std::cout << "stream = " << uiNumStreams << std::endl;
  std::cout << "level = " << iLevel << std::endl;
  std::cout << "associatedImage = '" << strAssociatedImageName << '\'' << std::endl;
  std::cout << "downsample = " << dDownsampleFactor << std::endl;

  ReaderIOType::Pointer p_clImageIO = ReaderIOType::New();
  ReaderType::Pointer   p_clReader = ReaderType::New();
  WriterType::Pointer   p_clWriter = WriterType::New();

  p_clImageIO->SetFileName(p_cInputImage);

  p_clReader->SetImageIO(p_clImageIO);
  p_clReader->SetFileName(p_cInputImage);

  p_clWriter->SetInput(p_clReader->GetOutput());
  p_clWriter->SetFileName(p_cOutputImage);

  try
  {
    p_clImageIO->ReadImageInformation();
  }
  catch (itk::ExceptionObject & e)
  {
    std::cerr << "Error: " << e << std::endl;
    return iFailCode;
  }

  if (strAssociatedImageName.size() > 0)
    p_clImageIO->SetAssociatedImageName(strAssociatedImageName);
  else
    p_clImageIO->SetLevel(iLevel);

  if (dDownsampleFactor > 0.0 && !p_clImageIO->SetLevelForDownsampleFactor(dDownsampleFactor))
    return iFailCode;

  if (uiNumStreams > 0)
  {
    if (!p_clImageIO->CanStreamRead())
      return iFailCode;

    p_clImageIO->UseStreamedReadingOn();
    p_clImageIO->SetApproximateStreaming(bApproximateStreaming);

    itk::ImageIOBase::Pointer p_clWriterIO =
      itk::ImageIOFactory::CreateImageIO(p_cOutputImage, itk::IOFileModeEnum::WriteMode);
    if (!p_clWriterIO)
    {
      std::cerr << "Error: Could not create ImageIO for output image '" << p_cOutputImage << "'." << std::endl;
      return iFailCode;
    }

    if (bUseCompression)
      std::cout << "Warning: Compression may disable streaming." << std::endl;

    p_clWriterIO->UseStreamedWritingOn();

    p_clWriter->SetImageIO(p_clWriterIO);
    p_clWriter->SetNumberOfStreamDivisions(uiNumStreams);
  }

  // XXX: Just so you know, this might disable streaming
  p_clWriter->SetUseCompression(bUseCompression);

  try
  {
    p_clWriter->Update();
  }
  catch (itk::ExceptionObject & e)
  {
    std::cerr << "Error: " << e << std::endl;
    return iFailCode;
  }

  // Use this to compress output images when updating tests
  // CompressImageFile(p_cOutputImage);

  return iSuccessCode;
}
