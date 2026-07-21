# ShaderTemplateMatcherGpu

This package is the GUI/OpenGL runtime probe for the headless
`ShaderTemplateMatcher` contract. It deliberately keeps synthetic input and
readback verification small; production frame sources remain outside this
probe. The program must report shader compile, dispatch, readback, and CPU
parity results to stdout and exit automatically.

The default run uses a deterministic synthetic fixture. A real
VisualStateModel fixture can be supplied without changing shader code:

    bin\\ShaderTemplateMatcherGpu.exe --fixture --frame <frame.vsm> \
        --crop-map <crop-map.vsm> --manifest <manifest.json>

The external frame and crop-map are converted to grayscale before upload. The
manifest and evidence contract remain shared with the headless matcher.
