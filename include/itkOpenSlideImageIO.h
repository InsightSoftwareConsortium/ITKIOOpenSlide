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
#ifndef __itkOpenSlideImageIO_h
#define __itkOpenSlideImageIO_h

#include "itkImageIOBase.h"

namespace itk
{

// Forward declare a wrapper class that is responsible for openslide_t (among other things)
class OpenSlideWrapper;

/** \class OpenSlideImageIO
 *
 * OpenSlide is a C library that provides a simple interface to read whole-slide
 * images (also known as virtual slides).  The following formats can be read:
 *
 * - Trestle (.tif),
 * - Hamamatsu (.vms, .vmu)
 * - Aperio (.svs, .tif)
 * - MIRAX (.mrxs)
 * - Generic tiled TIFF (.tif)
 *
 *  \ingroup IOFilters
 *
 *  \ingroup \ITKIOOpenSlide
 */
class ITK_EXPORT OpenSlideImageIO : public ImageIOBase
{
public:
  /** Standard class typedefs. */
  typedef OpenSlideImageIO   Self;
  typedef ImageIOBase        Superclass;
  typedef SmartPointer<Self> Pointer;
  typedef std::vector<std::string> AssociatedImageNameContainer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(OpenSlideImageIO, ImageIOBase);

  typedef Superclass::ArrayOfExtensionsType ArrayOfExtensionsType;

 /*-------- This part of the interfaces deals with reading data. ----- */

  /** Determine the file type. Returns true if this ImageIO can read the
   * file specified. */
  virtual bool CanReadFile(const char*);

  /** Determine if the ImageIO can stream reading from the
      current settings. Default is false. If this is queried after
      the header of the file has been read then it will indicate if
      that file can be streamed */
  virtual bool CanStreamRead();

  /** Set the spacing and dimension information for the set filename. */
  virtual void ReadImageInformation();

  /** Reads the data from disk into the memory buffer provided. */
  virtual void Read(void* buffer);

  /*-------- This part of the interfaces deals with writing data. ----- */

  /** Determine the file type. Returns true if this ImageIO can write the
   * file specified. */
  virtual bool CanWriteFile(const char*);

  /** Set the spacing and dimension information for the set filename. */
  virtual void WriteImageInformation();

  /** Writes the data to disk from the memory buffer provided. Make sure
   * that the IORegions has been set properly. */
  virtual void Write(const void* buffer);

/** Method for supporting streaming.  Given a requested region, determine what
 * could be the region that we can read from the file. This is called the
 * streamable region, which will be smaller than the LargestPossibleRegion and
 * greater or equal to the RequestedRegion */
  virtual ImageIORegion
  GenerateStreamableReadRegionFromRequestedRegion( const ImageIORegion & requested ) const;

/** Detect the vendor of the current file. */
  virtual std::string GetVendor() const;

/** Sets the level to read. Level 0 (default) is the highest resolution level.
 * This method overrides any previously selected associated image. 
 * Call ReadImageInformation() again after calling this function. */
  virtual void SetLevel(int iLevel);

/** Returns the currently selected level. */
  virtual int GetLevel() const;

/** Sets the associated image to extract.
 * This method overrides any previously selected level.
 * Call ReadImageInformation() again after calling this function. */
  virtual void SetAssociatedImageName(const std::string &strName);

/** Returns the currently selected associated image name (empty string if none). */
  virtual std::string GetAssociatedImageName() const;

/** Sets the best level to read for the given downsample factor.
 * This method overrides any previously selected associated image. 
 * Call ReadImageInformation() again after calling this function. */
  virtual bool SetLevelForDownsampleFactor(double dDownsampleFactor);

/** Returns all associated image names stored int he file. */
  virtual AssociatedImageNameContainer GetAssociatedImageNames() const;

protected:
  OpenSlideImageIO();
  ~OpenSlideImageIO();
  virtual void PrintSelf(std::ostream& os, Indent indent) const;

private:
  OpenSlideImageIO(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

  OpenSlideWrapper *m_p_clOpenSlideWrapper; // Opaque pointer to a wrapper that manages openslide_t
};

} // end namespace itk

#endif // __itkOpenSlideImageIO_h
