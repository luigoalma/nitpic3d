#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define C_DEBUG CONSOLE_YELLOW
#define C_DEFAULT CONSOLE_WHITE
#define C_TITLE CONSOLE_CYAN
#define C_GOOD CONSOLE_GREEN
#define C_BAD CONSOLE_RED

#if DEBUG
#define debugmsgprint(...) printf(__VA_ARGS__)
#else
#define debugmsgprint(...) (void)0
#endif

typedef struct {
	u32 otherapp_file_offset;
	u32 otherapp_limit_size;
} otherapp_boundaries;

typedef struct {
	u32 magicvar;
	u32 magicsize;
	char sploitmagic[8];
	u32 otherapp_file_offset;
	u32 otherapp_limit_size;
} __attribute__((packed)) SploitSaveHeader;

typedef struct {
	bool save;
	bool otherapp;
} SaveStatus;

#define SaveStatus_IsGood(x) ((x).save && (x).otherapp)

// going lazy mode, and providing myself premade strings, rather than snprintf them

static const char* const SavePaths[3] = {
	"./eur/SAVEDATA",
	"./usa/SAVEDATA",
	"./jpn/SAVEDATA"
};

static const char* const OtherappPaths[3] = {
	"./eur/otherapp.bin",
	"./usa/otherapp.bin",
	"./jpn/otherapp.bin"
};

static const char* const sploitheaders[3] = {
	"NP3DEUR",
	"NP3DUSA",
	"NP3DJPN"
};

static const char* const regions[3] = {
	"EUR",
	"USA",
	"JPN"
};

static const u64 GameTitleIDs[3] = {
	0x0004000000187E00LLU,
	0x0004000000187D00LLU,
	0x0004000000169A00LLU
};

static bool CheckSAVE(int region, otherapp_boundaries* boundaries) {
	FILE* fp = fopen(SavePaths[region], "rb");
	if (!fp) return false;

	if (fseek(fp, 0, SEEK_END)) {
		fclose(fp);
		return false;
	}

	if (ftell(fp) != 0xb278) {
		fclose(fp);
		return false;
	}

	rewind(fp);

	SploitSaveHeader header_data;

	if (fread(&header_data, 1, sizeof(header_data), fp) < sizeof(header_data)) {
		fclose(fp);
		return false;
	}

	fclose(fp);

	// this does not guarantee finding all foul data, we dont know ROP boundaries
	if (header_data.magicvar != 0x1000d00 || header_data.magicsize != 0xb278 ||
	  strcmp(header_data.sploitmagic, sploitheaders[region]) ||
	  header_data.otherapp_file_offset < sizeof(header_data) || header_data.otherapp_file_offset >= 0xb270 ||
	  header_data.otherapp_file_offset + header_data.otherapp_limit_size > 0xb270) {
		return false;
	}

	boundaries->otherapp_file_offset = header_data.otherapp_file_offset;
	boundaries->otherapp_limit_size = header_data.otherapp_limit_size;
	return true;
}

static bool CheckOtherapp(int region, const otherapp_boundaries* boundaries) {
	FILE* fp = fopen(OtherappPaths[region], "rb");
	if (!fp) return false;

	if (fseek(fp, 0, SEEK_END)) {
		fclose(fp);
		return false;
	}

	if (ftell(fp) > boundaries->otherapp_limit_size) {
		fclose(fp);
		return false;
	}

	return true;
}

static u8* LoadPreparedSave(int region) {
	FILE* fp = fopen(SavePaths[region], "rb");
	if (!fp) return NULL;

	u8* save = malloc(0xb278);
	if (!save) {
		fclose(fp);
		return NULL;
	}

	if (fread(save, 1, 0xb278, fp) != 0xb278) {
		fclose(fp);
		free(save);
		return NULL;
	}

	fclose(fp);

	const SploitSaveHeader* header = (SploitSaveHeader*)save;

	fp = fopen(OtherappPaths[region], "rb");
	if (!fp) {
		free(save);
		return NULL;
	}

	if (!fread(&save[header->otherapp_file_offset], 1, header->otherapp_limit_size, fp)) {
		fclose(fp);
		free(save);
		return NULL;
	}

	fclose(fp);

	return save;
}

static Result FormatSave(u64 tid, FS_MediaType media) {
	u32 gamePath[3] = {media, tid & 0xffffffff, (tid >> 32) & 0xffffffff};
	FS_Path FSgamePath = {PATH_BINARY, sizeof(gamePath), gamePath};
	// game's default format settings
	Result res = FSUSER_FormatSaveData(ARCHIVE_USER_SAVEDATA, FSgamePath, 512, 1, 1, 3, 3, true);
	debugmsgprint(C_DEBUG "[DEBUG] Format Result %08lX\n", res);
	return res;
}

