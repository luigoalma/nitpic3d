#include <3ds.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <memory>
#include <new>

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

static const FS_Path sdmcPath = {PATH_EMPTY, 1, ""};

typedef struct {
	u32 magicvar;
	u32 magicsize;
	char sploitmagic[8];
	u32 otherapp_file_offset;
	u32 otherapp_limit_size;
} __attribute__((packed)) SploitSaveHeader;

class RegionSaveStatus {
protected:
	static char BasePath[256];
	bool GotOtherapp;
	bool GotSave; // got at least in one language
	RegionSaveStatus() : GotOtherapp(false), GotSave(false) {}
	static bool CheckSAVE(const char* regionname, const char* language, const char* sploitheader, size_t otherapp_size);
	static bool CheckOtherapp(const char* regionname, size_t& otherapp_size);
	static u8* LoadPreparedSave(const char* regionname, const char* language);
public:
	bool IsAvailable() const {return GotOtherapp && GotSave;}
	virtual bool IsLanguageAvailable(int index) const = 0;
	virtual int GetLanguageCount() const = 0;
	virtual int GetAvailableLanguageCount() const = 0;
	virtual const char* GetLanguageStr(int index) const = 0;
	virtual const char* GetRegionStr() const = 0;
	virtual u64 GetTitleId() const = 0;
	virtual u8* LoadPreparedSave(int langindex) const = 0;

	static bool SetCWDBase();
};
char RegionSaveStatus::BasePath[256] = {0};

template<u64 TitleId, const char* const RegionName, const char* const* const SploitHeaders, const char* const* LanguageShortNames, const char* const* LanguageNames, int LanguageCount>
class SaveStatusTemplate : public RegionSaveStatus {
	u32 LanguageAvailability;
	u32 LanguageAvailableCount;
public:
	virtual bool IsLanguageAvailable(int index) const {
		if (index < 0 || index >= LanguageCount) return false;
		return (LanguageAvailability & BIT(index)) != 0;
	}
	virtual int GetLanguageCount() const {return LanguageCount;}
	virtual int GetAvailableLanguageCount() const {return LanguageAvailableCount;}
	virtual const char* GetLanguageStr(int index) const {
		if (index < 0 || index >= LanguageCount) return nullptr;
		return LanguageNames[index];
	}
	virtual const char* GetRegionStr() const {return RegionName;}
	virtual u64 GetTitleId() const {return TitleId;}
	virtual u8* LoadPreparedSave(int langindex) const {
		if (langindex < 0 || langindex >= LanguageCount) return nullptr;
		return RegionSaveStatus::LoadPreparedSave(RegionName, LanguageShortNames[langindex]);
	}
	SaveStatusTemplate() : RegionSaveStatus(), LanguageAvailability(0), LanguageAvailableCount(0) {
		size_t otherapp_size;
		GotOtherapp = CheckOtherapp(RegionName, otherapp_size);
		if (!GotOtherapp) return;
		for (int i = 0; i < LanguageCount; ++i) {
			if (CheckSAVE(RegionName, LanguageShortNames[i], SploitHeaders[i], otherapp_size)) {
				LanguageAvailability |= BIT(i);
				++LanguageAvailableCount;
			}
		}
		GotSave = LanguageAvailability != 0;
	}
};

const char EURRegionName[] = "eur";
const char* const EURSploitHeaders[5] = {"NP3DEDE", "NP3DEEN", "NP3DEES", "NP3DEFR", "NP3DEIT"};
const char* const EURLanguageShortNames[5] = {"de", "en", "es", "fr", "it"};
const char* const EURLanguageNames[5] = {"German", "English", "Spanish", "French", "Italian"};
const char USARegionName[] = "usa";
const char* const USASploitHeaders[3] = {"NP3DUEN", "NP3DUES", "NP3DUFR"};
const char* const USALanguageShortNames[3] = {"en", "es", "fr"};
const char* const USALanguageNames[3] = {"English", "Spanish", "French"};
const char JPNRegionName[] = "jpn";
const char* const JPNSploitHeaders[3] = {"NP3DJJA"};
const char* const JPNLanguageShortNames[3] = {"ja"};
const char* const JPNLanguageNames[3] = {"Japanese"};

