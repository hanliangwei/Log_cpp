#include "log.h"
#include <Windows.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <debugapi.h>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <queue>
#include <fstream>
#include <memory>

namespace TinyLOG
{
	inline std::string timeNow(bool bDateOnly = false)
	{
		std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
		std::time_t now_time = std::chrono::system_clock::to_time_t(now);
		std::tm* local_time = std::localtime(&now_time);

		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

		std::ostringstream oss;
		oss << std::setfill('0') << std::setw(4) << local_time->tm_year + 1900 << "-"
			<< std::setfill('0') << std::setw(2) << local_time->tm_mon + 1 << "-"
			<< std::setfill('0') << std::setw(2) << local_time->tm_mday;

		if (!bDateOnly)
		{
			oss << " "
				<< std::setfill('0') << std::setw(2) << local_time->tm_hour << ":"
				<< std::setfill('0') << std::setw(2) << local_time->tm_min << ":"
				<< std::setfill('0') << std::setw(2) << local_time->tm_sec << ":"
				<< std::setfill('0') << std::setw(3) << ms.count();
		}

		std::string time_str = oss.str();

		return time_str;
	}

	inline std::string LogModule2String(LogModule m)
	{
		switch (m)
		{
		case LogModule::kModuleNone: return "";
		case LogModule::kModuleBIV: return "BIV";
		case LogModule::kModulePV: return "PV";
		case LogModule::kModuleBoxCalibration: return "BoxCalibration";
		case LogModule::kModuleCamera: return "Camera";
		case LogModule::kModuleAll: return "ModuleAll";
		case LogModule::kModuleLTFalcon: return "LTFalcon";
		case LogModule::kModuleImplementHalcon: return "ImplementHalcon";
		case LogModule::kMarkCore: return "Core";
		}
		return "unknown module";
	}
	inline std::string LogLevel2String(LogLevel level)
	{
		switch (level)
		{
		case LogLevel::kLogError:return "E";
		case LogLevel::kLogWarning:return "W";
		case LogLevel::kLogInfo: return "I";
		case LogLevel::kLogDebug: return "D";
		case LogLevel::kLogVerbose: return "V";
		}
		return "unknown level";
	}

	inline const char* GetFileName(char* path)
	{
		char* p = strrchr(path, '\\');
		char* filename;
		if (p == NULL) {
			filename = path;
		}
		else {
			filename = p + 1;
		}
		return filename;
	}

	void CreateRecursiveDirectory(const std::string& path)
	{
		if (CreateDirectoryA(path.c_str(), NULL) || GetLastError() == ERROR_ALREADY_EXISTS) {
			return;
		}

		std::string parentPath = path.substr(0, path.find_last_of('\\'));
		CreateRecursiveDirectory(parentPath);
		CreateDirectoryA(path.c_str(), NULL);
	}

