# Native HD text coverage investigation

Target: original SimGolf interface in the Windows SDL port. This branch contains an investigation checklist, **not an HD text fix**.

## Verified source paths (upstream)

- `port/src/port/gdi.c`: `TextOutA` records glyph positions and original pixels in `TextRec` for the standard 16-bit path. `gdi_hd_text` returns a cleaned screen and glyph runs. `text_out_pj` returns early; 8-bit destinations do not create `TextRec` in this path.
- `port/src/port/hd.c` and `port/src/platform/hd_web.js`: browser text replay exists.
- `port/src/port/hd_native.c`: `hd_present` publishes game frames under `g_lock`; `upload` copies them to the presentation thread. Layered frames publish course pixels in `g_frame` and UI pixels in `g_ui`.
- Native compositor: `compose` renders course, UI, and native overlays; native replay must preserve this order.

## Before implementing

1. Confirm this fork's actual source revision against upstream: paths and code may differ.
2. Instrument counts of standard 16-bit TextOut records, `text_out_pj` calls, 8-bit calls, pruned records, and runs that reach the screen.
3. Capture a deterministic screen at the original 800x600 resolution and correlate labels with these counters. Do not infer coverage from counters alone.
4. Snapshot cleaned pixels **and** bounded glyph-run data on the game thread, and publish them under the existing lock with the same frame serial. Never call mutable GDI capture from the presentation thread.
5. Ensure the cleaned pixels replace the correct layer: `g_ui` for layered frames, `g_frame` for nonlayered frames. Course snapshots are separate and must remain intact.
6. Reuse the original font bytes, glyph advances, clipping, color, bold and underline metadata; use a native glyph rasterizer with cached textures.
7. Keep the original low-resolution text when fonts/runs are unavailable. Do not erase it unless the replay is guaranteed to render.
8. Test overlapping text, tooltips, construction tray, dialogs, menus, resize, 1080p, 4K, and repeated frames. Check that no doubled text appears.
9. Investigate JGLD-owned blits only for labels that remain uncaptured.

## Acceptance gates

- Zero game simulation or save-format changes.
- No visible text doubling, missing glyphs, or overlay-order regression.
- Same font identity and text placement as the classic UI, with sharper glyph edges at high output resolutions.
- Diagnostics identify which labels remain ineligible for replay.
- Windows build and screenshots are required before claiming success.

No runtime test has been performed by this document.
