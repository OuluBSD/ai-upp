# VideoChangedRegionClusterer

This package clusters changed-region events and candidate UI elements using two main mechanisms:
1. An Autoencoder neural network to extract low-dimensional (2D) feature representations of image crops, followed by K-Means clustering.
2. A connected-component spatial & text layout clustering to group OCR candidates within proximity on the screen.

Build using:
`bin\build.exe -m MSVS22x64 .\reference\VideoChangedRegionClusterer\VideoChangedRegionClusterer.upp`
