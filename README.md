# nitpic3d

nitpic3d, a secondary 3DS userland exploit for `Picross 3D: Round 2` (Europe and USA) and `カタチ新発見！ 立体ピクロス２` (Japan).

## Exploit explanation

Summary: 
> Out of bounds array access allowing to point to fabricated objects and vtable.

Description: 
> Game only checks save header. With the last interacted save slot index at +0xb270 in the save data unchecked we can achieve a predictable out of bounds access, as well inserting ROP data without detecting save corruption. Game references an object from an array of 3 elements and passes it to a function that will read object pointers and hit a vtable call. With a copy save data left in memory and a properly calculated index, we can point to a fake object position in the save, vtable jump to a stack pivot and start the ROP chain.

## Building

Run `make` on an unix compatible terminal. Windows CMD is not usable.

* Need devkitPro's toolchain and environment values setup for 3ds building

## Installing

* Place the `nitpic3d_installer` itself from releases or your built output in `build/` and place it in the 3ds's SD card in `/3ds/`.
* After copying folder, place the desired `otherapp.bin` in the desired region folder inside `/3ds/nitpic3d_installer/`.
  * `otherapp.bin` can be obtained [here](https://smealum.github.io/3ds/), except for European consoles running version 11.10 or above, for that go [here](https://deadphoenix8091.github.io/3ds/#otherapp) instead. Select the desired system version exploit will be running on and download with `Download otherapp`.
  * Note: Do not put it on any of the language folder! Just on the region folder.
* Run it from another another homebrew entrypoint, or another homebrewed console if planing to install to cart version.
* Instructions on provided `README.md` inside `nitpic3d_installer`, plus simple control on screen when installer is running.

## Running the exploit

Just open the game, tap to enter the saves screen.

If you get the message `Welcome to the Picross 3D Café!` (English), `Willkommen im Picross 3D-Café!` (German), `¡Te doy la bienvenida a la cafetería de Picross 3D!` (Spanish), `Bienvenue au Café Picross 3D!` (French), `Ti do il benvenuto al Caffè Picross 3D!` (Italian) or `いらっしゃいませ。 立体ピクロス カフェへようこそ。` (Japanese) and with no save slots used, just tap again. If doesn't run, double check if you installed exploit properly.

Extra note: Once installed to a specific language, exploit will only run successfully if game runs on that language.

## Credits and special thanks

* Kartik for finding that the game is crashable with random data, letting me investigate and helping me search initial pivot points. Also testing completed exploit save in EUR New3DS. (And enduring my excitement at given moments during exploitation.)
* yellows8 for the the very handy [3ds_ropkit](https://github.com/yellows8/3ds_ropkit)
* Zoogie for helping with the 3ds_ropkit and finding stack pivot, as well helping me test out initial testing phase SAVEDATAs
* knight-ryu12 for testing completed exploit SAVE on JPN New3DS
* ihaveahax for testing on USA New3DS and Old3DS
* LunaDook for testing on JPN Old3DS and USA New3DS too
* Everyone I've may forgotten to mention that assisted and/or supported me
  * If I forgot someone, or some detail, tell me
