# nitpic3d Installer

The installer will load a nitpic3d save and otherapp and install them to `Picross3D: Round 2` in digital or card.

It can install for the three regions of the game: `eur`, `usa` and `jpn`

## SD card placement

The installer's folder itself, containing the `3dsx`, `smdh` and the region folders, should be copied to the `sdmc:/3ds/` instead of the individual contents.

For each region folder there should be `SAVEDATA` targetting that region and the desired `otherapp.bin`. If either are missing for a certain region, or nitpic3d `SAVEDATA` is not intended for the region it is placed in, installation will be disabled for that region. The installer may also refuse to install the save if `otherapp.bin` is too big to fit into the save, as indicated by the nitpic3d save headers.

## Controls

- Press X to install to the cart version of the game.
- Press Y to install to the selected digital version of the game.
- Press either X or Y while holding L to format the save archive before installing save.
