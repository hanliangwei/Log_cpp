#include "pch.h"
#include "log.h"


namespace TinyLOG
{

	void LogConfigger::ParseConfigFile(const std::string& filename)
	{
		//writen by gpt
		std::ifstream configFile(filename);
		if (!configFile.is_open()) {
			//LOGE(LogModule::kModuleAll, "Failed to open config file:%s", filename.c_str());
			std::cout << "Failed to open file: " << filename << std::endl;
			return;
		}

		std::string line;
		while (std::getline(configFile, line)) {
			// 忽略空白行
			if (line.empty()) {
				continue;
			}
			line.erase(0, line.find_first_not_of(' '));
			line.erase(line.find_last_not_of(' ') + 1);
			// 忽略注释行
			if (line.substr(0, 2) == "//") {
				continue;
			}

			// 查找等号位置
			size_t equalPos = line.find('=');
			if (equalPos == std::string::npos) {
				continue;  // 配置格式不正确，忽略该行
			}

			std::string key = line.substr(0, equalPos);
			std::string value = line.substr(equalPos + 1);

			// 去除首尾空白字符
			key.erase(0, key.find_first_not_of(' '));
			key.erase(key.find_last_not_of(' ') + 1);
			value.erase(0, value.find_first_not_of(' '));
			value.erase(value.find_last_not_of(' ') + 1);
			assignConfigValue(key, value);
		}
		configFile.close();

	}

	void LogConfigger::assignConfigValue(const std::string& key, const  std::string& value)
	{
		if ("logRootFolder" == key)
			m_logRootFolder = value;
		else if ("logTag" == key)
			m_logTag = value;
		else if ("logOutputMode" == key)
			m_logOutputMode = std::stoull(value);
		else if ("logLevel" == key)
			m_logLevel = std::stoi(value);
		else if ("logErrorMask" == key)
			m_logErrorMask = std::stoull(value);
		else if ("logWarningMask" == key)
			m_logWarningMask = std::stoull(value);
		else if ("logInfoMask" == key)
			m_logInfoMask = std::stoull(value);
		else if ("logDebugMask" == key)
			m_logDebugMask = std::stoull(value);
		else if ("logVerboseMask" == key)
			m_logVerboseMask = std::stoull(value);
		else if ("logWriterType" == key)
			m_logWriterType = (LTLOG::LogWriterType)std::stoi(value);
		else if ("logPerFixSize" == key)
			m_logPerFixSize = std::stoull(value);
	}

}
