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

#include "itkIOCommon.h"
#include "itkOpenSlideImageIO.h"

namespace itk
{

OpenSlideImageIO::OpenSlideImageIO()
{
  this->SetNumberOfDimensions(2); // OpenSlide is 2D.

  m_Spacing[0] = 1.0;
  m_Spacing[1] = 1.0;

  m_Origin[0] = 0.0;
  m_Origin[1] = 0.0;

  // Trestle, Aperio, and generic tiled tiff
  this->AddSupportedReadExtension(".tif");
  // Hamamatsu
  this->AddSupportedReadExtension(".vms");
  this->AddSupportedReadExtension(".vmu");
  // Aperio
  this->AddSupportedReadExtension(".svs");
  // MIRAX
  this->AddSupportedReadExtension(".svs");
}

OpenSlideImageIO::~OpenSlideImageIO()
{
}

void OpenSlideImageIO::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);
}

bool OpenSlideImageIO::CanReadFile( const char* filename )
{
  std::string fname( filename );

  bool supportedExtension = false;
  for( ArrayOfExtensionsType::const_iterator extIt = this->GetSupportedReadExtensions().begin();
       extIt != this->GetSupportedReadExtensions().end();
       ++extIt )
    {
    if( fname.rfind( *extIt ) != std::string::npos )
      {
      supportedExtension = true;
      }
    }
  if( !supportedExtension )
    {
    return false;
    }

  return true;
}


void OpenSlideImageIO::ReadImageInformation()
{
}


void OpenSlideImageIO::Read( void * buffer)
{
}


bool OpenSlideImageIO::CanWriteFile( const char * name )
{
  return false;
}


void
OpenSlideImageIO
::WriteImageInformation(void)
{
  // add writing here
}


/**
 *
 */
void
OpenSlideImageIO
::Write( const void* buffer)
{
}

/** Given a requested region, determine what could be the region that we can
 * read from the file. This is called the streamable region, which will be
 * smaller than the LargestPossibleRegion and greater or equal to the
RequestedRegion */
ImageIORegion
OpenSlideImageIO
::GenerateStreamableReadRegionFromRequestedRegion( const ImageIORegion & requested ) const
{
  std::cout << "OpenSlideImageIO::GenerateStreamableReadRegionFromRequestedRegion()" << std::endl;
  std::cout << "Requested region = " << requested << std::endl;
  //
  // OpenSlide is the ultimate streamer.
  //
  ImageIORegion streamableRegion = requested;

  std::cout << "StreamableRegion = " << streamableRegion << std::endl;

  return streamableRegion;
}


} // end namespace itk