typedef SaveStatusTemplate<0x0004000000187E00LLU, EURRegionName, EURSploitHeaders, EURLanguageShortNames, EURLanguageNames, 5> EURSaveStatus;
typedef SaveStatusTemplate<0x0004000000187D00LLU, USARegionName, USASploitHeaders, USALanguageShortNames, USALanguageNames, 3> USASaveStatus;
typedef SaveStatusTemplate<0x0004000000169A00LLU, JPNRegionName, JPNSploitHeaders, JPNLanguageShortNames, JPNLanguageNames, 1> JPNSaveStatus;

bool RegionSaveStatus::CheckSAVE(const char* regionname, const char* language, const char* sploitheader, size_t otherapp_size) {
	Handle filehandle;

	std::unique_ptr<char> path(new(std::nothrow) char[256]);
	if (!path) {
		return false;
	}

	auto path_len = snprintf(path.get(), 256, "%s/%s/%s/SAVEDATA", BasePath, regionname, language);
	if (path_len < 0 || path_len >= 256) {
		return false;
	}

	debugmsgprint(C_DEBUG "%s\n", path.get());

	FS_Path filePath = {PATH_ASCII, (u32)path_len+1, path.get()};
	Result res = FSUSER_OpenFileDirectly(&filehandle, ARCHIVE_SDMC, sdmcPath, filePath, FS_OPEN_READ, 0);
	if (R_FAILED(res)) {
		debugmsgprint(C_DEBUG "[DEBUG] Open file directly %08lX\n", res);
		return false;
	}

	u64 filesize;
	res = FSFILE_GetSize(filehandle, &filesize);

	if (R_FAILED(res) || filesize != 0xb278) {
		if(R_FAILED(res)) debugmsgprint(C_DEBUG "[DEBUG] Get Size %08lX\n", res);
		else debugmsgprint(C_DEBUG "[DEBUG] File size %llu\n", filesize);
		res = FSFILE_Close(filehandle);
		if (R_FAILED(res)) svcCloseHandle(filehandle);
		return false;
	}

	SploitSaveHeader header_data;

	u32 totalread;
	res = FSFILE_Read(filehandle, &totalread, 0LLU, &header_data, sizeof(header_data));

	if (R_FAILED(res) || totalread != sizeof(header_data)) {
		if(R_FAILED(res)) debugmsgprint(C_DEBUG "[DEBUG] File Read %08lX\n", res);
		else debugmsgprint(C_DEBUG "[DEBUG] Total read %lu\n", totalread);
		res = FSFILE_Close(filehandle);
		if (R_FAILED(res)) svcCloseHandle(filehandle);
		return false;
	}

	res = FSFILE_Close(filehandle);
	if (R_FAILED(res)) {
		debugmsgprint(C_DEBUG "[DEBUG] File close %08lX\n", res);
		svcCloseHandle(filehandle);
	}

	// this does not guarantee finding all foul data, we dont know ROP boundaries
	if (header_data.magicvar != 0x1000d00 || header_data.magicsize != 0xb278 ||
	  strcmp(header_data.sploitmagic, sploitheader) ||
	  header_data.otherapp_file_offset < sizeof(header_data) || header_data.otherapp_file_offset >= 0xb270 ||
	  header_data.otherapp_file_offset + header_data.otherapp_limit_size > 0xb270) {
		debugmsgprint(C_DEBUG "[DEBUG] Bad header\n");
		return false;
	}

	if (header_data.otherapp_limit_size < otherapp_size) {
		debugmsgprint(C_DEBUG "[DEBUG] Otherapp too big\n");
		return false;
	}

	return true;
}

bool RegionSaveStatus::CheckOtherapp(const char* regionname, size_t& otherapp_size) {
	Handle filehandle;

	std::unique_ptr<char> path(new(std::nothrow) char[256]);
	if (!path) {
		return false;
	}

	auto path_len = snprintf(path.get(), 256, "%s/%s/otherapp.bin", BasePath, regionname);
	if (path_len < 0 || path_len >= 256) {
		return false;
	}

	debugmsgprint(C_DEBUG "%s\n", path.get());

	FS_Path filePath = {PATH_ASCII, (u32)path_len+1, path.get()};
	Result res = FSUSER_OpenFileDirectly(&filehandle, ARCHIVE_SDMC, sdmcPath, filePath, FS_OPEN_READ, 0);
	if (R_FAILED(res)) {
		debugmsgprint(C_DEBUG "[DEBUG] Open file directly %08lX\n", res);
		return false;
	}

	u64 filesize;
	res = FSFILE_GetSize(filehandle, &filesize);

	if (R_FAILED(res)) {
		debugmsgprint(C_DEBUG "[DEBUG] Get Size %08lX\n", res);
		res = FSFILE_Close(filehandle);
		if (R_FAILED(res)) svcCloseHandle(filehandle);
		return false;
	}

	otherapp_size = (size_t)filesize;

	res = FSFILE_Close(filehandle);
	if (R_FAILED(res)) {
		debugmsgprint(C_DEBUG "[DEBUG] File close %08lX\n", res);
		svcCloseHandle(filehandle);
	}

	return true;
}

