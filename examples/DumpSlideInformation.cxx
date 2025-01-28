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

void
Usage(const char * cArg0)
{
  std::cerr << "Usage: " << cArg0 << " slideFile" << std::endl;
  exit(1);
}


class DumpSlideInformation
{
public:
  using PixelType = itk::RGBAPixel<unsigned char>;
  using ImageType = itk::Image<PixelType, 2>;
  using ReaderIOType = itk::OpenSlideImageIO;

  DumpSlideInformation() { m_CLReaderIO = ReaderIOType::New(); }

  // Don't try to load an image larger than this size
  size_t
  MaxImageSizeInBytes() const
  {
    return (size_t)1024 * 1024 * 100;
  }

  void
  SetFileName(const std::string & fileName)
  {
    m_CLReaderIO->SetFileName(fileName);
  }

  bool
  DumpInformation() const;
  bool
  WriteLevels() const;
  bool
  WriteAssociatedImages() const;

private:
  ReaderIOType::Pointer m_CLReaderIO;

  bool
  ReadImageInformation() const
  {
    try
    {
      m_CLReaderIO->ReadImageInformation();
    }
    catch (itk::ExceptionObject & e)
    {
      std::cerr << "Error: " << e << std::endl;
      return false;
    }

    return true;
  }

  ImageType::SizeType
  GetSize() const
  {
    ImageType::SizeType clSize;
    clSize[0] = m_CLReaderIO->GetDimensions(0);
    clSize[1] = m_CLReaderIO->GetDimensions(1);
    return clSize;
  }

  ImageType::SpacingType
  GetSpacing() const
  {
    ImageType::SpacingType clSpacing;
    clSpacing[0] = m_CLReaderIO->GetSpacing(0);
    clSpacing[1] = m_CLReaderIO->GetSpacing(1);
    return clSpacing;
  }

  bool
  DumpImageInformation() const;
  bool
  DumpMetaData() const;
  bool
  DumpLevelInformation() const;
  bool
  DumpAssociatedImageInformation() const;

  bool
  WriteImage(const char * fileName) const;
};


