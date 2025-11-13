#include "log.h"
#include <sstream>

namespace TinyLog
{
	LogConfigger::LogConfigger()
	{
		m_logTag = "Vision";
		m_logRootFolder = ".\\log";
		m_logOutputMode = 2; 
		m_logLevel = 2;
		m_logWriterType = LogWriterType::PER_DAY;
		m_logPerFixSize = 0;
		m_logPerFixSizeMaxCount = ULLONG_MAX;

		parseConfigFile(".\\log.config");
	}
	void LogConfigger::parseConfigFile(const std::string& filename)
	{
		//writen by gpt
		std::ifstream configFile(filename);
		if (!configFile.is_open()) {
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

	std::vector<std::string> LogConfigger::splitString(const std::string& str, char delimiter) {
		std::vector<std::string> tokens;
		std::string token;
		std::istringstream tokenStream(str);

		// 使用getline和istringstream分割字符串  
		while (std::getline(tokenStream, token, delimiter)) {
			tokens.push_back(token);
		}

		return tokens;
	}

	void LogConfigger::assignConfigValue(const std::string& key, const  std::string& value)
	{
		auto assignMask = [&](ModuleMask &mask) {
			auto moduleNames = splitString(value, '|');
			for (const auto& n : moduleNames)
				mask[n] = true;
		};

		if ("logRootFolder" == key)
			m_logRootFolder = value;
		else if ("logTag" == key)
			m_logTag = value;
		else if ("logOutputMode" == key)
			m_logOutputMode = std::stoull(value);
		else if ("logLevel" == key)
			m_logLevel = std::stoi(value);
		else if ("logErrorMask" == key)
			assignMask(m_errorMask);
		else if ("logWarningMask" == key)
			assignMask(m_warningMask);
		else if ("logInfoMask" == key)
			assignMask(m_infoMask);
		else if ("logDebugMask" == key)
			assignMask(m_debugMask);
		else if ("logVerboseMask" == key)
			assignMask(m_verboseMask);
		else if ("logWriterType" == key)
			m_logWriterType = (LogWriterType)std::stoi(value);
		else if ("logPerFixSize" == key)
			m_logPerFixSize = std::stoull(value);
		else if ("logModules" == key)
			m_moduleNames = splitString(value, '|');
	}
}