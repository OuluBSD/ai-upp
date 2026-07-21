# ShaderTemplateMatcherGpu

This package is the GUI/OpenGL runtime probe for the headless
`ShaderTemplateMatcher` contract. It deliberately keeps synthetic input and
readback verification small; production frame sources remain outside this
probe. The program must report shader compile, dispatch, readback, and CPU
parity results to stdout and exit automatically.
