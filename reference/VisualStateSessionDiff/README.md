# VisualStateSessionDiff

Console reference package demonstrating `VsmSessionDiff`.

Creates 2 synthetic session directories in temp, seeds each with
`divergences.json` files (one with 2 divergences: one shared, one unique to A;
the other with 2 divergences: the same shared one, and one unique to B), runs
the session diff comparator, and verifies the result has `only_in_a == 1`,
`only_in_b == 1`, and `in_both == 1`.

Run:

```sh
bin/build.exe -m 7 -j12 VisualStateSessionDiff
bin\VisualStateSessionDiff.exe
```

See `Manager/2-plan/ai-upp/root/VisualStateModel/docs/SESSION_DIFF.md` for design notes and API reference.
