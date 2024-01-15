#pragma once
#include <climits>
#include <string>
#include <fstream>
#include <iostream>
#pragma warning(disable:4996)

#define LOGE(_module, ...) LOG(kLogError, _module, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__);
#define LOGW(_module, ...) LOG(kLogWarning,_module, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__);
#define LOGI(_module,...) LOG(kLogInfo,_module, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__);
#define LOGD(_module,...) LOG(kLogDebug, _module, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__);
#define LOGV(_module,...) LOG(kLogVerbose,_module, __FILE__,  __FUNCTION__, __LINE__, __VA_ARGS__);

namespace LTLOG
{
	typedef unsigned long long ULL;

	enum LogOutputMode
	{
		Console = (1 << 0),
		SystemDebugger = (1 << 1), // debug view查看
		File = (1 << 2)
	};

	enum LogWriterType
	{
		PerDay = 0,
		PerLevel,
		PerModule,
		PerFixedSize,
	};

	enum LogModule : ULL
	{
		kModuleNone = 0,
		kModuleBIV = (1 << 0),
		kModulePV = (1 << 1),
		kModuleCamera = (1 << 2),
		kModuleBoxCalibration = (1 << 3),
		kModuleLTFalcon=(1<<4),
		kModuleImplementHalcon = (1<<5),
		kMarkCore = (1 << 6),
		//...
		kModuleAll = ULLONG_MAX
	};

	enum LogLevel {
		kLogError = 0,
		kLogWarning,
		kLogInfo,
		kLogDebug,
		kLogVerbose,

		kLogLevelCount
	};
	struct LogDetail
	{
		LogDetail(LogLevel l, LogModule m) :m_logLevel(l), m_logModule(m) {}
		LogLevel m_logLevel;
		LogModule m_logModule;
	};
	typedef std::pair<LogDetail, std::string> LogItem;

	class LogConfigger
	{
		typedef unsigned long long ULL;
	public:
		static LogConfigger* getInstance()
		{
			static LogConfigger o;
			return &o;
		}

		std::string m_logTag = "LTPostVision";//固定的tag
		std::string m_logRootFolder = ""; //log存放路径
		ULL m_logOutputMode = 2; //输出位置，控制台/内核/文件
		int m_logLevel = 2; //控制error/warning/info...等级，值越大，开的越详细
		//不同log等级下要输出的模块
		ULL m_logErrorMask = ULLONG_MAX;
		ULL m_logWarningMask = ULLONG_MAX;
		ULL m_logInfoMask = ULLONG_MAX;
		ULL m_logDebugMask = 0;
		ULL m_logVerboseMask = 0;
		LTLOG::LogWriterType m_logWriterType = LTLOG::LogWriterType::PerDay; // log写入策略，每天写/每个等级写/每个模块写/...
		ULL m_logPerFixSize = 0;//如果用 LogWriterType::PerFixSize，则用配置该参数
		ULL m_logPerFixSizeMaxCount = ULLONG_MAX; //如果配置固定大小，可配置该参数最多生成几个文件, not impl
	private:
		void ParseConfigFile(const std::string& filename);

		LogConfigger()
		{
			ParseConfigFile(".\\log.config");
		}
		void assignConfigValue(const std::string& key, const  std::string& value);
		LogConfigger(const LogConfigger&) = delete;
		LogConfigger(const LogConfigger&&) = delete;
		LogConfigger operator = (const LogConfigger&) = delete;
	};

	extern "C" __declspec(dllexport) void LOG(LogLevel level, LogModule module, const char* szFileName, const char* szFuncName, int nLine, const char* format, ...);
	extern "C" __declspec(dllexport) void SetLogLevel(int level);
	extern "C" __declspec(dllexport) int GetLogLevel();
}

//#define LOGE_(...) LOG(kLogError,LogModule::kModuleNone , __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__);
//#define LOGW_(...) LOG(kLogWarning,LogModule::kModuleNone, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__);
//#define LOGI_(...) LOG(kLogInfo,LogModule::kModuleNone, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__);
//#define LOGD_(...) LOG(kLogDebug, LogModule::kModuleNone, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__);
//#define LOGV_(...) LOG(kLogVerbose,LogModule::kModuleNone, __FILE__,  __FUNCTION__, __LINE__, __VA_ARGS__);
