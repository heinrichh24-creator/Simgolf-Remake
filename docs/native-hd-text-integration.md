# Native text replay: fork-specific integration contract

The fork is **not** at the same renderer revision as upstream. Apply changes to this fork's own sources; do not paste upstream line-number patches.

## Source audit

- `port/src/port/gdi.c`: `gdi_hd_text(frame, out, cap, font_ready)` copies the screen, prunes records, restores underlying text pixels, and serializes records in **newest-first** order. The browser renders these in reverse order. A native implementation must do the same.
- `port/src/port/hd_native.c`: `hd_present(screen)` publishes `g_frame` (course snapshot when layered) and `g_ui` (full screen when layered). `upload()` copies these under `g_lock`.
- `fill(... FILL_OVERLAY ...)` constructs a difference layer: pixels from `local_ui` are visible only when different from `local`. This is a critical caveat: **simply replacing UI pixels with cleaned background can expose/erase terrain incorrectly**, depending on the comparison and masks.

## Safe staging

**Stage A: instrumentation (current branch).** Counters for `TextOutA` routes and intact screen records. Diagnostics are opt-in.

**Stage B: native glyph renderer without pixel erasure.** Replay glyphs over unchanged native frame as a debug-only comparison. Because original glyphs remain underneath, this is not a production visual improvement. It is a geometry/font/color test. No renderer should claim crisp text from this stage.

**Stage C: synchronized cleanup + replay.** On the game thread, only for runs whose font is already cached/ready, build a cleaned screen and serialized glyphs together. Publish both under `g_lock` with one frame serial. Use cleaned image for non-layered `g_frame`; for layered rendering, use cleaned image as `g_ui` and ensure the diff-mask/terrain behavior remains correct. Snapshot the text data to local render-thread storage in `upload()`.

**Stage D: compositing.** Draw text after the classic UI pixels but before native overlays that must cover the text. Reproduce the original clip rectangle and reverse the newest-first record sequence. Respect canvas-to-output transforms, including tall canvases, scaling and letterboxing. When a font/run cannot be rendered, retain the original glyph pixels.

## Safety/quality gates

- Do not call `gdi_hd_text` on the render thread.
- Do not erase a glyph before the replacement font has been accepted by the native renderer.
- Bound serialized runs and allocations; handle malformed/overfull lists by leaving original text intact.
- Confirm `gdi_hd_text` assumes a contiguous screen buffer (`frame[y * d->w + x]`), so verify stride and canvas dimensions before enabling cleanup.
- Do not change `g_snap`, the terrain mask, gameplay, input, or save data.
- Compare screenshots of original vs replay at native resolution, 1080p and 4K; include menu, construction tray, tooltip, modal, and overlapped text.
- Keep `SIMGOLF_HD_TEXT_DIAG` off by default.

**Status:** design contract only; the native renderer and Windows build are not yet implemented/tested.
