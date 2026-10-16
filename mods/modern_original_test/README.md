# Modern Original UI – version 0.2 (experimental)

This is a separate third course HUD style; it does not change Frosted Mint or the native 2002 Original HUD.

**Requires a rebuilt executable** containing the new HUD_MODERN_ORIGINAL renderer. The existing release and the original v0.1 prototype cannot render this mode merely by replacing a mod file.

## Using it
1. Close the game. Install this folder at `mods/modern_original_test` (not inside `mods/builtin`).
2. After updating to a compatible compiled game executable, enable "Modern Original (Experimental)" in Mods and restart.
3. Open a course. There are three large descending circular buttons for Course, Buildings and People. Their category trays only appear when opened.
4. To compare against the other styles, disable the experimental mod and restart.

## Revising geometry without building again
Edit `mod.toml` [settings] values `ui.mo_*` and restart. The positions use the player's 788x450 reference coordinate space, scaled from screen height: golf (42,312), building (121,338), people (177,406), diameter 70. `ui.mo_scale` scales all pieces, and the tray starts at x=216 and uses 80 units of height.

The existing Frosted Mint design tokens are reused by default. A local `ui/frosted_mint.toml` in this mod folder may override its glass and button colours, borders, type and animation in a later visual iteration.

## Limits
- The underlying course actions still call the original 2002 game routines.
- Changes to art icons, widget behavior or drawing algorithms require one more engine update.
- This PR was source-reviewed, not compiled or gameplay-tested in Windows.
