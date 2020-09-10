.PHONY: all

all: eur usa jpn installer

clean:
	$(MAKE) -C sploit clean
	$(MAKE) -C nitpic3d_installer clean
	rm -rf build

eur: build/nitpic3d_installer/eur/SAVEDATA
usa: build/nitpic3d_installer/usa/SAVEDATA
jpn: build/nitpic3d_installer/jpn/SAVEDATA

build/nitpic3d_installer/%/SAVEDATA: sploit/build/%/SAVEDATA
	mkdir -p $(dir $@)
	cp $< $@

sploit/build/eur/SAVEDATA: FLAGS := eur
sploit/build/usa/SAVEDATA: FLAGS := usa
sploit/build/jpn/SAVEDATA: FLAGS := jpn

sploit/build/%/SAVEDATA:
	$(MAKE) -C sploit $(FLAGS)

installer: installer_resources build/nitpic3d_installer/nitpic3d_installer.3dsx build/nitpic3d_installer/nitpic3d_installer.smdh

build/nitpic3d_installer/nitpic3d_installer.%: nitpic3d_installer/nitpic3d_installer.%
	mkdir -p $(dir $@)
	cp $< $@

nitpic3d_installer/nitpic3d_installer.%:
	$(MAKE) -C nitpic3d_installer

installer_resources: build/nitpic3d_installer
	cp installer_resources/* build/nitpic3d_installer

build/nitpic3d_installer:
	mkdir -p $@
