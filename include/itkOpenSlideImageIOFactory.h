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
#ifndef itkOpenSlideImageIOFactory_h
#define itkOpenSlideImageIOFactory_h

#include "itkObjectFactoryBase.h"
#include "itkImageIOBase.h"
#include "IOOpenSlideExport.h"

namespace itk
{
/** \class OpenSlideImageIOFactory
 *
 * \brief OpenSlide is a C library that provides a simple interface
 * to read whole-slide images (also known as virtual slides).
 *
 * \ingroup IOOpenSlide
 *
 */
class IOOpenSlide_EXPORT OpenSlideImageIOFactory : public ObjectFactoryBase
{
public:
  ITK_DISALLOW_COPY_AND_MOVE(OpenSlideImageIOFactory);

  /** Standard class type alias. */
  using Self = OpenSlideImageIOFactory;
  using Superclass = ObjectFactoryBase;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;

  /** Class methods used to interface with the registered factories. */
  virtual const char* GetITKSourceVersion() const;
  virtual const char* GetDescription() const;

  /** Method for class instantiation. */
  itkFactorylessNewMacro(Self);
  static OpenSlideImageIOFactory* FactoryNew() { return new OpenSlideImageIOFactory;}

  /** Run-time type information (and related methods). */
  itkOverrideGetNameOfClassMacro(OpenSlideImageIOFactory);

  /** Register one factory of this type  */
  static void RegisterOneFactory()
    {
    OpenSlideImageIOFactory::Pointer metaFactory = OpenSlideImageIOFactory::New();
    ObjectFactoryBase::RegisterFactory(metaFactory);
    }

protected:
  OpenSlideImageIOFactory();
  ~OpenSlideImageIOFactory();
};

} // end namespace itk

#endif
