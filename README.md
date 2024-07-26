## ContentAwareImageRecoloring
 The source code of the paper “Palette-based Content-Aware Image Recoloring”

## Requirements
1. OS: Windows 10
2. Qt: 5.12.9
3. OpenCV: 4.1.2
4. IDE: Visual Studio 2019
5. OpenGL

## Usage
1. Click "RgbPalette_Recolor_GUI-1.sln" to compile
2. Click the button "Open Image & Semantic map" to load the input image and semantic features (see the directory "data", semantic feature data named as "xxx_feat.data"), then the original and recolored images will be shown on the right
3. Fill the palette size and Click the button "Extract Palette" to extract the color palette of the input image
4. Click the button "Calc. Weight" to calculate the mixing weights
5. Modify the palette colors to recolor the input images

## Semantic features extraction
1. We follow Yagiz Aksoy's code to extract the semantic features, please refer to [https://github.com/iyah4888/SIGGRAPH18SSS](https://github.com/iyah4888/SIGGRAPH18SSS) and replace the **main_hyper.py** with **SemanticExtractor.py**
2. Run **SemanticExtractor.py** to extract the semantic feature of an input image (could run with CPU)

## Contact info
Email Addr.: dzj@qhu.edu.cn
