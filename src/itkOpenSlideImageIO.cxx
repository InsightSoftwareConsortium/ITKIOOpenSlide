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
#include <sstream>

#include "itkIOCommon.h"
#include "itkOpenSlideImageIO.h"
#include "itksys/SystemTools.hxx"
#include "itkMetaDataDictionary.h"
#include "itkMetaDataObject.h"

// OpenSlide
#include "openslide.h"

namespace itk
{

// OpenSlide wrapper class
// This is responsible for freeing the OpenSlide context on destruction
// It also allows for seamless access to various levels and associated images through one set of functions (as opposed to two)
class OpenSlideWrapper {
public:
  // Detects the vendor. Should return NULL if the file is not readable.
  static const char * DetectVendor(const char *p_cFileName) {
    return openslide_detect_vendor(p_cFileName);
  }

  // Weak check if the file can be read
  static bool CanReadFile(const char *p_cFileName) {
    return DetectVendor(p_cFileName) != NULL;
  }

  // Returns version of OpenSlide library
  static const char * GetVersion() {
    return openslide_get_version();
  }

  // Constructors
  OpenSlideWrapper() {
    m_Osr = NULL;
    m_Level = 0;
  }

  OpenSlideWrapper(const char *p_cFileName) {
    m_Osr = NULL;
    m_Level = 0;
    Open(p_cFileName);
  }

  // Destructor
  ~OpenSlideWrapper() {
    Close();
  }

  // Tells the ImageIO if the wrapper is in a state where stream reading can occur.
  // While OpenSlide supports reading regions of level images, it does not for associated images.
  bool CanStreamRead() const {
    return m_AssociatedImage.empty();
  }

  // Closes the currently opened file
  void Close() {
    if (m_Osr != NULL) {
      openslide_close(m_Osr);
      m_Osr = NULL;
    }
  }

  // Checks weather a slide file is currently opened
  bool IsOpened() const {
    return m_Osr != NULL;
  }

  // Opens a slide file
  bool Open(const char *p_cFileName) {
    Close();
    m_Osr = openslide_open(p_cFileName);
    return m_Osr != NULL;
  }

  // Get error string, NULL if there is no error
  const char * GetError() const {
    if (m_Osr == NULL)
      return "OpenSlideWrapper has no file open.";

    return openslide_get_error(m_Osr);
  }

  // Sets the level that is accessible with ReadRegion, GetDimensions, GetSpacing.
  // Clears any associated image context.
  void SetLevel(int32_t i32Level) {
    m_Level = i32Level;
    m_AssociatedImage.clear();
  }

  // Returns the currently selected level
  int32_t GetLevel() const {
    return m_Level;
  }

  // Sets the associated image that is accessible with ReadRegion, GetDimensions
  void SetAssociatedImageName(const std::string &strImageName) {
    m_AssociatedImage = strImageName;
    m_Level = 0;
  }

  // Returns the currently selected associated image
  const std::string & GetAssociatedImageName() const {
    return m_AssociatedImage;
  }

  // Given a downsample factor, uses OpenSlide to determine the best level to use.
  bool SetBestLevelForDownsample(double dDownsample) {
    if (m_Osr == NULL)
      return false;

    const int32_t i32Level = openslide_get_best_level_for_downsample(m_Osr, dDownsample);

    if (i32Level < 0)
      return false;

    SetLevel(i32Level);

    return true;
  }

  // Returns the number of levels in this file
  int32_t GetLevelCount() const {
    if (m_Osr == NULL)
      return -1;

    return openslide_get_level_count(m_Osr);
  }

  // Returns NULL for success
  // NOTE: When reading associated images, x, y, width and height are ignored.
  const char * ReadRegion(uint32_t *p_ui32Dest, int64_t i64X, int64_t i64Y, int64_t i64Width, int64_t i64Height) const {
    if (m_Osr == NULL)
      return "OpenSlideWrapper has no file open.";

    if (m_AssociatedImage.size() > 0) {
      openslide_read_associated_image(m_Osr, m_AssociatedImage.c_str(), p_ui32Dest);
    }
    else {
      const double dDownsampleFactor = openslide_get_level_downsample(m_Osr, m_Level);

      if (dDownsampleFactor <= 0.0)
        return "Could not get downsample factor.";

      // NOTE: API expects level 0 coordinates. So we upsample the coordinates.
      // XXX: This can subtly change the image compared to reading all at once.
      //      The handling of coordinates internally in OpenSlide is quite similar!
      i64X = (int64_t)(i64X * dDownsampleFactor);
      i64Y = (int64_t)(i64Y * dDownsampleFactor);

      openslide_read_region(m_Osr, p_ui32Dest, i64X, i64Y, m_Level, i64Width, i64Height);
    }

    return openslide_get_error(m_Osr);
  }

