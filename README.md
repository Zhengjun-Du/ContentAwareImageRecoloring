# ContentAwareImageRecoloring
 The source code of the paper “Palette-based Content-Aware Image Recoloring”

# Requirements
1. OS: Windows 10
2. Qt: 5.12.9
3. OpenCV: 4.1.2
4. IDE: Visual Studio 2019
5. OpenGL

# Usage
1. Click "RgbPalette_Recolor_GUI-1.sln" to compile
2. Click the button "Open Image & Semantic map" to load the input image and semantic features (see the directory "data", semantic feature data named as "xxx_feat.data"), then the original and recolored images will be shown on the right
3. Fill the palette size and Click the button "Extract Palette"
4. Click the button "Calc. Weight" to calculate the mixing weights
5. Modify the palette colors to recolor the input images

# Semantic features extraction
