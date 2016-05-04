#!/usr/bin/env python

#==========================================================================
#
#   Copyright Insight Software Consortium
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#          http://www.apache.org/licenses/LICENSE-2.0.txt
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#
#==========================================================================*/

# This examples shows how to do streamed processing of an image read with the
# IOOpenSlide module to reduce memory consumption.

import sys

import itk

if len(sys.argv) != 4:
    print("Usage: " + sys.argv[0] + " <inputImage> <outputImage> <radius>")
    sys.exit(1)

inputImage = sys.argv[1]
outputImage = sys.argv[2]
radiusValue = int(sys.argv[3])

PixelType = itk.ctype('unsigned char')
Dimension = 2
ImageType = itk.Image[PixelType, Dimension]

reader = itk.ImageFileReader[ImageType].New(FileName=inputImage)
imageio = itk.OpenSlideImageIO.New()
reader.SetImageIO(imageio)

median = itk.MedianImageFilter[ImageType,ImageType].New(Input=reader.GetOutput())
median.SetRadius(radiusValue)

writer = itk.ImageFileWriter[ImageType].New(Input=median.GetOutput(),
    FileName=outputImage)
# Process the image in three chunks
writer.SetNumberOfStreamDivisions(3)
writer.Update()