static Result WriteSaveFile(const u8* save, u64 tid, FS_MediaType media) {
	Handle file;
	Result res;

	if (media == MEDIATYPE_SD) {
		u64 in = ((u64)SECUREVALUE_SLOT_SD << 32) | (tid & 0xffffffff);
		u8 out = 0;
		res = FSUSER_ControlSecureSave(SECURESAVE_ACTION_DELETE, &in, sizeof(in), &out, sizeof(out));
		debugmsgprint(C_DEBUG "[DEBUG] Secure delete %08lX\n", res);
		if (R_FAILED(res)) return res;
	}

	u32 gamePath[3] = {media, tid & 0xffffffff, (tid >> 32) & 0xffffffff};
	FS_Path FSgamePath = {PATH_BINARY, sizeof(gamePath), gamePath};
	static const u16 UTF16_SAVE_PATH[] = {'/', 'S', 'A', 'V', 'E', 'D', 'A', 'T', 'A', 0};
	FS_Path FSsavePath = {PATH_UTF16, sizeof(UTF16_SAVE_PATH), UTF16_SAVE_PATH};

	FS_Archive gameArchive;

	res = FSUSER_OpenArchive(&gameArchive, ARCHIVE_USER_SAVEDATA, FSgamePath);
	debugmsgprint(C_DEBUG "[DEBUG] Open Archive %08lX\n", res);
	if (R_FAILED(res)) return res;

	res = FSUSER_OpenFile(&file, gameArchive, FSsavePath, FS_OPEN_WRITE | FS_OPEN_CREATE, 0);
	debugmsgprint(C_DEBUG "[DEBUG] Open file %08lX\n", res);
	if (R_FAILED(res)) {
		Result res2 = FSUSER_CloseArchive(gameArchive);
		(void)res2;
		debugmsgprint(C_DEBUG "[DEBUG] Close Archive %08lX\n", res2);
		return res;
	}

	res = FSFILE_SetSize(file, 0xb278);
	debugmsgprint(C_DEBUG "[DEBUG] Set size %08lX\n", res);
	if (R_FAILED(res)) {
		Result res2 = FSFILE_Close(file);
		if (R_FAILED(res2)) svcCloseHandle(file); // close handle at least anyway, cause idk what else I could do here
		debugmsgprint(C_DEBUG "[DEBUG] File close %08lX\n", res);
		res2 = FSUSER_CloseArchive(gameArchive);
		debugmsgprint(C_DEBUG "[DEBUG] Close Archive %08lX\n", res2);
		return res;
	}

	u32 written;
	Result writeres = FSFILE_Write(file, &written, 0LLU, save, 0xb278, FS_WRITE_FLUSH);
	debugmsgprint(C_DEBUG "[DEBUG] File write %08lX\n", writeres);
	res = FSFILE_Close(file);
	if (R_FAILED(res)) svcCloseHandle(file); // close handle at least anyway, cause idk what else I could do here
	debugmsgprint(C_DEBUG "[DEBUG] File close %08lX\n", res);

	// pick and choose at error to return if any
	if (R_FAILED(writeres)) res = writeres;
	if (written != 0xb278 && R_SUCCEEDED(res)) {
		res = MAKERESULT(RL_FATAL, RS_CANCELED, RM_APPLICATION, RD_INVALID_SIZE);
		debugmsgprint(C_DEBUG "[DEBUG] Bad written size\n");
	}
	if (R_FAILED(res)) {
		Result res2 = FSUSER_CloseArchive(gameArchive);
		(void)res2;
		debugmsgprint(C_DEBUG "[DEBUG] Close Archive %08lX\n", res2);
		return res;
	}

	u8 in, out;
	Result commitres = FSUSER_ControlArchive(gameArchive, ARCHIVE_ACTION_COMMIT_SAVE_DATA, &in, sizeof(in), &out, sizeof(out));
	debugmsgprint(C_DEBUG "[DEBUG] Commit Archive %08lX\n", res);

	res = FSUSER_CloseArchive(gameArchive);
	debugmsgprint(C_DEBUG "[DEBUG] Close Archive %08lX\n", res);

	if (R_FAILED(writeres)) res = commitres;
	if (R_FAILED(res)) return res;

	return 0;
}

static void PressAToContinue() {
	printf("Press A to continue\n");
	while (aptMainLoop())
	{
		hidScanInput();
		if (hidKeysDown() & KEY_A) break;
		gfxFlushBuffers();
		gfxSwapBuffers();
		gspWaitForVBlank();
	}
}

