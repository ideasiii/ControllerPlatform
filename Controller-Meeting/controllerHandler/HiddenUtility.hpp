#pragma once

#include <chrono>
#include <cryptopp/base64.h>
#include <cryptopp/filters.h>
#include <cryptopp/sha.h>
#include <ctime>
#include <errno.h>
#include <fcntl.h>
#include <iomanip>
#include <linux/limits.h>
#include <regex>
#include <sstream>
#include <stdint.h>
#include <string>
#include <string.h>
#include <sys/inotify.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "common.h"
#include "CMysqlHandler.h"
#include "LogHandler.h"
#include "MysqlSource.h"

// Written and placed in the public domain by rrmmnn
// Copyright assigned to the Crypto++ project.

namespace CryptoPP
{
	class VectorSink : public Bufferless<Sink>
	{
	public:

		VectorSink(std::vector<uint8_t>& out)
			: _out(&out)
		{
		}

		size_t Put2(const byte *inString, size_t length, int /*messageEnd*/, bool /*blocking*/)
		{
			_out->insert(_out->end(), inString, inString + length);
			return 0;
		}

	private:
		std::vector<uint8_t>* _out;
	};
}

// Utility functions that may require C++11,
// Lead to difficulty merging into Global (for now)
class HiddenUtility
{
public:
	// Returns Unix timestamp in milliseconds, UTC
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

	static void sha256(uint8_t *out, std::string data)
	{
		sha256(out, (const uint8_t *)data.data(), data.size());
	}

	// 對 data 做 SHA256 hash 並將結果放在 out
	// 必須確保 out 的長度至少為 32 (CryptoPP::SHA256::DIGESTSIZE)
	static void sha256(uint8_t *out, const uint8_t *data, uint dataLen)
	{
		CryptoPP::SHA256().CalculateDigest(out, data, dataLen);
	}

	static std::vector<byte> decodeBase64(std::string src)
	{
		std::vector<byte> srcDecoded;
		CryptoPP::StringSource(
			src,
			true,
			new CryptoPP::Base64Decoder(new CryptoPP::VectorSink(srcDecoded))
		);

	    return srcDecoded;
	}

	static void arrayToHex(const uint8_t *m, uint size,
		const char *delimiter, uint bytesPerLine)
	{
		for (int i = 0; i < size; i++)
		{
			printf("0x%02x%s", m[i], delimiter);
			if (bytesPerLine && (i % bytesPerLine == 0) && i)
			{
				printf("\n");
			}
		}
		printf("\n");
	}

	static void arrayToOneLineHex(const uint8_t *m, uint size)
	{
		for (int i = 0; i < size; i++)
		{
			printf("%02x", m[i]);
		}
		printf("\n");
	}

	// Allocates a memory block and stores hex representation of src
	// Remember to delete after use
	static std::string getHexString(const uint8_t *src, uint srcSize)
	{
		static char hexconvtab[] = "0123456789abcdef";
		char *pRaw = new char[srcSize * 2 + 1];
		char *raw = pRaw;

		for (uint i = 0; i < srcSize; i++)
		{
			uint8_t b = src[i];
			*(pRaw++) = hexconvtab[b >> 4];
			*(pRaw++) = hexconvtab[b & 15];
		}

		*pRaw = '\0';
		std::string ret(raw);
		delete[] raw;

		return ret;
	}

	// A handy function for generating random IV.
	static void getRandomBytes(uint8_t *outBuf, const int len)
	{
		int fd = open("/dev/urandom", O_RDONLY);
		read(fd, outBuf, len);
		close(fd);
	}

private:
	HiddenUtility()
	{
	}

	~HiddenUtility()
	{
	}
};