u8* RegionSaveStatus::LoadPreparedSave(const char* regionname, const char* language) {
	Handle filehandle;

	std::unique_ptr<char> savepath(new(std::nothrow) char[256]);
	std::unique_ptr<char> otherapppath(new(std::nothrow) char[256]);
	if (!savepath || !otherapppath) {
		return nullptr;
	}

	auto savepath_len = snprintf(savepath.get(), 256, "%s/%s/%s/SAVEDATA", BasePath, regionname, language);
	if (savepath_len < 0 || savepath_len >= 256) {
		return nullptr;
	}

	auto otherapppath_len = snprintf(otherapppath.get(), 256, "%s/%s/otherapp.bin", BasePath, regionname);
	if (otherapppath_len < 0 || otherapppath_len >= 256) {
		return nullptr;
	}

	FS_Path savefilePath = {PATH_ASCII, (u32)savepath_len+1, savepath.get()};
	FS_Path otherappfilePath = {PATH_ASCII, (u32)otherapppath_len+1, otherapppath.get()};

	Result res = FSUSER_OpenFileDirectly(&filehandle, ARCHIVE_SDMC, sdmcPath, savefilePath, FS_OPEN_READ, 0);
	if (R_FAILED(res)) {
		debugmsgprint(C_DEBUG "[DEBUG] Open file directly %08lX\n", res);
		return nullptr;
	}

	u8* save = (u8*)malloc(0xb278);
	if (!save) {
		res = FSFILE_Close(filehandle);
		if (R_FAILED(res)) svcCloseHandle(filehandle);
		return nullptr;
	}

	u32 totalread;
	res = FSFILE_Read(filehandle, &totalread, 0LLU, save, 0xb278);

	if (R_FAILED(res) || totalread != 0xb278) {
		if(R_FAILED(res)) debugmsgprint(C_DEBUG "[DEBUG] File Read %08lX\n", res);
		else debugmsgprint(C_DEBUG "[DEBUG] Total read %lu\n", totalread);
		res = FSFILE_Close(filehandle);
		if (R_FAILED(res)) svcCloseHandle(filehandle);
		free(save);
		return nullptr;
	}

	res = FSFILE_Close(filehandle);
	if (R_FAILED(res)) {
		debugmsgprint(C_DEBUG "[DEBUG] File close %08lX\n", res);
		svcCloseHandle(filehandle);
	}

	const SploitSaveHeader* header = (SploitSaveHeader*)save;

	res = FSUSER_OpenFileDirectly(&filehandle, ARCHIVE_SDMC, sdmcPath, otherappfilePath, FS_OPEN_READ, 0);
	if (R_FAILED(res)) {
		debugmsgprint(C_DEBUG "[DEBUG] Open file directly %08lX\n", res);
		free(save);
		return nullptr;
	}

	res = FSFILE_Read(filehandle, &totalread, 0LLU, &save[header->otherapp_file_offset], header->otherapp_limit_size);
	if (R_FAILED(res) || !totalread) {
		if(R_FAILED(res)) debugmsgprint(C_DEBUG "[DEBUG] File Read %08lX\n", res);
		else debugmsgprint(C_DEBUG "[DEBUG] Nothing read.");
		res = FSFILE_Close(filehandle);
		if (R_FAILED(res)) svcCloseHandle(filehandle);
		free(save);
		return nullptr;
	}

	res = FSFILE_Close(filehandle);
	if (R_FAILED(res)) {
		debugmsgprint(C_DEBUG "[DEBUG] File close %08lX\n", res);
		svcCloseHandle(filehandle);
	}

	return save;
}

