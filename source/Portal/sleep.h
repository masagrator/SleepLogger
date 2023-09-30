#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

	int thrd_sleep( const struct timespec* duration, struct timespec* remaining );
	int clock_nanosleep(clockid_t clockid, int flags, const struct timespec *request, struct timespec *_Nullable remain);
	int nanosleep(const struct timespec *req, struct timespec *_Nullable rem);

#ifdef __cplusplus
}
#endif