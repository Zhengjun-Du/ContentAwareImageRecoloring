import cv2
import numpy as np

def SaveFeatures(h,w,data,path=''):
    data = data[:,:3]
    col_min = np.min(data, axis=0)
    col_max = np.max(data, axis=0)

    image = np.zeros((h*w,3))
    image[:,0] = (data[:,0] - col_min[0]) / (col_max[0]-col_min[0])
    image[:,1] = (data[:,1] - col_min[1]) / (col_max[1]-col_min[1])
    image[:,2] = (data[:,2] - col_min[2]) / (col_max[0]-col_min[2])
    image = image.reshape(h,w,3)*255

    #cv2.imshow("feature_image", image)
    #cv2.waitKey(0)

    cv2.imwrite(path,image)


if __name__ == "__main__":
    data = np.random.randint(20,size=7500).reshape(2500,3)
    print(data)
    SaveFeatures(50,50,data)