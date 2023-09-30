#include "skyline/inlinehook/And64InlineHook.hpp"
#include "skyline/utils/cpputils.hpp"
#include "skyline/inlinehook/memcpy_controlled.hpp"
#include "nn/fs.h"
#include "ltoa.h"
#include "sleep.h"

FILE* file = 0;
bool nx_lock = false;

uintptr_t TextRegionOffset = 0;

ptrdiff_t returnInstructionOffset(uintptr_t LR) {
	return LR - TextRegionOffset;
}

bool Initialized = false;
bool lock_nx = false;

int (*thrd_sleep_original)(const struct timespec* duration, struct timespec* remaining);
int thrd_sleep_hook(const struct timespec* duration, struct timespec* remaining) {
	if (!Initialized) {
		nn::fs::MountSdCardForDebug("sdmc");
		file = fopen("sdmc:/SleepDebug.txt", "w");
		if (file)
			Initialized = true;
	}
	else if (duration -> tv_nsec >= 10'000'000 &&  duration -> tv_nsec <= 33'333'333) {
		while (nx_lock) 
			nn::os::SleepThread(nn::TimeSpan(10000));
		nx_lock = true;
		fwrite("Address0: 0x", 12, 1, file);
		char buffer[64] = "";
		ultoa((unsigned long)returnInstructionOffset((uintptr_t)__builtin_return_address(0)), &buffer[0], 16);
		fwrite(&buffer[0], strlen(&buffer[0]), 1, file);
		fwrite(", duration -> tv_nsec: ", 15, 1, file);
		ultoa(duration -> tv_nsec, &buffer[0], 10);
		fwrite(&buffer[0], strlen(&buffer[0]), 1, file);
		fwrite(", remaining -> tv_nsec: ", 15, 1, file);
		ultoa(remaining -> tv_nsec, &buffer[0], 10);
		fwrite(&buffer[0], strlen(&buffer[0]), 1, file);
		fwrite("\n", 1, 1, file);
		nx_lock = false;
	}
	return thrd_sleep_original(duration, remaining);
}

void* (*clock_nanosleep_original)(clockid_t clockid, int flags, const struct timespec *request, struct timespec *_Nullable remain);
void* clock_nanosleep_hook(clockid_t clockid, int flags, const struct timespec *request, struct timespec *_Nullable remain) {
	if (!Initialized) {
		nn::fs::MountSdCardForDebug("sdmc");
		file = fopen("sdmc:/SleepDebug.txt", "w");
		if (file)
			Initialized = true;
	}
	else if (request -> tv_nsec >= 10'000'000 && request -> tv_nsec <= 33'333'333) {
		while (nx_lock) 
			nn::os::SleepThread(nn::TimeSpan(10000));
		nx_lock = true;
		fwrite("Address0: 0x", 12, 1, file);
		char buffer[64] = "";
		ultoa((unsigned long)returnInstructionOffset((uintptr_t)__builtin_return_address(0)), &buffer[0], 16);
		fwrite(&buffer[0], strlen(&buffer[0]), 1, file);
		fwrite(", request -> tv_nsec: ", 15, 1, file);
		ultoa(request -> tv_nsec, &buffer[0], 10);
		fwrite(&buffer[0], strlen(&buffer[0]), 1, file);
		if (remain) {
			fwrite(", remain -> tv_nsec: ", 15, 1, file);
			ultoa(remain -> tv_nsec, &buffer[0], 10);
			fwrite(&buffer[0], strlen(&buffer[0]), 1, file);
		}
		fwrite("\n", 1, 1, file);
		nx_lock = false;
	}
	return clock_nanosleep_original(clockid, flags, request, remain);
}

void* (*nanosleep_original)(const struct timespec *req, struct timespec *_Nullable rem);
void* nanosleep_hook(const struct timespec *req, struct timespec *_Nullable rem) {
	if (!Initialized) {
		nn::fs::MountSdCardForDebug("sdmc");
		file = fopen("sdmc:/SleepDebug.txt", "w");
		if (file)
			Initialized = true;
	}
	else if (req -> tv_nsec >= 10'000'000 && req -> tv_nsec <= 33'333'333) {
		while (nx_lock) 
			nn::os::SleepThread(nn::TimeSpan(10000));
		nx_lock = true;
		fwrite("Address0: 0x", 12, 1, file);
		char buffer[64] = "";
		ultoa((unsigned long)returnInstructionOffset((uintptr_t)__builtin_return_address(0)), &buffer[0], 16);
		fwrite(&buffer[0], strlen(&buffer[0]), 1, file);
		fwrite(", req -> tv_nsec: ", 15, 1, file);
		ultoa(req -> tv_nsec, &buffer[0], 10);
		fwrite(&buffer[0], strlen(&buffer[0]), 1, file);
		if (rem) {
			fwrite(", rem -> tv_nsec: ", 15, 1, file);
			ultoa(req -> tv_nsec, &buffer[0], 10);
			fwrite(&buffer[0], strlen(&buffer[0]), 1, file);
		}
		fwrite("\n", 1, 1, file);
		nx_lock = false;
	}
	return nanosleep_original(req, rem);
}

void* (*SleepThread_original)(uint64_t nanoseconds);
void* SleepThread_hook(uint64_t nanoseconds) {
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
	A64HookFunction((void**)&thrd_sleep, reinterpret_cast<void*>(thrd_sleep_hook), (void**)&thrd_sleep_original);
	A64HookFunction((void**)&clock_nanosleep, reinterpret_cast<void*>(clock_nanosleep_hook), (void**)&clock_nanosleep_original);
	A64HookFunction((void**)&nanosleep, reinterpret_cast<void*>(nanosleep_hook), (void**)&nanosleep_original);

}