  // Computes the spacing depending on selected level
  // Default spacing is relative to 1 MPP if the function fails to detect spacing information (downsample factor is considered)
  bool GetSpacing(double &dSpacingX, double &dSpacingY) const {
    dSpacingX = dSpacingY = 1.0;

    if (m_Osr == NULL)
      return false;

    if (m_AssociatedImage.size() > 0)
      return false;

    const double dDownsample = openslide_get_level_downsample(m_Osr, m_Level);

    if (dDownsample <= 0.0)
      return false;

    if (!GetPropertyValue(OPENSLIDE_PROPERTY_NAME_MPP_X, dSpacingX) || !GetPropertyValue(OPENSLIDE_PROPERTY_NAME_MPP_Y, dSpacingY)) {
      dSpacingX = dSpacingY = dDownsample;
      return false;
    }

    dSpacingX *= dDownsample;
    dSpacingY *= dDownsample;

    return true;
  }

  // Returns the dimension of the level or associated image
  bool GetDimensions(int64_t &i64Width, int64_t &i64Height) const {
    i64Width = i64Height = 0;

    if (m_Osr == NULL)
      return false;

    if (m_AssociatedImage.size() > 0)
      openslide_get_associated_image_dimensions(m_Osr, m_AssociatedImage.c_str(), &i64Width, &i64Height);
    else
      openslide_get_level_dimensions(m_Osr, m_Level, &i64Width, &i64Height);

    return i64Width > 0 && i64Height > 0;
  }

  // Retrieves associated image names from the open slide and places them into a std::vector
  std::vector<std::string> GetAssociatedImageNames() const {
    if (m_Osr == NULL)
      return std::vector<std::string>();

    const char * const * p_cNames = openslide_get_associated_image_names(m_Osr);

    if (p_cNames == NULL)
      return std::vector<std::string>();
  
    std::vector<std::string> vNames;

    for (int i = 0; p_cNames[i] != NULL; ++i)
      vNames.push_back(p_cNames[i]);

    return vNames;
  }