bool RegionSaveStatus::SetCWDBase() {
	std::unique_ptr<char> _cwd(new(std::nothrow) char[256+9]);
	char* cwd = _cwd.get();
	if (!getcwd(cwd, 256+9))
		return false;
	char* start = strchr(cwd, '/'); // trying to truncate sdmc:
	if (!start)
		return false;
	// and any remove any / from end of path
	// if cwd == "/" then first '/' is also removed
	// since save and otherapp paths already preinclude a starting /
	for (int i = strlen(cwd)-1; &cwd[i] >= start && cwd[i] == '/'; --i) {
		cwd[i] = 0;
	}
	strncpy(BasePath, start, 256);
	BasePath[255] = 0;
	return true;
}

static Result FormatSave(u64 tid, FS_MediaType media) {
	u32 gamePath[3] = {media, (u32)(tid & 0xffffffff), (u32)((tid >> 32) & 0xffffffff)};
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

	u32 gamePath[3] = {media, (u32)(tid & 0xffffffff), (u32)((tid >> 32) & 0xffffffff)};
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
	printf(C_DEFAULT "Press A to continue\n");
	while (aptMainLoop())
	{
		hidScanInput();
		if (hidKeysDown() & KEY_A) break;
		gfxFlushBuffers();
		gfxSwapBuffers();
		gspWaitForVBlank();
	}
}

static void UpdateDigitalTarget(int& digital_target, bool digital_availability[3], const RegionSaveStatus* regionsaves[3], bool step_forward) {
	for (int i = 0; i < 3; ++i) {
		int index = digital_target;
		if (digital_availability[index] && regionsaves[index]->IsAvailable()) {
			digital_target = index;
			break;
		}
		step_forward ? (++index) : (--index);
		if (index < 0) index = 2;
		else if (index > 2) index = 0;
		digital_target = index;
	}
}

static void UpdateLinguisticTarget(int& lang_target, const RegionSaveStatus& savestatus, bool step_forward) {
	if (savestatus.GetLanguageCount() <= 1) {
		lang_target = 0;
		return;
	}
	for (int i = 0; i < savestatus.GetLanguageCount(); ++i) {
		int index = lang_target;
		if (savestatus.IsLanguageAvailable(index)) {
			lang_target = index;
			break;
		}
		step_forward ? (++index) : (--index);
		if (index < 0) index = savestatus.GetLanguageCount()-1;
		else if (index >= savestatus.GetLanguageCount()) index = 0;
		lang_target = index;
	}
}

static void PrintDigitalTarget(const RegionSaveStatus& savestatus) {
	printf(C_DEFAULT "Digital target: %s\n", savestatus.GetRegionStr());
}

static void PrintLinguisticTarget(const RegionSaveStatus& savestatus, int lang_target) {
	printf(C_DEFAULT "Language target: %s\n", savestatus.GetLanguageStr(lang_target));
}

static void InstallToDigitalTarget(const RegionSaveStatus& savestatus, int lang_target, bool formatsave) {
	consoleClear();

	do {
		Result res;
		u8* save = savestatus.LoadPreparedSave(lang_target);
		if (!save) {
			printf(C_BAD "Failed to load save to memory.\n");
			break;
		}

		if (formatsave) {
			puts(C_DEFAULT "Formating save...");
			res = FormatSave(savestatus.GetTitleId(), MEDIATYPE_SD);
			if (R_FAILED(res)) {
				printf(C_DEFAULT "Save format fail: " C_BAD "0x%08lX\n", res);
				free(save);
				break;
			} else {
				puts(C_GOOD "Successfully formatted save.");
			}
		}

		res = WriteSaveFile(save, savestatus.GetTitleId(), MEDIATYPE_SD);
		free(save);

		if (R_FAILED(res)) {
			printf(C_DEFAULT "Failed to write save: " C_BAD "0x%08lX\n", res);
			break;
		}

		printf(C_GOOD "Save installed!\n");
	} while(false);

	PressAToContinue();
}

