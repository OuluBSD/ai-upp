# VisualStateGroundTruthInit

Console reference package demonstrating `VsmGroundTruthTemplateGenerator`.

Creates a synthetic 3-frame session in a temp directory, runs the template
generator to produce a skeleton ground truth JSON file, loads the generated
file back to verify it parses correctly, and verifies that it contains exactly
3 frames and one example divergence entry with the expected message.

Run:

```sh
bin/build.exe -m 7 -j12 VisualStateGroundTruthInit
bin\VisualStateGroundTruthInit.exe
```

See `Manager/2-plan/ai-upp/root/VisualStateModel/docs/GROUND_TRUTH_TEMPLATE.md` for design notes and API reference.