  // Forms an ITK MetaDataDictionary
  MetaDataDictionary GetMetaDataDictionary() const {
    if (m_Osr == NULL)
      return MetaDataDictionary();

    MetaDataDictionary clTags;

    const char * const * p_cNames = openslide_get_property_names(m_Osr);

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

  // Templated functions for accessing and casting property values
  template<typename ValueType>
  bool GetPropertyValue(const char *p_cKey, ValueType &value) const {
    if (m_Osr == NULL)
      return false;

    const char * const p_cValue = openslide_get_property_value(m_Osr, p_cKey);
    if (p_cValue == NULL)
      return false;

    std::stringstream valueStream;

    valueStream << p_cValue;
    valueStream >> value;

    return !valueStream.fail() && !valueStream.bad();
  }

  bool GetPropertyValue(const char *p_cKey, std::string &strValue) const {
    if (m_Osr == NULL)
      return false;

    const char * const p_cValue = openslide_get_property_value(m_Osr, p_cKey);
    if (p_cValue == NULL)
      return false;

    strValue = p_cValue;
    return true;
  }

private:
  openslide_t *m_Osr;
  int32_t      m_Level;
  std::string  m_AssociatedImage;
};

OpenSlideImageIO::OpenSlideImageIO()
{
  typedef RGBAPixel<unsigned char> PixelType;
  PixelType clPixel;

  m_OpenSlideWrapper = NULL;
  m_OpenSlideWrapper = new OpenSlideWrapper();

  this->SetNumberOfDimensions(2); // OpenSlide is 2D.
  this->SetPixelTypeInfo(&clPixel);  

  m_Spacing[0] = 1.0;
  m_Spacing[1] = 1.0;

  m_Origin[0] = 0.0;
  m_Origin[1] = 0.0;

  m_Dimensions[0] = 0;
  m_Dimensions[1] = 0;

  // Trestle, Aperio, Ventana, and generic tiled tiff
  this->AddSupportedReadExtension(".tif");
  // Hamamatsu
  this->AddSupportedReadExtension(".vms");
  this->AddSupportedReadExtension(".vmu");
  this->AddSupportedReadExtension(".ndpi");
  // Aperio
  this->AddSupportedReadExtension(".svs");
  // MIRAX
  this->AddSupportedReadExtension(".mrxs");
  // Leica
  this->AddSupportedReadExtension(".scn");
  // Philips
  this->AddSupportedReadExtension(".tiff");
  // Ventana
  this->AddSupportedReadExtension(".bif");
  // Sakura
  this->AddSupportedReadExtension(".svslide");
}

OpenSlideImageIO::~OpenSlideImageIO()
{
  if (m_OpenSlideWrapper != NULL) {
    delete m_OpenSlideWrapper;
    m_OpenSlideWrapper = NULL;
  }
}

void OpenSlideImageIO::PrintSelf(std::ostream& os, Indent indent) const {
  Superclass::PrintSelf(os, indent);
  os << indent << "Level: " << GetLevel() << '\n';
  os << indent << "Associated Image: " << GetAssociatedImageName() << '\n';
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
  return m_OpenSlideWrapper != NULL && m_OpenSlideWrapper->CanStreamRead();
}

void OpenSlideImageIO::ReadImageInformation() {
  typedef RGBAPixel<unsigned char> PixelType;
  PixelType clPixel;

  this->SetNumberOfDimensions(2);
  this->SetPixelTypeInfo(&clPixel);

  m_Dimensions[0] = 0;
  m_Dimensions[1] = 0;

  m_Spacing[0] = 1.0;
  m_Spacing[1] = 1.0;

  m_Origin[0] = 0.0;
  m_Origin[1] = 0.0;

  if (m_OpenSlideWrapper == NULL) {
    itkExceptionMacro( "Error OpenSlideImageIO could not open file: "
                       << this->GetFileName()
                       << std::endl
                       << "Reason: NULL OpenSlideWrapper pointer.");
  }

  if (!m_OpenSlideWrapper->Open(this->GetFileName())) {
    itkExceptionMacro( "Error OpenSlideImageIO could not open file: "
                       << this->GetFileName()
                       << std::endl
                       << "Reason: "
                       << itksys::SystemTools::GetLastSystemError() );
    // NOTE: OpenSlide needs to be opened to query API for errors. This is assumed to be related to a system error.
  }


  // This will fill in default values as needed (in case it fails)
  m_OpenSlideWrapper->GetSpacing(m_Spacing[0], m_Spacing[1]);

  {
    int64_t i64Width = 0, i64Height = 0;
    if (!m_OpenSlideWrapper->GetDimensions(i64Width, i64Height)) {
      std::string strError = "Unknown";
      const char * const p_cError = m_OpenSlideWrapper->GetError();

      if (p_cError != NULL) {
        strError = p_cError;
        m_OpenSlideWrapper->Close(); // Can only safely close this now
      }

      itkExceptionMacro( "Error OpenSlideImageIO could not read dimensions: "
                         << this->GetFileName()
                         << std::endl
                         << "Reason: " << strError );
    }

    // i64Width and i64Height are known to be positive
    if ((uint64_t)i64Width > std::numeric_limits<SizeValueType>::max() || (uint64_t)i64Height > std::numeric_limits<SizeValueType>::max()) {
      itkExceptionMacro( "Error OpenSlideImageIO image dimensions are too large for SizeValueType: "
                         << this->GetFileName()
                         << std::endl
                         << "Reason: " 
                         << i64Width << " > " << std::numeric_limits<SizeValueType>::max() 
                         << " or " << i64Height << " > " << std::numeric_limits<SizeValueType>::max() );
    }

    m_Dimensions[0] = (SizeValueType)i64Width;
    m_Dimensions[1] = (SizeValueType)i64Height;
  }

  this->SetMetaDataDictionary(m_OpenSlideWrapper->GetMetaDataDictionary());
}


void OpenSlideImageIO::Read( void * buffer)
{
  uint32_t * const p_u32Buffer = (uint32_t *)buffer;

  if (m_OpenSlideWrapper == NULL || !m_OpenSlideWrapper->IsOpened()) {
    itkExceptionMacro( "Error OpenSlideImageIO could not read region: "
                       << this->GetFileName()
                       << std::endl
                       << "Reason: OpenSlide context is not opened." );
  }

  const ImageIORegion clRegionToRead = this->GetIORegion();
  const ImageIORegion::SizeType clSize = clRegionToRead.GetSize();
  const ImageIORegion::IndexType clStart = clRegionToRead.GetIndex();

  if ( ((uint64_t)clSize[0])*((uint64_t)clSize[1]) > std::numeric_limits<ImageIORegion::SizeValueType>::max() ) {
    itkExceptionMacro( "Error OpenSlideImageIO could not read region: "
                       << this->GetFileName()
                       << std::endl
                       << "Reason: Requested region size in pixels overflows." );
  }

  const char *p_cError = m_OpenSlideWrapper->ReadRegion(p_u32Buffer, clStart[0], clStart[1], clSize[0], clSize[1]);

  if (p_cError != NULL) {
    std::string strError = p_cError; // Copy this since Close() may destroy the backing buffer
    m_OpenSlideWrapper->Close(); // Can only safely close this now
    itkExceptionMacro( "Error OpenSlideImageIO could not read region: "
                       << this->GetFileName()
                       << std::endl
                       << "Reason: " << strError );
  }

  // Re-order the bytes (ARGB -> RGBA)
  const int64_t i64TotalSize = clRegionToRead.GetNumberOfPixels();
  for (int64_t i = 0; i < i64TotalSize; ++i) {
    // XXX: Endianness?
    RGBAPixel<unsigned char> clPixel;
    clPixel.SetRed((p_u32Buffer[i] >> 16) & 0xff);
    clPixel.SetGreen((p_u32Buffer[i] >> 8) & 0xff);
    clPixel.SetBlue(p_u32Buffer[i] & 0xff);
    clPixel.SetAlpha((p_u32Buffer[i] >> 24) & 0xff);

    p_u32Buffer[i] = *reinterpret_cast<uint32_t *>(clPixel.GetDataPointer());
  }
}

bool OpenSlideImageIO::CanWriteFile( const char * /*name*/ )
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
OpenSlideImageIO::Write( const void* /*buffer*/) {
}

/** Given a requested region, determine what could be the region that we can
 * read from the file. This is called the streamable region, which will be
 * smaller than the LargestPossibleRegion and greater or equal to the
RequestedRegion */
ImageIORegion
OpenSlideImageIO::GenerateStreamableReadRegionFromRequestedRegion( const ImageIORegion & requested ) const {
  return requested;
}

/** Get underlying OpenSlide library version */
std::string OpenSlideImageIO::GetOpenSlideVersion() const {
  const char * const p_cVersion = OpenSlideWrapper::GetVersion();
  return p_cVersion != NULL ? std::string(p_cVersion) : std::string();
}

/** Detect the vendor of the current file. */
std::string OpenSlideImageIO::GetVendor() const {
  const char * const p_cVendor = OpenSlideWrapper::DetectVendor(this->GetFileName());
  return p_cVendor != NULL ? std::string(p_cVendor) : std::string();
}

/** Sets the level to read. Level 0 (default) is the highest resolution level.
 * This method overrides any previously selected associated image. 
 * Call ReadImageInformation() again after calling this function. */
void OpenSlideImageIO::SetLevel(int iLevel) {
  if (m_OpenSlideWrapper == NULL)
    return;

  m_OpenSlideWrapper->SetLevel(iLevel);
}

/** Returns the currently selected level. */
int OpenSlideImageIO::GetLevel() const {
  if (m_OpenSlideWrapper == NULL)
    return -1;

  return m_OpenSlideWrapper->GetLevel();
}

/** Returns the number of available levels. */
int OpenSlideImageIO::GetLevelCount() const {
  if (m_OpenSlideWrapper == NULL)
    return -1;

  return m_OpenSlideWrapper->GetLevelCount();
}

/** Sets the associated image to extract.
 * This method overrides any previously selected level.
 * Call ReadImageInformation() again after calling this function. */
void OpenSlideImageIO::SetAssociatedImageName(const std::string &strName) {
  if (m_OpenSlideWrapper == NULL)
    return;

  m_OpenSlideWrapper->SetAssociatedImageName(strName);
}

/** Returns the currently selected associated image name (empty string if none). */
std::string OpenSlideImageIO::GetAssociatedImageName() const {
  if (m_OpenSlideWrapper == NULL)
    return std::string();

  return m_OpenSlideWrapper->GetAssociatedImageName();
}

/** Sets the best level to read for the given downsample factor.
 * This method overrides any previously selected associated image. 
 * Call ReadImageInformation() again after calling this function. */
bool OpenSlideImageIO::SetLevelForDownsampleFactor(double dDownsampleFactor) {
  if (m_OpenSlideWrapper == NULL)
    return false;

  return m_OpenSlideWrapper->SetBestLevelForDownsample(dDownsampleFactor);
}

/** Returns all associated image names stored in the file. */
OpenSlideImageIO::AssociatedImageNameContainer OpenSlideImageIO::GetAssociatedImageNames() const {
  if (m_OpenSlideWrapper == NULL)
    return AssociatedImageNameContainer();

  return m_OpenSlideWrapper->GetAssociatedImageNames();
}

} // end namespace itk
