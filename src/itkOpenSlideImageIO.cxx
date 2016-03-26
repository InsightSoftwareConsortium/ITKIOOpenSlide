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

#include <cctype>
#include <memory>
#include <sstream>

#include "itkIOCommon.h"
#include "itkOpenSlideImageIO.h"
#include "itksys/SystemTools.hxx"
#include "itkMetaDataDictionary.h"
#include "itkMetaDataObject.h"

// OpenSlide
#include "openslide/openslide.h"

namespace itk
{

class OpenSlideWrapper {
public:
  openslide_t *p_osr;
  int32_t i32Level;
  std::string strAssociatedImage;

  static bool CanReadFile(const char *p_cFileName) {
    return openslide_detect_vendor(p_cFileName) != NULL;
  }

  OpenSlideWrapper() {
    p_osr = NULL;
    i32Level = 0;
  }

  OpenSlideWrapper(const char *p_cFileName) {
    p_osr = NULL;
    i32Level = 0;
    Open(p_cFileName);
  }

  ~OpenSlideWrapper() {
    Close();
  }

  bool CanStreamRead() const {
    return strAssociatedImage.empty();
  }

  void Close() {
    if (p_osr != NULL) {
      openslide_close(p_osr);
      p_osr = NULL;
    }
  }

  bool IsOpened() const {
    return p_osr != NULL;
  }

  bool Open(const char *p_cFileName) {
    Close();
    p_osr = openslide_open(p_cFileName);
    return p_osr != NULL;
  }

  int32_t GetBestLevelForDownsample(double dDownsample) const {
    if (p_osr == NULL)
      return -1;

    return openslide_get_best_level_for_downsample(p_osr, dDownsample);
  }

  int32_t GetLevelCount() const {
    if (p_osr == NULL)
      return -1;

    return openslide_get_level_count(p_osr);
  }

  // Returns NULL for success
  const char * ReadRegion(uint32_t *p_ui32Dest, int64_t i64X, int64_t i64Y, int64_t i64Width, int64_t i64Height) const {
    if (p_osr == NULL)
      return "OpenSlideWrapper has no file open.";

    if (strAssociatedImage.size() > 0)
      openslide_read_associated_image(p_osr, strAssociatedImage.c_str(), p_ui32Dest);
    else
      openslide_read_region(p_osr, p_ui32Dest, i64X, i64Y, i32Level, i64Width, i64Height);

    return openslide_get_error(p_osr);
  }

  bool GetSpacing(double &dSpacingX, double &dSpacingY) const {
    if (p_osr == NULL)
      return false;

    if (strAssociatedImage.size() > 0)
      return false;

    const double dDownsample = openslide_get_level_downsample(p_osr, i32Level);

    if (dDownsample <= 0.0)
      return false;

    if (!GetPropertyValue(OPENSLIDE_PROPERTY_NAME_MPP_X, dSpacingX) || !GetPropertyValue(OPENSLIDE_PROPERTY_NAME_MPP_Y, dSpacingY))
      return false;

    dSpacingX *= dDownsample;
    dSpacingY *= dDownsample;

    return true;
  }

  bool GetOrigin(double &dOriginX, double &dOriginY) const {
    if (p_osr == NULL)
      return false;

    if (strAssociatedImage.size() > 0)
      return false;

    return GetPropertyValue(OPENSLIDE_PROPERTY_NAME_BOUNDS_X, dOriginX) && GetPropertyValue(OPENSLIDE_PROPERTY_NAME_BOUNDS_Y, dOriginY);
  }

  bool GetDimensions(int64_t &i64Width, int64_t &i64Height) const {
    if (p_osr == NULL)
      return false;

    if (strAssociatedImage.size() > 0)
      openslide_get_associated_image_dimensions(p_osr, strAssociatedImage.c_str(), &i64Width, &i64Height);
    else
      openslide_get_level_dimensions(p_osr, i32Level, &i64Width, &i64Height);

    return i64Width > 0 && i64Height > 0;
  }

  std::vector<std::string> GetAssociatedImageNames() const {
    if (p_osr == NULL)
      return std::vector<std::string>();

    const char * const * p_cNames = openslide_get_associated_image_names(p_osr);

    if (p_cNames == NULL)
      return std::vector<std::string>();
  
    std::vector<std::string> vNames;

    for (int i = 0; p_cNames[i] != NULL; ++i)
      vNames.push_back(p_cNames[i]);

    return vNames;
  }

  MetaDataDictionary GetMetaDataDictionary() const {
    if (p_osr == NULL)
      return MetaDataDictionary();

    MetaDataDictionary clTags;

    const char * const * p_cNames = openslide_get_property_names(p_osr);

    if (p_cNames != NULL) {
      std::string strValue;

      for (int i = 0; p_cNames[i] != NULL; ++i) {
        strValue.clear();

        if (GetPropertyValue(p_cNames[i], strValue))
          EncapsulateMetaData<std::string>(clTags, p_cNames[i], strValue);
      }
    }

    return clTags;
  }

