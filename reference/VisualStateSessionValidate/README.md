# VisualStateSessionValidate

Console reference package demonstrating `VsmSessionValidator`.

Builds two synthetic sessions:
1. A valid session with 3 frames and 1 crop
2. A deliberately broken session with a missing frame asset file

Then validates both:
- The valid session should pass validation
- The broken session should report validation errors (specifically a missing asset file)

Checks tested:
- manifest.json exists and is parseable
- All referenced frame asset files exist
- All referenced crop asset files exist
- Frame indexes are unique
- session_id is non-empty

Run:

```sh
bin/build.exe -m 7 -j12 VisualStateSessionValidate
bin/VisualStateSessionValidate.exe
```

See `Manager/2-plan/ai-upp/root/VisualStateModel/docs/SESSION_VALIDATOR.md` for the session validator design notes.
