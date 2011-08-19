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
#ifndef __itkOpenSlideImageIOFactory_h
#define __itkOpenSlideImageIOFactory_h

#include "itkObjectFactoryBase.h"
#include "itkImageIOBase.h"

namespace itk
{
/** \class OpenSlideImageIOFactory
 *
 * \brief OpenSlide is a C library that provides a simple interface
 * to read whole-slide images (also known as virtual slides).
 *
 */
class ITK_EXPORT OpenSlideImageIOFactory : public ObjectFactoryBase
{
public:
  /** Standard class typedefs. */
  typedef OpenSlideImageIOFactory  Self;
  typedef ObjectFactoryBase        Superclass;
  typedef SmartPointer<Self>       Pointer;
  typedef SmartPointer<const Self> ConstPointer;

  /** Class methods used to interface with the registered factories. */
  virtual const char* GetITKSourceVersion() const;
  virtual const char* GetDescription() const;

  /** Method for class instantiation. */
  itkFactorylessNewMacro(Self);
  static OpenSlideImageIOFactory* FactoryNew() { return new OpenSlideImageIOFactory;}

  /** Run-time type information (and related methods). */
  itkTypeMacro(OpenSlideImageIOFactory, ObjectFactoryBase);

  /** Register one factory of this type  */
  static void RegisterOneFactory()
    {
    OpenSlideImageIOFactory::Pointer metaFactory = OpenSlideImageIOFactory::New();
    ObjectFactoryBase::RegisterFactory(metaFactory);
    }

protected:
  OpenSlideImageIOFactory();
  ~OpenSlideImageIOFactory();

private:
  OpenSlideImageIOFactory(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

};

} // end namespace itk

#endif
