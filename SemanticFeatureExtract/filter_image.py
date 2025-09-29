import cv2
import numpy as np

def FilterImage(feature, gray_img):
    c = feature.shape[-1]
    n1 = c - c % 3

    for i in range(0,n1,3):
        feature[:,:,i:i+3] = cv2.ximgproc.guidedFilter(gray_img, feature[:,:,i:i+3], 10, 2, -1)

    for i in range(n1,c):
        feature[:,:,i] = cv2.ximgproc.guidedFilter(gray_img, feature[:,:,i], 10, 2, -1)

    return feature

if __name__ == "__main__":
    img_path = 'E:/2023/ContentAwareRecoloring/Data/image/apple.png'
    feature = cv2.imread(img_path)
    gimg = cv2.imread(img_path)
    gimg = cv2.cvtColor(gimg,cv2.COLOR_RGB2GRAY)
    xx = FilterImage(feature, gimg)

    cv2.imshow("feature_image", xx)
    cv2.waitKey(0)