static bool CartLangSelect(const RegionSaveStatus& savestatus, int& lang_target) {
	lang_target = 0;

	auto printinfo = [&]() {
		consoleClear();
		printf(C_DEFAULT "Game's region: " C_GOOD " %s\n", savestatus.GetRegionStr());
		PrintLinguisticTarget(savestatus, lang_target);
	};

	UpdateLinguisticTarget(lang_target, savestatus, true);
	PrintLinguisticTarget(savestatus, lang_target);

	if (savestatus.GetAvailableLanguageCount() <= 1)
		return aptMainLoop(); // just check if we should still running

	bool apt_run = aptMainLoop();
	for (; apt_run; apt_run = aptMainLoop())
	{
		hidScanInput();

		u32 kDown = hidKeysDown();

		if (kDown & KEY_START) break;

		else if (kDown & KEY_DUP) {
			if (++lang_target >= savestatus.GetLanguageCount()) lang_target = 0;
			UpdateLinguisticTarget(lang_target, savestatus, true);
			printinfo();
		}
		else if (kDown & KEY_DDOWN) {
			if (--lang_target < 0) lang_target = savestatus.GetLanguageCount()-1;
			UpdateLinguisticTarget(lang_target, savestatus, false);
			printinfo();
		}

		gfxFlushBuffers();
		gfxSwapBuffers();
		gspWaitForVBlank();
	}

	return apt_run;
}

static void InstallToCart(const RegionSaveStatus* regionsaves[3], bool formatsave) {
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
			if (regionsaves[target]->GetTitleId() == titleid) break;
		}

		if (target == 3) {
			printf(C_DEFAULT "Card not Picross3d! " C_BAD "0x%016llX\n", titleid);
			break;
		}

		printf(C_DEFAULT "Game's region: " C_GOOD " %s\n", regionsaves[target]->GetRegionStr());

		if (!regionsaves[target]->IsAvailable()) {
			printf(C_BAD "Save for this region not is ready!");
			break;
		}

		int lang_target;

		if(!CartLangSelect(*regionsaves[target], lang_target)) {
			// apt said to stop, so we cancel
			break;
		}

		u8* save = regionsaves[target]->LoadPreparedSave(lang_target);
		if (!save) {
			printf(C_BAD "Failed to load save to memory.\n");
			break;
		}

		if (formatsave) {
			puts(C_DEFAULT "Formating save...");
			res = FormatSave(regionsaves[target]->GetTitleId(), MEDIATYPE_GAME_CARD);
			if (R_FAILED(res)) {
				printf(C_DEFAULT "Save format fail: " C_BAD "0x%08lX\n", res);
				free(save);
				break;
			} else {
				puts(C_GOOD "Successfully formatted save.");
			}
		}

		res = WriteSaveFile(save, regionsaves[target]->GetTitleId(), MEDIATYPE_GAME_CARD);
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

static Result CheckDigitalAvailability(const RegionSaveStatus* regionsaves[3], bool digital_availability[3], bool& is_digital_available) {
	u32 titlecount;
	Result res = AM_GetTitleCount(MEDIATYPE_SD, &titlecount);
	if (R_FAILED(res)) {
		return res;
	}
	std::unique_ptr<u64> installedtitles(new(std::nothrow) u64[titlecount]);
	if (!installedtitles) {
		return MAKERESULT(RL_FATAL, RS_OUTOFRESOURCE, RM_APPLICATION, RD_OUT_OF_MEMORY);
	}
	res = AM_GetTitleList(&titlecount, MEDIATYPE_SD, titlecount, installedtitles.get());
	if (R_FAILED(res)) {
		return res;
	}
	for (u32 i = 0, found = 0; i < titlecount; ++i) {
		for (int j = 0; j < 3; ++j) {
			if (installedtitles.get()[i] == regionsaves[j]->GetTitleId()) {
				digital_availability[j] = true;
				if (regionsaves[j]->IsAvailable()) is_digital_available = true;
				++found;
			}
		}
		if (found == 3) break;
	}
	return 0;
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
		if (has_digital) puts("DPad Left/Right - Pick digital region");
		puts("DPad Up/Down - Pick language");
	} else {
		puts(C_BAD "Installs disabled, no saves ready");
	}
	puts(C_DEFAULT "Start - Exit");
}

static void PrintSaveInformation(const RegionSaveStatus* regionsaves[3]) {
	printf(C_DEFAULT "SAVEDATA Status:");
	for (int i = 0; i < 3; ++i) {
		printf(C_DEFAULT "\n- %s:", regionsaves[i]->GetRegionStr());
		if (!regionsaves[i]->IsAvailable()) {
			printf(C_BAD " Unavailable.");
			continue;
		}
		printf(C_GOOD "\n - ");
		for (int j = 0, printed_count = 0; j < regionsaves[i]->GetLanguageCount(); ++j) {
			if (regionsaves[i]->IsLanguageAvailable(j)) {
				if (printed_count) printf(", ");
				printf("%s", regionsaves[i]->GetLanguageStr(j));
				++printed_count;
			}
		}
	}
	printf("\n\n");
}