	class LogWriterBase
	{
	public:
		LogWriterBase()
		{
			CreateRecursiveDirectory(LogConfigger::getInstance()->m_logRootFolder);
		}
		virtual ~LogWriterBase() {}
		virtual void write(const LogItem& msg) = 0;

	};
	class LogWriterPerDay : public LogWriterBase
	{
	public:
		LogWriterPerDay()
		{
			m_lastDay = timeNow(true);
		}
		~LogWriterPerDay() override
		{
			if (m_ofs.is_open())
				m_ofs.close();
		}
		void write(const LogItem& logItem) override
		{
			std::string now = timeNow(true);
			if (m_lastDay != now && m_ofs.is_open())
				m_ofs.close();

			if (!m_ofs.is_open())
			{
				m_ofs.open((LogConfigger::getInstance()->m_logRootFolder + "\\" + now + ".log").c_str(), std::ios::out | std::ios::app);
			}
			m_ofs << logItem.second;
			m_ofs.flush();
		}
	private:
		std::ofstream m_ofs;
		std::string m_lastDay;
	};
	class LogWriterPerLevel : public LogWriterBase
	{
	public:
		LogWriterPerLevel()
		{
			m_LogWriters.resize(LogLevel::kLogLevelCount);
			for (int i = 0; i < LogLevel::kLogLevelCount; ++i)
			{
				m_LogWriters[i] = std::make_shared<LogWriterPerDay>();
				CreateRecursiveDirectory(LogConfigger::getInstance()->m_logRootFolder + "\\" + LogLevel2String((LogLevel)i));
			}
		}
		~LogWriterPerLevel() override
		{
		}
		void write(const LogItem& logItem) override 
		{
			auto oldRoot = LogConfigger::getInstance()->m_logRootFolder;
			LogConfigger::getInstance()->m_logRootFolder = LogConfigger::getInstance()->m_logRootFolder + "\\" + LogLevel2String(logItem.first.m_logLevel);
			m_LogWriters[(int)logItem.first.m_logLevel]->write(logItem);
			LogConfigger::getInstance()->m_logRootFolder = oldRoot;
		}
	private:
		std::vector<std::shared_ptr<LogWriterBase>> m_LogWriters;
	};
	class LogWriterPerFixedSize : public LogWriterBase
	{
	public:
		LogWriterPerFixedSize()
		{
			std::string now = timeNow(true);
			while (m_curFileIdx < 100)
			{
				std::string logName = LogConfigger::getInstance()->m_logRootFolder + "\\" + now + "_" + std::to_string(m_curFileIdx) + ".log";
				auto fileSize = GetFileSize(logName);
				auto fixSize = (LogConfigger::getInstance()->m_logPerFixSize << 20);
				if (fileSize < fixSize)
					break;
				m_curFileIdx++;
			}
		}
		~LogWriterPerFixedSize() override
		{
		}
		void write(const LogItem& logItem) override {
			std::string now = timeNow(true);
			std::string logName = LogConfigger::getInstance()->m_logRootFolder + "\\" + now + "_" + std::to_string(m_curFileIdx) + ".log";

			if (!m_ofs.is_open())
			{
				m_ofs.open(logName.c_str(), std::ios::out | std::ios::app);
				if (!m_ofs.is_open())
					return;
			}
			m_ofs << logItem.second;
			m_ofs.flush();

			//每次写完判断是否超出大小
			auto fileSize = GetFileSize(logName);
			if (fileSize >= (LogConfigger::getInstance()->m_logPerFixSize << 20))
			{
				printf("close file,m_curFileIdx:%d\n", m_curFileIdx);
				m_ofs.close();
				m_curFileIdx = (m_curFileIdx + 1) % LogConfigger::getInstance()->m_logPerFixSizeMaxCount;
			}
		}

	private:
		long long GetFileSize(const std::string& filename)
		{
			std::ifstream file(filename, std::ios::binary | std::ios::ate);
			if (!file.is_open()) {
				std::cerr << "Failed to open the file: " << filename << std::endl;
				return 0;
			}

			std::streampos fileSize = file.tellg(); // bytes
			file.close();

			return static_cast<long long>(fileSize);
		}
		std::ofstream m_ofs;
		int m_curFileIdx = 1;
	};

	class OfflineLogger
	{
	public:
		static OfflineLogger* getInstance()
		{
			static OfflineLogger o;
			std::call_once(m_initFlag, [&]() {
				o.run();
				});
			return &o;
		}

