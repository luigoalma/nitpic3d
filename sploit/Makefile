include $(DEVKITARM)/base_tools
#---------------------------------------------------------------------------------
# path to tools
#---------------------------------------------------------------------------------
export PATH  :=    $(DEVKITARM)/bin:$(PATH)
export CC    :=    gcc
#---------------------------------------------------------------------------------
# the prefix on the compiler executables
#---------------------------------------------------------------------------------
PREFIX       :=    arm-none-eabi-
#---------------------------------------------------------------------------------

.PHONY: all eur usa jpn

all: eur usa jpn
eur: eur_de eur_en eur_es eur_fr eur_it
usa: usa_en usa_es usa_fr
jpn: jpn_ja

eur_de: build/eur/de/SAVEDATA build/eur/de/nitpic3d.elf
eur_en: build/eur/en/SAVEDATA build/eur/en/nitpic3d.elf
eur_es: build/eur/es/SAVEDATA build/eur/es/nitpic3d.elf
eur_fr: build/eur/fr/SAVEDATA build/eur/fr/nitpic3d.elf
eur_it: build/eur/it/SAVEDATA build/eur/it/nitpic3d.elf

usa_en: build/usa/en/SAVEDATA build/usa/en/nitpic3d.elf
usa_es: build/usa/es/SAVEDATA build/usa/es/nitpic3d.elf
usa_fr: build/usa/fr/SAVEDATA build/usa/fr/nitpic3d.elf

jpn_ja: build/jpn/ja/SAVEDATA build/jpn/ja/nitpic3d.elf

clean:
	rm -rf build

build/eur/de/nitpic3d.elf: FLAGS := -Ttext=0x08883828 -DEUR=1 -DDE=1
build/eur/en/nitpic3d.elf: FLAGS := -Ttext=0x08882f98 -DEUR=1 -DEN=1
build/eur/es/nitpic3d.elf: FLAGS := -Ttext=0x08882f88 -DEUR=1 -DES=1
build/eur/fr/nitpic3d.elf: FLAGS := -Ttext=0x08883fd8 -DEUR=1 -DFR=1
build/eur/it/nitpic3d.elf: FLAGS := -Ttext=0x088834b8 -DEUR=1 -DIT=1
build/usa/en/nitpic3d.elf: FLAGS := -Ttext=0x08881c48 -DUSA=1 -DEN=1
build/usa/es/nitpic3d.elf: FLAGS := -Ttext=0x08881c58 -DUSA=1 -DES=1
build/usa/fr/nitpic3d.elf: FLAGS := -Ttext=0x08882768 -DUSA=1 -DFR=1
build/jpn/ja/nitpic3d.elf: FLAGS := -Ttext=0x0886ab78 -DJPN=1

build/%/nitpic3d.elf: nitpic3d.s
	mkdir -p $(dir $@)
	arm-none-eabi-gcc -x assembler-with-cpp -nostartfiles -nostdlib -g -Wl,--use-blx -T nitpic3d.ld $(FLAGS) -o $@ $<

build/%/SAVEDATA: build/%/nitpic3d.elf
	arm-none-eabi-objcopy -O binary $< $@
