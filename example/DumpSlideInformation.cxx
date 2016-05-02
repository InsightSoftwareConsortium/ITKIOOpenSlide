#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <sstream>

#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkOpenSlideImageIO.h"
#include "itkImage.h"
#include "itkRGBAPixel.h"
#include "itkMetaDataObject.h"

void Usage(const char *p_cArg0) {
  std::cerr << "Usage: " << p_cArg0 << " slideFile" << std::endl;
  exit(1);
}

class DumpSlideInformation {
public:
  typedef itk::RGBAPixel<unsigned char> PixelType;
  typedef itk::Image<PixelType, 2> ImageType;
  typedef itk::OpenSlideImageIO ReaderIOType;

  DumpSlideInformation() {
    m_p_clReaderIO = ReaderIOType::New();
  }

  // Don't try to load an image larger than this size
  size_t MaxImageSizeInBytes() const {
    return (size_t)1024 * 1024 * 100;
  }

  void SetFileName(const std::string &strFileName) {
    m_p_clReaderIO->SetFileName(strFileName);
  }

  bool DumpInformation() const;
  bool WriteLevels() const;
  bool WriteAssociatedImages() const;

private:
  ReaderIOType::Pointer m_p_clReaderIO;

  bool ReadImageInformation() const {
    try {
      m_p_clReaderIO->ReadImageInformation();
    }
    catch (itk::ExceptionObject &e) {
      std::cerr << "Error: " << e << std::endl;
      return false;
    }

    return true;
  }

  ImageType::SizeType GetSize() const {
    ImageType::SizeType clSize;  
    clSize[0] = m_p_clReaderIO->GetDimensions(0);
    clSize[1] = m_p_clReaderIO->GetDimensions(1);
    return clSize;
  }

  ImageType::SpacingType GetSpacing() const {
    ImageType::SpacingType clSpacing;
    clSpacing[0] = m_p_clReaderIO->GetSpacing(0);
    clSpacing[1] = m_p_clReaderIO->GetSpacing(1);
    return clSpacing;
  }

  bool DumpImageInformation() const;
  bool DumpMetaData() const;
  bool DumpLevelInformation() const;
  bool DumpAssociatedImageInformation() const;

  bool WriteImage(const char *p_cFileName) const;
};


int main(int argc, char **argv) {
  const char * const p_cArg0 = argv[0];
  if (argc != 2)
    Usage(p_cArg0);

  std::string strSlideImage = argv[1];

  DumpSlideInformation clInfo;
  clInfo.SetFileName(strSlideImage);

  if (!clInfo.DumpInformation() || !clInfo.WriteLevels() || !clInfo.WriteAssociatedImages())
    return -1;

  return 0;
}

bool DumpSlideInformation::DumpInformation() const {
  return ReadImageInformation() && DumpImageInformation() && DumpMetaData() && DumpLevelInformation() && DumpAssociatedImageInformation();
}

bool DumpSlideInformation::DumpImageInformation() const {
  std::cout << "\nImage Information:\n" << std::endl;
  std::cout << "Dimensions: " << m_p_clReaderIO->GetNumberOfDimensions() << std::endl;
  std::cout << "Component type: " << itk::ImageIOBase::GetComponentTypeAsString(m_p_clReaderIO->GetComponentType()) << std::endl;
  std::cout << "Pixel type: " << itk::ImageIOBase::GetPixelTypeAsString(m_p_clReaderIO->GetPixelType()) << std::endl;
  std::cout << "Vendor: " << m_p_clReaderIO->GetVendor() << std::endl;

  return true;
}

bool DumpSlideInformation::DumpMetaData() const {
  std::cout << "\nMeta Data:\n" << std::endl;

  itk::MetaDataDictionary &clTags = m_p_clReaderIO->GetMetaDataDictionary();
  std::vector<std::string> vKeys = clTags.GetKeys();

  std::cout << "Number of keys: " << vKeys.size() << std::endl;
  std::cout << "Entries:" << std::endl;
  for (size_t i = 0; i < vKeys.size(); ++i) {
    const std::string &strKey = vKeys[i];
    std::string strValue;

    if (itk::ExposeMetaData(clTags, strKey, strValue))
      std::cout << strKey << " = " << strValue << std::endl;
  }

  return true;
}

bool DumpSlideInformation::DumpLevelInformation() const {
  std::cout << "\nLevel Information:\n" << std::endl;

  const int iLevelCount = m_p_clReaderIO->GetLevelCount();
  std::cout << "Level count: " << iLevelCount << std::endl;

  std::cout << "Levels:" << std::endl;
  for (int iLevel = 0; iLevel < iLevelCount; ++iLevel) {
    m_p_clReaderIO->SetLevel(iLevel);

    if (!ReadImageInformation())
      return false;

    std::cout << "Level " << iLevel << ": dimensions = " << GetSize() << ", spacing = " << GetSpacing() << ", size in bytes = " << m_p_clReaderIO->GetImageSizeInBytes() << std::endl;
  }

  return true;
}

