import cv2
import numpy as np
import sys
import os
import math

size = os.path.getsize(sys.argv[1])
if size > 500000:
    size = 500000
square = math.floor(math.sqrt(size))
img = np.fromfile(open(sys.argv[1], 'rb'), np.uint8, square*square)

img = np.reshape(img, (square,square))
print(np.min(img),np.max(img),np.mean(img))
cv2.imwrite(sys.argv[1]+".png", img)
cv2.imshow("res",img)
cv2.waitKey(0)
