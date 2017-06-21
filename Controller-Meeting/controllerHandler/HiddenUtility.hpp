#pragma once

#include <chrono>
#include <ctime>
#include <errno.h>
#include <iomanip>
#include <linux/limits.h>
#include <regex>
#include <sstream>
#include <string>
#include <string.h>
#include <sys/inotify.h>
#include <unistd.h>
#include "common.h"
#include "CMysqlHandler.h"
#include "LogHandler.h"
#include "MysqlSource.h"

// Utility functions that may require C++11,
// which means merge into Global is not possible (for now)
class HiddenUtility
{
public:
	// Returns Unix timestamp in milliseconds, UTC timezone
	inline static int64_t unixTimeMilli()
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>
			(std::chrono::system_clock::now().time_since_epoch()).count();
	}

	static std::string currentDateTime(bool inUtc)
	{
		auto now = std::chrono::system_clock::now();
		auto now_time_t = std::chrono::system_clock::to_time_t(now);

		std::stringstream ss;

		if (inUtc)
		{
			ss << std::put_time(std::gmtime(&now_time_t), "%Y-%m-%d %X");
		}
		else
		{
			ss << std::put_time(std::localtime(&now_time_t), "%Y-%m-%d %X");
		}

		return ss.str();
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

	// Get path of current process
	// This only works on Linux
	static void getSelfExePath(char *dest, size_t destLen)
	{
		int pathLen = readlink("/proc/self/exe", dest, destLen);
		dest[pathLen] = '\0';
	}

	// 取得與 process image 位於同一資料夾內的 config 檔
	static std::string getConfigPathInProcessImageDirectory()
	{
		char selfExePath[PATH_MAX];

		getSelfExePath(selfExePath, PATH_MAX);
		std::string strConfPath(selfExePath);
		strConfPath.append(".conf");

		return strConfPath;
	}

	// Checks whether 's' match regular expression pattern 'pattern'
	static bool RegexMatch(std::string const& s, std::string const& pattern)
	{
		std::regex regexp(pattern);
		return std::regex_match(s, regexp);
	}

	// Checks whether 's' match regular expression pattern 'pattern'
	static bool RegexMatch(std::string const& s, const char* pattern)
	{
		std::regex regexp(pattern);
		return std::regex_match(s, regexp);
	}

	// 從 DB 取出資料
	// 當存取 DB 時發生錯誤或是得到的結果為空時, 返回 false, 否則返回 true
	static bool selectFromDb(const char* callerDesc, std::string const& strSQL, std::list<std::map<std::string, std::string>>& listRet)
	{
		std::unique_ptr<CMysqlHandler> mysql(MysqlSource::getInstance().getMysqlHandler());
		if (mysql == nullptr)
		{
			_log("%s selectFromDb() Mysql cannot make connection", callerDesc);
			return false;
		}

		int nRet = mysql->query(strSQL, listRet);
		std::string strError = mysql->getLastError();
		mysql->close();

		if (FALSE == nRet)
		{
			_log("%s selectFromDb() Mysql Error: %s", callerDesc, strError.c_str());
			return false;
		}
		else if (listRet.size() < 1)
		{
			_log("%s selectFromDb() Mysql zero return row", callerDesc);
			return false;
		}

		return true;
	}

	// 當存取 DB 時發生錯誤, 返回 false, 否則返回 true
	static bool execOnDb(const char* callerDesc, std::string const& strSQL)
	{
		std::unique_ptr<CMysqlHandler> mysql(MysqlSource::getInstance().getMysqlHandler());
		if (mysql == nullptr)
		{
			_log("%s insertIntoDb() Mysql cannot make connection", callerDesc);
			return false;
		}

		if (FALSE == mysql->sqlExec(strSQL))
		{
			_log("%s Mysql sqlExec Error: %s", callerDesc, mysql->getLastError().c_str());
			mysql->close();
			return false;
		}

		_log("%s run MYSQL Success: %s", callerDesc, strSQL.c_str());
		mysql->close();
		return true;
	}

private:
	HiddenUtility()
	{
	}

	~HiddenUtility()
	{
	}
};