int
main(int argc, char ** argv)
{
  const char * const cArg0 = argv[0];
  if (argc != 2)
  {
    Usage(cArg0);
    return EXIT_FAILURE;
  }

  std::string slideImage = argv[1];

  DumpSlideInformation clInfo;
  clInfo.SetFileName(slideImage);

  if (!clInfo.DumpInformation() || !clInfo.WriteLevels() || !clInfo.WriteAssociatedImages())
  {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}


bool
DumpSlideInformation ::DumpInformation() const
{
  return ReadImageInformation() && DumpImageInformation() && DumpMetaData() && DumpLevelInformation() &&
         DumpAssociatedImageInformation();
}


bool
DumpSlideInformation ::DumpImageInformation() const
{
  std::cout << "\nImage Information:\n" << std::endl;
  std::cout << "Dimensions: " << m_CLReaderIO->GetNumberOfDimensions() << std::endl;
  std::cout << "Component type: " << itk::ImageIOBase::GetComponentTypeAsString(m_CLReaderIO->GetComponentType())
            << std::endl;
  std::cout << "Pixel type: " << itk::ImageIOBase::GetPixelTypeAsString(m_CLReaderIO->GetPixelType()) << std::endl;
  std::cout << "Vendor: " << m_CLReaderIO->GetVendor() << std::endl;

  return true;
}


bool
DumpSlideInformation ::DumpMetaData() const
{
  std::cout << "\nMeta Data:\n" << std::endl;

  itk::MetaDataDictionary & clTags = m_CLReaderIO->GetMetaDataDictionary();
  std::vector<std::string>  keys = clTags.GetKeys();

  std::cout << "Number of keys: " << keys.size() << std::endl;
  std::cout << "Entries:" << std::endl;
  for (size_t i = 0; i < keys.size(); ++i)
  {
    const std::string & key = keys[i];
    std::string         value;

    if (itk::ExposeMetaData(clTags, key, value))
    {
      std::cout << key << " = " << value << std::endl;
    }
  }

  return true;
}


bool
DumpSlideInformation ::DumpLevelInformation() const
{
  std::cout << "\nLevel Information:\n" << std::endl;

  const int levelCount = m_CLReaderIO->GetLevelCount();
  std::cout << "Level count: " << levelCount << std::endl;

  std::cout << "Levels:" << std::endl;
  for (int level = 0; level < levelCount; ++level)
  {
    m_CLReaderIO->SetLevel(level);

    if (!ReadImageInformation())
    {
      return false;
    }

    std::cout << "Level " << level << ": dimensions = " << GetSize() << ", spacing = " << GetSpacing()
              << ", size in bytes = " << m_CLReaderIO->GetImageSizeInBytes() << std::endl;
  }

  return true;
}


bool
DumpSlideInformation ::DumpAssociatedImageInformation() const
{
  std::cout << "\nAssociated image information:\n" << std::endl;

  ReaderIOType::AssociatedImageNameContainer associatedImages = m_CLReaderIO->GetAssociatedImageNames();

  std::cout << "Number of associated images: " << associatedImages.size() << std::endl;
  std::cout << "Associated image names:" << std::endl;

  const size_t numWordsPerLine = 3;

  for (size_t i = 0; i < associatedImages.size(); i += numWordsPerLine)
  {
    const size_t jBegin = i;
    const size_t jEnd = std::min(associatedImages.size(), jBegin + numWordsPerLine);

    std::cout << '\'' << associatedImages[jBegin] << '\'';

    for (size_t j = jBegin + 1; j < jEnd; ++j)
    {
      std::cout << ", '" << associatedImages[j] << '\'';
    }

    std::cout << std::endl;
  }

  std::cout << "\nAssociated images:" << std::endl;

  for (size_t i = 0; i < associatedImages.size(); ++i)
  {
    const std::string & associatedImage = associatedImages[i];

    m_CLReaderIO->SetAssociatedImageName(associatedImage);

    if (!this->ReadImageInformation())
    {
      return false;
    }

    std::cout << associatedImage << ": dimensions = " << GetSize() << ", spacing = " << GetSpacing()
              << ", size in bytes = " << m_CLReaderIO->GetImageSizeInBytes() << std::endl;
  }

  return true;
}


bool
DumpSlideInformation ::WriteLevels() const
{
  std::cout << "\nWriting level images to file ...\n" << std::endl;

  const int iLevelCount = m_CLReaderIO->GetLevelCount();

  std::stringstream nameStream;

  for (int iLevel = 0; iLevel < iLevelCount; ++iLevel)
  {
    m_CLReaderIO->SetLevel(iLevel);

    if (!ReadImageInformation())
      return false;

    if (m_CLReaderIO->GetImageSizeInBytes() > MaxImageSizeInBytes())
    {
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


bool
DumpSlideInformation ::WriteAssociatedImages() const
{
  std::cout << "\nWriting associated images to file ...\n" << std::endl;

  ReaderIOType::AssociatedImageNameContainer associatedImages = m_CLReaderIO->GetAssociatedImageNames();

  for (size_t i = 0; i < associatedImages.size(); ++i)
  {
    const std::string & associatedImage = associatedImages[i];

    m_CLReaderIO->SetAssociatedImageName(associatedImage);

    if (!this->ReadImageInformation())
    {
      return false;
    }

    if (m_CLReaderIO->GetImageSizeInBytes() > MaxImageSizeInBytes())
    {
      std::cout << "Associated image '" << associatedImage << "' is too large. Skipping." << std::endl;
      continue;
    }

    std::string fileName = associatedImage + ".tiff";

    std::cout << "Writing associated image '" << associatedImage << "' to '" << fileName << "' ..." << std::endl;

    this->WriteImage(fileName.c_str());
  }

  return true;
}


bool
DumpSlideInformation ::WriteImage(const char * fileName) const
{
  using ReaderType = itk::ImageFileReader<ImageType>;
  using WriterType = itk::ImageFileWriter<ImageType>;

  ReaderType::Pointer clReader = ReaderType::New();

  const std::string inputFileName = m_CLReaderIO->GetFileName();

  clReader->SetImageIO(m_CLReaderIO);
  clReader->SetFileName(inputFileName);

  WriterType::Pointer clWriter = WriterType::New();
  clWriter->SetInput(clReader->GetOutput());
  clWriter->SetFileName(fileName);

  try
  {
    clWriter->Update();
  }
  catch (itk::ExceptionObject & e)
  {
    std::cerr << "Error: " << e << std::endl;
    return false;
  }

  return true;
}