static void PrintDigitalAvailability(const RegionSaveStatus* regionsaves[3], bool digital_availability[3], Result scan_result) {
	if (R_FAILED(scan_result)) {
		printf(C_DEFAULT "Digital search failed: " C_BAD "0x%08lX", scan_result);
	} else {
		printf(C_DEFAULT "Digital installs found: " C_GOOD);
		int print_count = 0;
		for (int i = 0; i < 3; ++i) {
			if (digital_availability[i]) {
				if (print_count) printf(", ");
				printf("%s", regionsaves[i]->GetRegionStr());
				++print_count;
			}
		}
		if (!print_count) printf(C_DEFAULT "None");
	}
	printf("\n\n");
}

int main(int argc, char** argv) {
	gfxInitDefault();
	amInit();
	fsInit();

	PrintConsole topScreen, bottomScreen;

	consoleInit(GFX_TOP, &topScreen);
	consoleInit(GFX_BOTTOM, &bottomScreen);

	consoleSelect(&topScreen);

	puts(C_TITLE " == nitpic3d Installer ==\n");

	if(!RegionSaveStatus::SetCWDBase()) {
		puts(C_BAD "Path Setup failed. Using sdmc root.\n");
	}

	EURSaveStatus eursave;
	USASaveStatus usasave;
	JPNSaveStatus jpnsave;

	const RegionSaveStatus* regionsaves[3] = {&eursave, &usasave, &jpnsave};

	bool digital_availability[3] = {false, false, false};
	bool is_digital_available = false;
	bool has_ready_saves = eursave.IsAvailable() || usasave.IsAvailable() || jpnsave.IsAvailable();

	Result digital_check_res = CheckDigitalAvailability(regionsaves, digital_availability, is_digital_available);

	PrintSaveInformation(regionsaves);
	PrintDigitalAvailability(regionsaves, digital_availability, digital_check_res);

	PrintControls(has_ready_saves, is_digital_available);

	consoleSelect(&bottomScreen);

	int digital_target = 0;
	int lang_target = 0;

	auto printtargets = [&]() {
		consoleClear();
		PrintDigitalTarget(*regionsaves[digital_target]);
		PrintLinguisticTarget(*regionsaves[digital_target], lang_target);
	};

	if (is_digital_available) {
		UpdateDigitalTarget(digital_target, digital_availability, regionsaves, true);
		UpdateLinguisticTarget(lang_target, *regionsaves[digital_target], true);
		printtargets();
	}

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
			int old_target = digital_target;
			if (++digital_target > 2) digital_target = 0;
			UpdateDigitalTarget(digital_target, digital_availability, regionsaves, true);
			if (old_target != digital_target) {
				lang_target = 0;
				UpdateLinguisticTarget(lang_target, *regionsaves[digital_target], true);
				printtargets();
			}
		}
		else if (is_digital_available && kDown & KEY_DLEFT) {
			int old_target = digital_target;
			if (--digital_target < 0) digital_target = 2;
			UpdateDigitalTarget(digital_target, digital_availability, regionsaves, false);
			if (old_target != digital_target) {
				lang_target = 0;
				UpdateLinguisticTarget(lang_target, *regionsaves[digital_target], true);
				printtargets();
			}
		}
		else if (is_digital_available && kDown & KEY_DUP) {
			if (++lang_target >= regionsaves[digital_target]->GetLanguageCount()) lang_target = 0;
			UpdateLinguisticTarget(lang_target, *regionsaves[digital_target], true);
			printtargets();
		}
		else if (is_digital_available && kDown & KEY_DDOWN) {
			if (--lang_target < 0) lang_target = regionsaves[digital_target]->GetLanguageCount()-1;
			UpdateLinguisticTarget(lang_target, *regionsaves[digital_target], false);
			printtargets();
		}
		else if (is_digital_available && kDown & KEY_Y) {
			InstallToDigitalTarget(*regionsaves[digital_target], lang_target, (kHeld & KEY_L) != 0);
			printtargets();
		}
		else if (has_ready_saves && kDown & KEY_X) {
			InstallToCart(regionsaves, (kHeld & KEY_L) != 0);
			if (is_digital_available) printtargets();
			else consoleClear();
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
