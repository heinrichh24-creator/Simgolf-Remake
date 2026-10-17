# Native SimGolf HD text: investigation and Windows acceptance

Branch: investigation/native-hd-text-coverage. This is the original SimGolf
UI rendered crisply on native Windows, NOT a redesigned UI.

## Design

gdi.c tracks intact 16-bit TextOutA glyph records (font, em-size,
COLORREF, clip, baseline, bold/underline, original background pixels).
Missing 8-bit, external, and overwritten strings remain original.

hd_native.c copies TTF bytes on the game thread, validates and publishes
records, then caches per-glyph white alpha textures on the renderer thread.
Cache keys are font, quarter-pixel em-height, character and x/y subpixel
phase. Colors are applied to cached textures per run. Original GDI sizing
uses stbtt_ScaleForMappingEmToPixels, NOT ScaleForPixelHeight.
GDI records arrive newest-first but must be replayed oldest-first.
The browser gdi_hd_text interface is preserved.

## Runtime modes

* Default (no flags): the original pixels, no experimental glyph replay.
* SIMGOLF_HD_TEXT_DRAW=1: overlay original fonts on top of existing glyphs,
  to compare size/geometry. Double lettering is EXPECTED in this mode.
* SIMGOLF_HD_TEXT_REPLACE=1: EXPERIMENTAL restoration of GDI-recorded
  original text pixels, followed by native glyph replay. Text that cannot
  be replayed remains in the original framebuffer.
* SIMGOLF_HD_TEXT_DIAG=1: summary counters; never log private text.

Replacement stays opt-in until visual tests pass on Windows.

## Windows build prerequisite

Native CMake requires a 32-bit Windows clang-cl/MSVC toolchain, SDL2 SDK, and
the generated translator output build/gen. In particular, it checks for
build/gen/golf/golf_protos.h, compiles build/gen/*/*.c, and uses the generated
build/gen/images/golf.bin and jgld.bin. Compiled golf.exe, jgld.dll, jgld.bin,
and jgld-nops.json do NOT substitute for generated C sources.

To regenerate build/gen, tools/translate_gen.sh also requires prior Ghidra
exports under build/ghidra_work/port, build/ghidra_work/port_jgld and
their corresponding externals maps. Never claim build/gen exists based on
the compiled binaries alone. On Windows with prerequisites installed,
run tools/build_native.sh and use a separate test install; do not overwrite
a working game executable.

## Portable checks

Run: sh port/tests/hd_text_wire_test.sh

Covers snapshot lengths and floating-point input validation, plus em-based
original-font glyph geometry and x/y subpixel rasters under AddressSanitizer
and UndefinedBehaviorSanitizer. GitHub Actions has a workflow to repeat
these checks. Portable tests CANNOT prove correct Windows SDL behavior.

## Verified Windows compiler checkpoint (no full game link)

GitHub Actions run 39012710813 compiled both port/src/port/gdi.c and
port/src/port/hd_native.c to Windows x86 object files using clang-cl, the
Windows SDK and official SDL2 2.32.10 VC development headers. Both the
windows-syntax and sanitizer-regression jobs passed. This verifies the
modified units compile, but does NOT establish that the entire game links
or runs; missing build/gen remains the full native build prerequisite.
The user's earlier standalone golf.exe / Sim Golf Remastered.exe uploads
are identical prebuilt binaries, not a newly compiled test release.

## Windows acceptance matrix

1. Capture reference images in original mode: main menu, loading, new game,
   course HUD, hover/tooltips, scorecards, build dialogs, pause and options.
2. Set SIMGOLF_HD_TEXT_DRAW=1: compare native glyph geometry, color,
   baseline, bold and underline. Double text is diagnostic and expected.
3. Set SIMGOLF_HD_TEXT_REPLACE=1: verify NO double lettering, blank regions,
   missing labels or altered background pixels. Unsupported GDI paths
   must remain the original game's pixels.
4. Repeat at 800x600, 1080p, 1440p, 4K, ultrawide, resizing, multiple
   UI-scale levels, and both layered and non-layered output.
5. Test rapid screen transitions and overlapping labels, the glyph draw
   order, dialog clipping, changing colors and GPU texture cache lifetime.
6. Benchmark frame timing and texture allocations before and after replay.
7. Audit SIMGOLF_HD_TEXT_DIAG output and identify 8-bit raster/non-GDI
   text coverage gaps instead of claiming that every glyph is replaced.

Release gate: the full Windows native build must compile, and every
regression above must be verified on real Windows output before enabling
replacement by default. Preserve the original fallback behavior.
