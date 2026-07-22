# AmpTemplateAtlasPack

Headless runtime tool for packing externally supplied card-game template
images into one atlas and an `uppsrc/AMP` manifest. The source directory is
always supplied by command line; no dataset or provider path belongs here.

Build with `bin\build.exe`. The normal invocation is:

`bin\AmpTemplateAtlasPack.exe --pack <template-root> <atlas.png> <manifest.json> <report.htm>`

