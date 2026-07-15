# VideoConvNetCppSampleExport Agent Notes

This package exports the changed-region manifest produced by task 0254 for a
thin ConvNetCpp-compatible consumer. It accepts `--candidates <file>`,
`--out-dir <dir>`, and optional `--review <file>`. It never predicts or
fabricates labels and does not require a model runtime.

Build with `bin\\build.exe -m MSVS22x64 --source-roots ".;../ConvNetCpp" .\\reference\\VideoConvNetCppSampleExport\\VideoConvNetCppSampleExport.upp`.

