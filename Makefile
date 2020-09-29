.PHONY: all

all: eur usa jpn installer
eur: eur_de eur_en eur_es eur_fr eur_it
usa: usa_en usa_es usa_fr
jpn: jpn_ja

clean:
	$(MAKE) -C sploit clean
	$(MAKE) -C nitpic3d_installer clean
	rm -rf build

eur_de: build/nitpic3d_installer/eur/de/SAVEDATA
eur_en: build/nitpic3d_installer/eur/en/SAVEDATA
eur_es: build/nitpic3d_installer/eur/es/SAVEDATA
eur_fr: build/nitpic3d_installer/eur/fr/SAVEDATA
eur_it: build/nitpic3d_installer/eur/it/SAVEDATA

usa_en: build/nitpic3d_installer/usa/en/SAVEDATA
usa_es: build/nitpic3d_installer/usa/es/SAVEDATA
usa_fr: build/nitpic3d_installer/usa/fr/SAVEDATA

jpn_ja: build/nitpic3d_installer/jpn/ja/SAVEDATA

build/nitpic3d_installer/%/SAVEDATA: sploit/build/%/SAVEDATA
	mkdir -p $(dir $@)
	cp $< $@

sploit/build/eur/de/SAVEDATA: FLAGS := eur_de
sploit/build/eur/en/SAVEDATA: FLAGS := eur_en
sploit/build/eur/es/SAVEDATA: FLAGS := eur_es
sploit/build/eur/fr/SAVEDATA: FLAGS := eur_fr
sploit/build/eur/it/SAVEDATA: FLAGS := eur_it

sploit/build/usa/en/SAVEDATA: FLAGS := usa_en
sploit/build/usa/es/SAVEDATA: FLAGS := usa_es
sploit/build/usa/fr/SAVEDATA: FLAGS := usa_fr

sploit/build/jpn/ja/SAVEDATA: FLAGS := jpn_ja


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