		void addMsg(const LogItem& msg)
		{
			if (!m_bRunning) return;
			std::lock_guard<std::mutex> lk(m_writerLock);
			m_logCaches.push(msg);
			m_cv.notify_all();
		}
		void stop()
		{
			m_bRunning = false;
			m_cv.notify_all();
		}
		~OfflineLogger()
		{
			stop();
			m_workerThread.join();
		}
	private:
		OfflineLogger()
		{
			auto logWriteType = LogConfigger::getInstance()->m_logWriterType;
			if (LogWriterType::PerDay == logWriteType)
				m_pLogWritter = std::make_shared<LogWriterPerDay>();
			else if (LogWriterType::PerFixedSize == logWriteType)
				m_pLogWritter = std::make_shared<LogWriterPerFixedSize>();
			else if (LogWriterType::PerLevel == logWriteType)
				m_pLogWritter = std::make_shared<LogWriterPerLevel>();
		}
		OfflineLogger(const OfflineLogger&) = delete;
		OfflineLogger operator = (const OfflineLogger&) = delete;
		void run()
		{
			if (m_bRunning) return;

			m_bRunning = true;
			m_workerThread = std::move(std::thread([&]() {
				while (m_bRunning)
				{
					std::unique_lock<std::mutex> lk(m_writerLock);
					while (m_bRunning && m_logCaches.size() == 0)
						m_cv.wait(lk, [&]() { return !m_bRunning || m_logCaches.size() != 0; });

					int size = m_logCaches.size();
					for (int i = 0; i < size; ++i)
					{
						try
						{
							if (m_pLogWritter)
								m_pLogWritter->write(m_logCaches.front());
						}
						catch (std::exception e)
						{
							char msg[1024];
							sprintf(msg, "%s write log exception: %s", LogConfigger::getInstance()->m_logTag, e.what());
							OutputDebugStringA(msg);
						}
						catch (...) {}
						m_logCaches.pop();
					}
				}
				}));
		}
	private:
		bool m_bRunning = false;
		std::thread m_workerThread;
		std::mutex m_writerLock;
		std::condition_variable m_cv;
		std::queue<LogItem> m_logCaches;
		static std::once_flag m_initFlag;
		std::shared_ptr<LogWriterBase> m_pLogWritter = nullptr;
	};
	std::once_flag  OfflineLogger::m_initFlag;

	extern "C" __declspec(dllexport) void LOG(LogLevel level, LogModule module, const char* szFileName, const char* szFuncName, int nLine, const char* format, ...) {

		if (LogConfigger::getInstance()->m_logLevel < (int)level ||
			(level == LogLevel::kLogError && (LogConfigger::getInstance()->m_logErrorMask & module) == 0) ||
			(level == LogLevel::kLogWarning && (LogConfigger::getInstance()->m_logWarningMask & module) == 0) ||
			(level == LogLevel::kLogInfo && (LogConfigger::getInstance()->m_logInfoMask & module) == 0) ||
			(level == LogLevel::kLogDebug && (LogConfigger::getInstance()->m_logDebugMask & module) == 0) ||
			(level == LogLevel::kLogVerbose && (LogConfigger::getInstance()->m_logVerboseMask & module) == 0))
		{
			return;
		}
		
		va_list args;
		va_start(args, format);
		char msg[4096] = { 0 };
		std::stringstream ss;
		ss << (std::this_thread::get_id());
		sprintf(msg, "%s %s [%s] [%s] %s %s:%d [%s] | ",
			timeNow().c_str(),
			LogConfigger::getInstance()->m_logTag,
			LogLevel2String(level).c_str(),
			LogModule2String(module).c_str(),
			GetFileName(const_cast<char*>(szFileName)),
			szFuncName,
			nLine,
			ss.str().c_str());
		//vsprintf(msg + strlen(msg), format, args);
		vsnprintf(msg + strlen(msg), sizeof(msg) - strlen(msg) - 1, format, args);
		strcat(msg, "\n");

		if ((LogConfigger::getInstance()->m_logOutputMode & LogOutputMode::Console) != 0)
			std::cout << msg;
		if ((LogConfigger::getInstance()->m_logOutputMode & LogOutputMode::SystemDebugger) != 0)
			OutputDebugStringA(msg);
		if ((LogConfigger::getInstance()->m_logOutputMode & LogOutputMode::File) != 0)
			OfflineLogger::getInstance()->addMsg({ {level,module},std::string(msg) });

		va_end(args);
	}

	extern "C" __declspec(dllexport) void LogInf(LogLevel level, LogModule module,char *file, char * func,int line, char* msg)
	{
		LOG(level, module, file, func, line, msg);
	}

	extern "C" __declspec(dllexport) void SetLogLevel(int level)
	{
		LogConfigger::getInstance()->m_logLevel = level;
	}

	extern "C" __declspec(dllexport) int GetLogLevel()
	{
		return LogConfigger::getInstance()->m_logLevel;
	}
}
