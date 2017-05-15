#include <chrono>
#include <errno.h>
#include <string.h>
#include <sys/inotify.h>
#include <unistd.h>
#include "LogHandler.h"

// Some functions in HiddenUtility requires C++11 support
class HiddenUtility
{
public:
	// Returns Unix timestamp in milliseconds, UTC timezone 
	inline static int64_t unixTimeMilli()
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>
			(std::chrono::system_clock::now().time_since_epoch()).count();
	}

	static int initInotifyWithOneWatchDirectory(int *oFd, int *oWd, const char *dir, uint32_t mask)
	{
		int fd = inotify_init();
		if (fd == -1)
		{
			_log("[HiddenUtility] inotify_init() failed: %s", strerror(errno));
			return errno;
		}

		int wd = inotify_add_watch(fd, dir, mask);
		if (wd == -1)
		{
			_log("[HiddenUtility] inotify_add_watch() failed: %s", strerror(errno));
			close(fd);
			return errno;
		}

		_log("[HiddenUtility] Added watch on %s", dir);

		*oFd = fd;
		*oWd = wd;

		return 0;
	}

	static bool strEndsWith(const char *hay, const char *needle)
	{
		const int hayLen = strlen(hay);
		const int needleLen = strlen(needle);

		return hayLen >= needleLen && strcmp(hay + hayLen - needleLen, needle) == 0;
	}

private:
	HiddenUtility()
	{
	}
	
	~HiddenUtility() 
	{
	}
};
