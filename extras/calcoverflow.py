#!/usr/bin/env python3

# This script was made to assist me get the overflow value I wanted
# Partially glued on together
# And not intended to be the most optimal
# I'll release it out just to show what values I are part of it

eur_language = ['DE', 'EN', 'ES', 'FR', 'IT']
usa_language = ['EN', 'ES', 'FR']

objectbases = [{'DE': 0x0889b384, 'EN': 0x0889aaf4, 'ES': 0x0889aae4, 'FR': 0x0889bb34, 'IT': 0x0889b014}, {'EN': 0x088997a4, 'ES': 0x088997b4, 'FR': 0x0889a2c4}, 0x08883620]
saveoffsets = [{'DE': 0x08883828, 'EN': 0x08882f98, 'ES': 0x08882f88, 'FR': 0x08883fd8, 'IT': 0x088834b8}, {'EN': 0x08881c48, 'ES': 0x08881c58, 'FR': 0x08882768}, 0x0886ab78]

def pick_language(languages, objectbases, saveoffsets):
	global objectbase
	global saveoffset
	while True:
		try:
			lang = input(f"Language ({'/'.join(languages)})? ")
			lang = lang.upper()
			objectbase = objectbases.get(lang, None)
			saveoffset = saveoffsets.get(lang, None)
			if not objectbase or not saveoffset:
				print("Bad language, retry.")
				continue
			break
		except KeyboardInterrupt:
			exit(0)
		except:
			print("Invalid value.")
			continue

while True:
	try:
		region = input("Region (EUR/USA/JPN)? ")
		region = region.lower()
		if region == 'eur':
			pick_language(eur_language, objectbases[0], saveoffsets[0])
		elif region == 'usa':
			pick_language(usa_language, objectbases[1], saveoffsets[1])
		elif region == 'jpn':
			objectbase = objectbases[2]
			saveoffset = saveoffsets[2]
		else:
			print("Bad region, retry.")
			continue
		break
	except KeyboardInterrupt:
		exit(0)
	except:
		print("Invalid value.")
		continue

while True:
	try:
		var = input("Input position of arbitrary pointer in save: ") # or as I called it, Object0
		if var[0] == '-':
			print("Expecting positive position.")
		if var[0] == '+':
			var = var[1:]
		int(var[0]) # a way to check if an int at start
		if var[0:2] == "0x":
			var = int(var[2:], 16)
		elif var[0:2] == "0o":
			var = int(var[2:], 8)
		elif var[0:2] == "0b":
			var = int(var[2:], 2)
		else:
			var = int(var)

		if var >= 0 and var < 0x18:
			print("Can't have pointer placed at header.")
			continue
		if var >= 0xb270 and var < 0xb274:
			print("Can't have pointer placed at save slot index.")
			continue
		if var % 4:
			print("Align the pointer by 4.")
			continue
		break
	except KeyboardInterrupt:
		exit(0)
	except:
		print("Invalid position value.")
		continue

found = False

var = saveoffset + var - 0x24 - 652 - objectbase
if var < 0:
    var = 0x100000000 + var

for i in range(0, 200 * 0x100000000, 0x100000000):
	testvar = i | var
	if testvar > (200*0xFFFFFFFF): # limit
		break
	if (testvar % 200) == 0:
		foundvalue = testvar // 200
		foundle = foundvalue.to_bytes(4, 'little')
		found = True
		break

if not found:
	print("No multiplier of 200 found to target this position.")
else:
	print(f"Found value: 0x{foundvalue:08X} ({foundle[0]:02X} {foundle[1]:02X} {foundle[2]:02X} {foundle[3]:02X})")

