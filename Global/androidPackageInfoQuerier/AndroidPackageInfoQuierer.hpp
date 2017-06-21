/**
 * Holds info of an Android package (apk) file
 * This class relys on an external tool, AAPT
 * Compiler must support C++11 features
 */

#include <cstdio>
#include <iostream>
#include <memory>
#include <regex>
#include <stdexcept>
#include <string>
#include <array>
#include "LogHandler.h"
#include "utility.h"

// aapt d badge 指令返回的結果中，與 apk 版本有關的 regex pattern
#define PKG_NAME_VER_PATTERN R"(package: name='(([A-Za-z]{1}[A-Za-z\d_]*\.)*[A-Za-z][A-Za-z\d_]*)' versionCode='(\d+)' versionName='([^\']+)')"

class AndroidPackageInfoQuierer
{
public:
	AndroidPackageInfoQuierer(std::string aaptPath, std::string pkgName) :
		aaptPath(aaptPath), packageName(pkgName), versionCode(-1)
	{
	}

	~AndroidPackageInfoQuierer()
	{
	}

	bool extractVersionFromApk(std::string apkPath)
	{
		char buffer[128];
		std::string cmd, result;

		cmd += aaptPath + " d badging " + apkPath;
		//_log("[AndroidPackageInfoQuierer] extractVersionFromApk() cmd = `%s`", cmd.c_str());
		FILE *hFile = popen(cmd.c_str(), "r");

		if (!hFile)
		{
			_log("[AndroidPackageInfoQuierer] extractVersionFromApk() popen() failed!");
			return false;
		}

		try
		{
			while (!feof(hFile))
			{
				if (fgets(buffer, sizeof(buffer), hFile) != NULL)
					result += buffer;
			}
		}
		catch (const bad_alloc &e)
		{
			_log("[AndroidPackageInfoQuierer] extractVersionFromApk() bad_alloc occured");
			pclose(hFile);
			return false;
		}

		int ret = pclose(hFile);
		if (ret != 0)
		{
			_log("[AndroidPackageInfoQuierer] extractVersionFromApk() execution of aapt failed (%d)", ret);
			return false;
		}

		//_log("[AndroidPackageInfoQuierer] extractVersionFromApk() result = %s", result.c_str());

		std::regex pkgNameVerRegex(PKG_NAME_VER_PATTERN);
		smatch sm;
		//_log("[AndroidPackageInfoQuierer] PKG_NAME_VER_PATTERN = %s", PKG_NAME_VER_PATTERN);

		if (!regex_search(result, sm, pkgNameVerRegex))
		{
			_log("[AndroidPackageInfoQuierer] extractVersionFromApk() regex no match");
			return false;
		}

		std::string extractedPkgName = sm[1].str();
		std::string extractedVersionCode = sm[3].str();
		std::string extractedVersionName = sm[4].str();

		if (!packageName.empty() && packageName.compare(extractedPkgName) != 0)
		{
			_log("[AndroidPackageInfoQuierer] extractVersionFromApk() package name does not match");
			return false;
		}

		convertFromString(versionCode, extractedVersionCode);
		versionName = extractedVersionName;

		return true;
	}

	int getVersionCode()
	{
		return versionCode;
	}

	std::string getVersionName()
	{
		return versionName;
	}

private:
	// Path of AAPT (Android Asset Packaging Tool).
	// aapt utility is used to extract info from AndroidManifest.xml in an apk
	std::string aaptPath;

	// Target pakage name.
	// Left blank so package validation on extraction will be skipped
	std::string packageName;

	//std::string apkPath;
	int versionCode;
	std::string versionName;
};