  template<typename ValueType>
  bool GetPropertyValue(const char *p_cKey, ValueType &value) const {
    if (p_osr == NULL)
      return false;

    const char * const p_cValue = openslide_get_property_value(p_osr, p_cKey);
    if (p_cValue == NULL)
      return false;

    std::stringstream valueStream;

    valueStream << p_cValue;
    valueStream >> value;

    return valueStream.good();
  }

  template<>
  bool GetPropertyValue(const char *p_cKey, std::string &strValue) const {
    if (p_osr == NULL)
      return false;

    const char * const p_cValue = openslide_get_property_value(p_osr, p_cKey);
    if (p_cValue == NULL)
      return false;

    strValue = p_cValue;
    return true;
  }
};

OpenSlideImageIO::OpenSlideImageIO()
{
  typedef itk::RGBAPixel<unsigned char> PixelType;
  PixelType clPixel;

  m_p_clOpenSlideWrapper = NULL;
  m_p_clOpenSlideWrapper = new OpenSlideWrapper();

  this->SetNumberOfDimensions(2); // OpenSlide is 2D.
  this->SetPixelTypeInfo(&clPixel);  

  m_Spacing[0] = 1.0;
  m_Spacing[1] = 1.0;

  m_Origin[0] = 0.0;
  m_Origin[1] = 0.0;

  m_Dimensions[0] = 0;
  m_Dimensions[1] = 0;

  // Trestle, Aperio, and generic tiled tiff
  this->AddSupportedReadExtension(".tif");
  // Hamamatsu
  this->AddSupportedReadExtension(".vms");
  this->AddSupportedReadExtension(".vmu");
  // Aperio
  this->AddSupportedReadExtension(".svs");
  // MIRAX
  this->AddSupportedReadExtension(".mrxs");
}

OpenSlideImageIO::~OpenSlideImageIO()
{
  if (m_p_clOpenSlideWrapper != NULL) {
    delete m_p_clOpenSlideWrapper;
    m_p_clOpenSlideWrapper = NULL;
  }
}

void OpenSlideImageIO::PrintSelf(std::ostream& os, Indent indent) const {
  Superclass::PrintSelf(os, indent);
  if (m_p_clOpenSlideWrapper != NULL) {
    os << indent << "Level: " << m_p_clOpenSlideWrapper->i32Level << '\n';
    os << indent << "Associated Image: " << m_p_clOpenSlideWrapper->strAssociatedImage << '\n';
  }
}

bool OpenSlideImageIO::CanReadFile( const char* filename ) {
  std::string fname(filename);
  bool supportedExtension = false;
  ArrayOfExtensionsType::const_iterator extIt;

  for( extIt = this->GetSupportedReadExtensions().begin(); extIt != this->GetSupportedReadExtensions().end(); ++extIt) {
    if( fname.rfind( *extIt ) != std::string::npos ) {
      supportedExtension = true;
    }
  }
  if( !supportedExtension ) {
    return false;
  }

  return OpenSlideWrapper::CanReadFile(filename);
}

bool OpenSlideImageIO::CanStreamRead() {
  return m_p_clOpenSlideWrapper != NULL && m_p_clOpenSlideWrapper->CanStreamRead();
}

void OpenSlideImageIO::ReadImageInformation() {
  typedef RGBAPixel<unsigned char> PixelType;
  PixelType clPixel;

  this->SetNumberOfDimensions(2);
  this->SetPixelTypeInfo(&clPixel);

  m_Dimensions[0] = 0;
  m_Dimensions[1] = 0;

  m_Spacing[0] = 1.0;  // We'll look for JPEG pixel size information later,
  m_Spacing[1] = 1.0;  // but set the defaults now

  m_Origin[0] = 0.0;
  m_Origin[1] = 0.0;

  if (m_p_clOpenSlideWrapper == NULL) {
    itkExceptionMacro( "Error OpenSlideImageIO could not open file: "
                       << this->GetFileName()
                       << std::endl
                       << "Reason: NULL OpenSlideWrapper pointer.");
  }

  if (!m_p_clOpenSlideWrapper->Open(this->GetFileName())) {
    itkExceptionMacro( "Error OpenSlideImageIO could not open file: "
                       << this->GetFileName()
                       << std::endl
                       << "Reason: "
                       << itksys::SystemTools::GetLastSystemError() );
  }

  {
    double a_dSpacing[2] = { 1.0, 1.0 };

    if (m_p_clOpenSlideWrapper->GetSpacing(a_dSpacing[0], a_dSpacing[1])) {
      m_Spacing[0] = a_dSpacing[0];
      m_Spacing[1] = a_dSpacing[1];
    }
  }

  {
    double a_dOrigin[2] = { 0.0, 0.0 };

    if (m_p_clOpenSlideWrapper->GetOrigin(a_dOrigin[0], a_dOrigin[1])) {
      m_Origin[0] = a_dOrigin[0]; 
      m_Origin[1] = a_dOrigin[1];
    }
  }

  {
    int64_t i64Width = 0, i64Height = 0;
    if (!m_p_clOpenSlideWrapper->GetDimensions(i64Width, i64Height)) {
      itkExceptionMacro( "Error OpenSlideImageIO could not read dimensions: "
                         << this->GetFileName()
                         << std::endl
                         << "Reason: Unknown." );
    }

    if (i64Width > std::numeric_limits<itk::SizeValueType>::max() || i64Height > std::numeric_limits<itk::SizeValueType>::max()) {
      itkExceptionMacro( "Error OpenSlideImageIO image dimensions are too large for SizeValueType: "
                         << this->GetFileName()
                         << std::endl
                         << "Reason: " 
                         << i64Width << " > " << std::numeric_limits<itk::SizeValueType>::max() 
                         << " or " << i64Height << " > " << std::numeric_limits<itk::SizeValueType>::max() );
    }

    m_Dimensions[0] = (SizeValueType)i64Width;
    m_Dimensions[1] = (SizeValueType)i64Height;
  }

  this->SetMetaDataDictionary(m_p_clOpenSlideWrapper->GetMetaDataDictionary());
}


void OpenSlideImageIO::Read( void * buffer)
{
  uint32_t * const p_u32Buffer = (uint32_t *)buffer;

  if (m_p_clOpenSlideWrapper == NULL || !m_p_clOpenSlideWrapper->IsOpened()) {
    itkExceptionMacro( "Error OpenSlideImageIO could not read region: "
                       << this->GetFileName()
                       << std::endl
                       << "Reason: OpenSlide context is not opened." );
  }

  const ImageIORegion clRegionToRead = this->GetIORegion();
  const ImageIORegion::SizeType clSize = clRegionToRead.GetSize();
  const ImageIORegion::IndexType clStart = clRegionToRead.GetIndex();

  if ( ((int64_t)clSize[0])*((int64_t)clSize[1]) > std::numeric_limits<ImageIORegion::SizeValueType>::max() ) {
    itkExceptionMacro( "Error OpenSlideImageIO could not read region: "
                       << this->GetFileName()
                       << std::endl
                       << "Reason: Requested region size in pixels overflows." );
  }

  const char *p_cError = m_p_clOpenSlideWrapper->ReadRegion(p_u32Buffer, clStart[0], clStart[1], clSize[0], clSize[1]);

  if (p_cError != NULL) {
    std::string strError = p_cError;
    m_p_clOpenSlideWrapper->Close(); // Can only safely close this now
    itkExceptionMacro( "Error OpenSlideImageIO could not read region: "
                       << this->GetFileName()
                       << std::endl
                       << "Reason: " << strError );
  }

  // Re-order the bytes
  const int64_t i64TotalSize = clRegionToRead.GetNumberOfPixels();
  for (int64_t i = 0; i < i64TotalSize; ++i) {
    // XXX: Endianness?
    RGBAPixel<unsigned char> clPixel;
    clPixel.SetRed((p_u32Buffer[i] >> 16) & 0xff);
    clPixel.SetGreen((p_u32Buffer[i] >> 8) & 0xff);
    clPixel.SetBlue(p_u32Buffer[i] & 0xff);
    clPixel.SetAlpha((p_u32Buffer[i] >> 24) & 0xff);

    p_u32Buffer[i] = *(uint32_t *)clPixel.GetDataPointer();
  }
}

bool OpenSlideImageIO::CanWriteFile( const char * name )
{
  return false;
}

void
OpenSlideImageIO
::WriteImageInformation(void) {
  // add writing here
}


/**
 *
 */
void
OpenSlideImageIO::Write( const void* buffer) {
}

/** Given a requested region, determine what could be the region that we can
 * read from the file. This is called the streamable region, which will be
 * smaller than the LargestPossibleRegion and greater or equal to the
RequestedRegion */
ImageIORegion
OpenSlideImageIO::GenerateStreamableReadRegionFromRequestedRegion( const ImageIORegion & requested ) const {
  return requested;
}

void OpenSlideImageIO::SetLevel(int iLevel) {
  m_p_clOpenSlideWrapper->i32Level = iLevel;
  m_p_clOpenSlideWrapper->strAssociatedImage.clear();
}

int OpenSlideImageIO::GetLevel() const {
  return m_p_clOpenSlideWrapper->i32Level;
}

void OpenSlideImageIO::SetAssociatedImageName(const std::string &strName) {
  m_p_clOpenSlideWrapper->strAssociatedImage = strName;
  m_p_clOpenSlideWrapper->i32Level = 0;
}

std::string OpenSlideImageIO::GetAssociatedImageName() const {
  return m_p_clOpenSlideWrapper->strAssociatedImage;
}

bool OpenSlideImageIO::SetLevelForDownsampleFactor(double dDownsampleFactor) {
  if (m_p_clOpenSlideWrapper == NULL)
    return false;

  const int iLevel = m_p_clOpenSlideWrapper->GetBestLevelForDownsample(dDownsampleFactor);

  if (iLevel < 0)
    return false;

  SetLevel(iLevel);

  return true;
}

OpenSlideImageIO::AssociatedImageNameContainer OpenSlideImageIO::GetAssociatedImageNames() const {
  return m_p_clOpenSlideWrapper->GetAssociatedImageNames();
}

} // end namespace itk
