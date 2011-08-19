/*=========================================================================
 *
 *  Copyright Insight Software Consortium
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

#include "itkOpenSlideImageIO.h"
#include "itkOpenSlideImageIOFactory.h"

#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

#define SPECIFIC_IMAGEIO_MODULE_TEST

int itkOpenSlideImageIOTest( int argc, char * argv[] )
{
  if( argc < 3 )
    {
    std::cerr << "Usage: " << argv[0] << " inputImage outputImage\n";
    return EXIT_FAILURE;
    }

  // There are other IO's that can read .tif's.
  itk::ObjectFactoryBase::UnRegisterAllFactories();
  itk::OpenSlideImageIOFactory::RegisterOneFactory();

  const unsigned int Dimension = 2;
  typedef itk::RGBPixel<unsigned char> PixelType;
  typedef itk::Image<PixelType, 2>     ImageType;

  typedef itk::ImageFileReader<ImageType> ReaderType;
  ReaderType::Pointer reader = ReaderType::New();

  reader->SetFileName( argv[1] );

  return EXIT_SUCCESS;
}