bool DumpSlideInformation::DumpAssociatedImageInformation() const {
  std::cout << "\nAssociated image information:\n" << std::endl;

  ReaderIOType::AssociatedImageNameContainer vAssociatedImages = m_p_clReaderIO->GetAssociatedImageNames();

  std::cout << "Number of associated images: " << vAssociatedImages.size() << std::endl;
  std::cout << "Associated image names:" << std::endl;

  const size_t numWordsPerLine = 3;

  for (size_t i = 0; i < vAssociatedImages.size(); i += numWordsPerLine) {
    const size_t jBegin = i;
    const size_t jEnd = std::min(vAssociatedImages.size(), jBegin + numWordsPerLine);

    std::cout << '\'' << vAssociatedImages[jBegin] << '\'';

    for (size_t j = jBegin+1; j < jEnd; ++j)
      std::cout << ", '" << vAssociatedImages[j] << '\'';

    std::cout << std::endl;
  }

  std::cout << "\nAssociated images:" << std::endl;

  for (size_t i = 0; i < vAssociatedImages.size(); ++i) {
    const std::string &strAssociatedImage = vAssociatedImages[i];

    m_p_clReaderIO->SetAssociatedImageName(strAssociatedImage);

    if (!ReadImageInformation())
      return false;

    std::cout << strAssociatedImage << ": dimensions = " << GetSize() << ", spacing = " << GetSpacing() << ", size in bytes = " << m_p_clReaderIO->GetImageSizeInBytes() << std::endl;
  }

  return true;
}

bool DumpSlideInformation::WriteLevels() const {
  std::cout << "\nWriting level images to file ...\n" << std::endl;

  const int iLevelCount = m_p_clReaderIO->GetLevelCount();

  std::stringstream nameStream;

  for (int iLevel = 0; iLevel < iLevelCount; ++iLevel) {
    m_p_clReaderIO->SetLevel(iLevel);

    if (!ReadImageInformation())
      return false;

    if (m_p_clReaderIO->GetImageSizeInBytes() > MaxImageSizeInBytes()) {
      std::cout << "Level " << iLevel << " image is too large. Skipping." << std::endl;
      continue;
    }

    nameStream.str("");
    nameStream.clear();

    nameStream << "level" << iLevel << ".tiff";

    std::cout << "Wrting level " << iLevel << " to '" << nameStream.str() << "' ..." << std::endl;

    WriteImage(nameStream.str().c_str());
  }

  return true;
}

bool DumpSlideInformation::WriteAssociatedImages() const {
  std::cout << "\nWriting associated images to file ...\n" << std::endl;

  ReaderIOType::AssociatedImageNameContainer vAssociatedImages = m_p_clReaderIO->GetAssociatedImageNames();

  for (size_t i = 0; i < vAssociatedImages.size(); ++i) {
    const std::string &strAssociatedImage = vAssociatedImages[i];

    m_p_clReaderIO->SetAssociatedImageName(strAssociatedImage);

    if (!ReadImageInformation())
      return false;

    if (m_p_clReaderIO->GetImageSizeInBytes() > MaxImageSizeInBytes()) {
      std::cout << "Associated image '" << strAssociatedImage << "' is too large. Skipping." << std::endl;
      continue;
    }

    std::string strFileName = strAssociatedImage + ".tiff";

    std::cout << "Writing associated image '" << strAssociatedImage << "' to '" << strFileName << "' ..." << std::endl;

    WriteImage(strFileName.c_str());
  }

  return true;
}

bool DumpSlideInformation::WriteImage(const char *p_cFileName) const {
  typedef itk::ImageFileReader<ImageType> ReaderType;
  typedef itk::ImageFileWriter<ImageType> WriterType;
 
  ReaderType::Pointer p_clReader = ReaderType::New();

  const std::string strInputFileName = m_p_clReaderIO->GetFileName();

  p_clReader->SetImageIO(m_p_clReaderIO);
  p_clReader->SetFileName(strInputFileName);
  
  WriterType::Pointer p_clWriter = WriterType::New();
  p_clWriter->SetInput(p_clReader->GetOutput());
  p_clWriter->SetFileName(p_cFileName);

  try {
    p_clWriter->Update();
  }
  catch (itk::ExceptionObject &e) {
    std::cerr << "Error: " << e << std::endl;
    return false;
  }

  return true;
}

