import sys
import itk

image = itk.imread(sys.argv[1])
itk.imwrite(image, sys.argv[2])
