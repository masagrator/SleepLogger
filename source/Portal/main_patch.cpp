#include "skyline/inlinehook/And64InlineHook.hpp"
#include "skyline/utils/cpputils.hpp"
#include "skyline/inlinehook/memcpy_controlled.hpp"
#include "nn/fs.h"
#include "ltoa.h"

FILE* file = 0;
bool nx_lock = false;

uintptr_t TextRegionOffset = 0;

ptrdiff_t returnInstructionOffset(uintptr_t LR) {
	return LR - TextRegionOffset;
}

void* (*SleepThread_original)(uint64_t nanoseconds);
void* SleepThread_hook(uint64_t nanoseconds) {
	static bool Initialized = false;
	static bool nx_lock = false;
	if (!Initialized) {
		nn::fs::MountSdCardForDebug("sdmc");
		file = fopen("sdmc:/SleepDebug.txt", "w");
		if (file)
			Initialized = true;
	}
	else if (nanoseconds >= 10'000'000 && nanoseconds <= 33'333'333) {
		while (nx_lock) 
			nn::os::SleepThread(nn::TimeSpan(10000));
		nx_lock = true;
		fwrite("Address0: 0x", 12, 1, file);
		char buffer[34] = "";
		ultoa((unsigned long)returnInstructionOffset((uintptr_t)__builtin_return_address(0)), &buffer[0], 16);
		fwrite(&buffer[0], strlen(&buffer[0]), 1, file);
		fwrite(", Nanoseconds: ", 15, 1, file);
		ultoa(nanoseconds, &buffer[0], 10);
		fwrite(&buffer[0], strlen(&buffer[0]), 1, file);
		fwrite("\n", 1, 1, file);
		nx_lock = false;
	}
	return SleepThread_original(nanoseconds);
}

void Portal_main()
{
	TextRegionOffset = (uintptr_t)skyline::utils::getRegionAddress(skyline::utils::region::Text);

	A64HookFunction((void**)&nn::os::SleepThread, reinterpret_cast<void*>(SleepThread_hook), (void**)&SleepThread_original);

}
