# nitpic3d Installer

The installer will load a nitpic3d save and otherapp and install them to `Picross3D: Round 2` in digital or card.

It can install for the three regions of the game: `eur`, `usa` and `jpn`

## SD card placement

The installer's folder itself, containing the `3dsx`, `smdh` and the region folders, should be copied to the `sdmc:/3ds/` instead of the individual contents.

For each region folder there should be language folders, each containing a `SAVEDATA` targetting that region and language. The desired `otherapp.bin` should be placed inside the region folder, not inside any of the language ones.

If any `SAVEDATA` is missing or invalid for a language or region, installation for that language is disabled for that region. If all languages are disabled, installation to that region is disabled.

If `otherapp.bin` is missing for a region, installation is disabled for that region. It may also disable its usage if it's too big to fit into the `SAVEDATA` file.

## Controls

- Press X to install to the cart version of the game.
- Press Y to install to the selected digital version of the game.
- Press either X or Y while holding L to format the save archive before installing save.
- Left and right on DPad to choose between regions
- UP and down on DPad to choose between languages

## Installation notes

Once installed for one language, the game must run always in that language. If you which to use another language, reinstall.