static void UpdateDigitalTarget(int* digital_target, bool is_digital_available, bool digital_availability[3], SaveStatus save_status[3], bool step_forward) {
	consoleClear();
	if (!is_digital_available) return;
	for (int i = 0; i < 3; ++i) {
		int index = *digital_target;
		if (digital_availability[index]) {
			*digital_target = index;
			break;
		}
		step_forward ? (++index) : (--index);
		if (index < 0) index = 2;
		else if (index > 2) index = 0;
		*digital_target = index;
	}
	printf(C_DEFAULT "Digital target: %s\n", regions[*digital_target]);
}

static void InstallToDigitalTarget(int target, bool formatsave) {
	consoleClear();

	do {
		Result res;
		u8* save = LoadPreparedSave(target);
		if (!save) {
			printf(C_BAD "Failed to load save to memory.\n");
			break;
		}

		if (formatsave) {
			puts(C_DEFAULT "Formating save...");
			res = FormatSave(GameTitleIDs[target], MEDIATYPE_SD);
			if (R_FAILED(res)) {
				printf(C_DEFAULT "Save format fail: " C_BAD "0x%08lX\n", res);
				free(save);
				break;
			} else {
				puts(C_GOOD "Successfully formatted save.");
			}
		}

		res = WriteSaveFile(save, GameTitleIDs[target], MEDIATYPE_SD);
		free(save);

		if (R_FAILED(res)) {
			printf(C_DEFAULT "Failed to write save: " C_BAD "0x%08lX\n", res);
			break;
		}

		printf(C_GOOD "Save installed!\n");
	} while(false);

	puts(C_DEFAULT);
	PressAToContinue();
}

static void InstallToCart(SaveStatus save_status[3], bool formatsave) {
	consoleClear();

	do {
		Result res;

		bool hascard;
		res = FSUSER_CardSlotIsInserted(&hascard);
		if (R_FAILED(res)) {
			printf(C_DEFAULT "Failed to check card: " C_BAD "0x%08lX.\n", res);
			break;
		}

		if (!hascard) {
			printf(C_BAD "Card slot has no card.\n");
			break;
		}

		FS_CardType type;
		res = FSUSER_GetCardType(&type);
		if (R_FAILED(res)) {
			printf(C_DEFAULT "Couldn't get card type: " C_BAD "0x%08lX.\n", res);
			break;
		}

		if (type != CARD_CTR) {
			printf(C_BAD "Card is not a 3DS card!\n");
			break;
		}

		u64 titleid = 0;
		u32 count;
		res = AM_GetTitleList(&count, MEDIATYPE_GAME_CARD, 1, &titleid);
		if (R_FAILED(res)) {
			printf(C_DEFAULT "Couldn't get card title id: " C_BAD "0x%08lX.\n", res);
			break;
		}

		int target;

		for (target = 0; target < 3; ++target) {
			if (GameTitleIDs[target] == titleid) break;
		}

		if (target == 3) {
			printf(C_DEFAULT "Card not Picross3d! " C_BAD "0x%016llX\n", titleid);
			break;
		}

		printf(C_DEFAULT "Game's region: " C_GOOD " %s\n", regions[target]);

		if (!SaveStatus_IsGood(save_status[target])) {
			printf(C_BAD "Save for this region not is ready!");
			break;
		}

		u8* save = LoadPreparedSave(target);
		if (!save) {
			printf(C_BAD "Failed to load save to memory.\n");
			break;
		}

		if (formatsave) {
			puts(C_DEFAULT "Formating save...");
			res = FormatSave(GameTitleIDs[target], MEDIATYPE_GAME_CARD);
			if (R_FAILED(res)) {
				printf(C_DEFAULT "Save format fail: " C_BAD "0x%08lX\n", res);
				free(save);
				break;
			} else {
				puts(C_GOOD "Successfully formatted save.");
			}
		}

		res = WriteSaveFile(save, GameTitleIDs[target], MEDIATYPE_GAME_CARD);
		free(save);

		if (R_FAILED(res)) {
			printf(C_DEFAULT "Failed to write save: " C_BAD "0x%08lX\n", res);
			break;
		}

		printf(C_GOOD "Save installed!\n");
	} while(false);

	puts(C_DEFAULT);
	PressAToContinue();
}

static void PrintControls(bool can_install, bool has_digital) {
	if (can_install) {
		puts(C_DEFAULT "X - Cart save install");
		if (has_digital) {
			puts(C_DEFAULT "Y - Digital save install");
		} else {
			puts(C_BAD "Digital install disabled");
		}
		puts(C_DEFAULT "Hold L to also format save");
	} else {
		puts(C_BAD "Installs disabled, no saves ready");
	}
	puts(C_DEFAULT "Start - Exit");
}

