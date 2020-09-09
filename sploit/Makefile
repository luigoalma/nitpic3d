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
eur: build/eur/SAVEDATA build/eur/nitpic3d.elf
usa: build/usa/SAVEDATA build/usa/nitpic3d.elf
jpn: build/jpn/SAVEDATA build/jpn/nitpic3d.elf

clean:
	rm -rf build

build/eur/nitpic3d.elf: FLAGS := -T nitpic3d_eur.ld -DEUR=1
build/usa/nitpic3d.elf: FLAGS := -T nitpic3d_usa.ld -DUSA=1
build/jpn/nitpic3d.elf: FLAGS := -T nitpic3d_jpn.ld -DJPN=1

build/%/nitpic3d.elf: nitpic3d.s
	mkdir -p $(dir $@)
	arm-none-eabi-gcc -x assembler-with-cpp -nostartfiles -nostdlib -g -o $@ $< -Wl,--use-blx $(FLAGS)

build/%/SAVEDATA: build/%/nitpic3d.elf
	arm-none-eabi-objcopy -O binary $< $@
