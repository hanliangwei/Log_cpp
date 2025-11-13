#pragma once
#include <climits>
#include <string>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>
#pragma warning(disable:4996)

#ifndef __log__
#define LOGE(...)
#define LOGW(...)
#define LOGI(...)
#define LOGD(...)
#define LOGV(...)
#else
#define LOGE(_module, ...) iTinyLog_LOG(LogLevel::E, _module, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__);
#define LOGW(_module, ...) iTinyLog_LOG(LogLevel::W,_module, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__);
#define LOGI(_module,...) iTinyLog_LOG(LogLevel::I,_module, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__);
#define LOGD(_module,...) iTinyLog_LOG(LogLevel::D, _module, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__);
#define LOGV(_module,...) iTinyLog_LOG(LogLevel::V,_module, __FILE__,  __FUNCTION__, __LINE__, __VA_ARGS__);
#endif

namespace TinyLog
{
	typedef unsigned long long ULL;
	typedef int ModuleIndex;
	typedef std::unordered_map<std::string, bool> ModuleMask;
	

	enum class LogOutputMode : ULL
	{
		CONSOLE = (1 << 0),
		SYSTEM_DEBUGGER = (1 << 1), // debug view查看
		FILE = (1 << 2)
	};

	enum LogWriterType
	{
		PER_DAY = 0,
		PER_LEVEL,
		PER_MODULE,
		PER_FIXED_SIZE,
	};

	enum class LogLevel : int
	{
		E = 0,
		W,
		I,
		D,
		V,

		COUNT
	};

	struct LogDetail
	{
		LogDetail(LogLevel l, ModuleIndex m) :m_logLevel(l), m_logModule(m) {}
		LogLevel m_logLevel;
		ModuleIndex m_logModule;
	};
	typedef std::pair<LogDetail, std::string> LogItem;

	class LogConfigger
	{
	public:
		static LogConfigger* getInstance()
		{
			static LogConfigger o;
			return &o;
		}

		std::string m_logTag;//固定的tag
		std::string m_logRootFolder; //log存放路径
		ULL m_logOutputMode; //输出位置，控制台/内核/文件
		int m_logLevel; //控制error/warning/info...等级，值越大，开的越详细
		//不同log等级下要输出的模块
		ModuleMask m_errorMask;
		ModuleMask m_warningMask;
		ModuleMask m_infoMask;
		ModuleMask m_debugMask;
		ModuleMask m_verboseMask;
		LogWriterType m_logWriterType; // log写入策略，每天写/每个等级写/每个模块写/...
		ULL m_logPerFixSize;//如果用 LogWriterType::PerFixSize，则用配置该参数
		ULL m_logPerFixSizeMaxCount; //如果配置固定大小，可配置该参数最多生成几个文件, not impl
		std::vector<std::string> m_moduleNames;
		
		std::string getModuleName(ModuleIndex idx)
		{
			if (idx<0 || idx >= m_moduleNames.size()) return "__";
			return m_moduleNames[idx];
		}
		int getModuleIndex(const std::string& moduleName)
		{
			auto iter = std::find(m_moduleNames.begin(), m_moduleNames.end(), moduleName);
			if (iter == m_moduleNames.end())
				return -1;
			return iter - m_moduleNames.begin();
		}
	private:
		void parseConfigFile(const std::string& filename);
		std::vector<std::string> splitString(const std::string& str, char delimiter);
		LogConfigger();
		void assignConfigValue(const std::string& key, const  std::string& value);
		LogConfigger(const LogConfigger&) {}
		LogConfigger(const LogConfigger&&) {}
		LogConfigger operator = (const LogConfigger&) {}
	};

	extern "C" __declspec(dllexport) void iTinyLog_LOG(LogLevel level, ModuleIndex module, const char* szFileName, const char* szFuncName, int nLine, const char* format, ...);
	extern "C" __declspec(dllexport) void iTinyLog_SetLogLevel(int level);
	extern "C" __declspec(dllexport) int iTinyLog_GetLogLevel();
	extern "C" __declspec(dllexport) int iTinyLog_GetLogModuleIndex(const char* szModuleName);

}
