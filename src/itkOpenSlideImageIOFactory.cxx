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
#include "itkOpenSlideImageIOFactory.h"
#include "itkCreateObjectFunction.h"
#include "itkOpenSlideImageIO.h"
#include "itkVersion.h"
#include "IOOpenSlideExport.h"

namespace itk
{
OpenSlideImageIOFactory::OpenSlideImageIOFactory()
{
  this->RegisterOverride(
    "itkImageIOBase", "itkOpenSlideImageIO", "OpenSlide Image IO", 1, CreateObjectFunction<OpenSlideImageIO>::New());
}

OpenSlideImageIOFactory::~OpenSlideImageIOFactory() {}

const char *
OpenSlideImageIOFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char *
OpenSlideImageIOFactory::GetDescription() const
{
  return "OpenSlide ImageIO Factory, allows the loading of OpenSlide images into insight";
}

// Undocumented API used to register during static initialization.
// DO NOT CALL DIRECTLY.

static bool OpenSlideImageIOFactoryHasBeenRegistered;

void IOOpenSlide_EXPORT
OpenSlideImageIOFactoryRegister__Private(void)
{
  if (!OpenSlideImageIOFactoryHasBeenRegistered)
  {
    OpenSlideImageIOFactoryHasBeenRegistered = true;
    OpenSlideImageIOFactory::RegisterOneFactory();
  }
}

} // end namespace itk