int main() {
	gfxInitDefault();
	amInit();
	fsInit();

	PrintConsole topScreen, bottomScreen;

	consoleInit(GFX_TOP, &topScreen);
	consoleInit(GFX_BOTTOM, &bottomScreen);

	consoleSelect(&topScreen);

	puts(C_TITLE " == nitpic3d Installer ==\n");

	printf(C_DEFAULT "SAVEFILE status:\n");

	SaveStatus save_status[3] = {{false, false}, {false, false}, {false, false}};
	bool digital_availability[3] = {false, false, false};
	bool is_digital_available = false;
	bool has_ready_saves = false;

	for (int i = 0; i < 3; ++i) {
		printf(C_DEFAULT " - %s: ", regions[i]);
		otherapp_boundaries save_otherapp_boundaries;
		save_status[i].save = CheckSAVE(i, &save_otherapp_boundaries);
		if (save_status[i].save) {
			save_status[i].otherapp = CheckOtherapp(i, &save_otherapp_boundaries);
			printf(C_GOOD "Good");
			if (save_status[i].otherapp) {
				has_ready_saves = true;
				puts("!");
			} else {
				puts(C_BAD ", but otherapp missing or too big.");
			}
		} else {
			puts(C_BAD "Missing or invalid");
		}
	}
	puts("");

	do {
		u64* installedtitles = NULL;

		u32 titlecount;
		Result res = AM_GetTitleCount(MEDIATYPE_SD, &titlecount);
		if (R_FAILED(res)) {
			printf(C_BAD "AM error counting SD titles: 0x%08lX\n", res);
			break;
		}
		installedtitles = calloc(titlecount, 8);
		if (!installedtitles) {
			puts(C_BAD "No memory to list SD titles.");
			break;
		}
		res = AM_GetTitleList(&titlecount, MEDIATYPE_SD, titlecount, installedtitles);
		if (R_FAILED(res)) {
			printf(C_BAD "AM error listing SD titles: 0x%08lX\n", res);
			free(installedtitles);
			break;
		}
		for (u32 i = 0, found = 0; i < titlecount; ++i) {
			for (int j = 0; j < 3; ++j) {
				if (installedtitles[i] == GameTitleIDs[j]) {
					digital_availability[j] = true;
					++found;
				}
			}
			if (found == 3) break;
		}

		printf(C_DEFAULT "Digital versions available:\n");
		for (int i = 0, printed = 0; i < 3; ++i) {
			if (digital_availability[i]) {
				printf(C_GOOD " - %s\n", regions[i]);
				++printed;
				if (!SaveStatus_IsGood(save_status[i])) {
					// disable, not available for installing save
					digital_availability[i] = false;
				} else {
					is_digital_available = true;
				}
			} else if (i == 2 && printed == 0) {
				puts(C_DEFAULT " - None");
			}
		}
		free(installedtitles);
	} while(false);

	puts("");
	PrintControls(has_ready_saves, is_digital_available);

	consoleSelect(&bottomScreen);

	int digital_target = 0;

	UpdateDigitalTarget(&digital_target, is_digital_available, digital_availability, save_status, true);

	gfxFlushBuffers();
	gfxSwapBuffers();
	gspWaitForVBlank();

	while (aptMainLoop())
	{
		hidScanInput();

		u32 kDown = hidKeysDown();
		u32 kHeld = hidKeysHeld();

		if (kDown & KEY_START) break;

		if (is_digital_available && kDown & KEY_DRIGHT) {
			if (++digital_target > 2) digital_target = 0;
			UpdateDigitalTarget(&digital_target, is_digital_available, digital_availability, save_status, true);
		}
		else if (is_digital_available && kDown & KEY_DLEFT) {
			if (--digital_target < 0) digital_target = 2;
			UpdateDigitalTarget(&digital_target, is_digital_available, digital_availability, save_status, false);
		}
		else if (is_digital_available && kDown & KEY_Y) {
			InstallToDigitalTarget(digital_target, (kHeld & KEY_L) != 0);
			UpdateDigitalTarget(&digital_target, is_digital_available, digital_availability, save_status, true);
		}
		else if (has_ready_saves && kDown & KEY_X) {
			InstallToCart(save_status, (kHeld & KEY_L) != 0);
			UpdateDigitalTarget(&digital_target, is_digital_available, digital_availability, save_status, true);
		}

		gfxFlushBuffers();
		gfxSwapBuffers();
		gspWaitForVBlank();
	}

	fsExit();
	amExit();
	gfxExit();
	return 0;
}
