#pragma once
#include <unordered_map>
#include <log_cpp/log.h>
using namespace TinyLog;


enum class LogModule : int
{
	Module1 = 0,
	Module2,
	Module3,
};

class ModuleManager
{
public:
	static ModuleManager* getIns() {
		static ModuleManager o;
		o.init();
		return &o;
	 }
	void init()
	{
		m_idMap[(int)LogModule::TERMINAL] = iTinyLog_GetLogModuleIndex("Module1");
		m_idMap[(int)LogModule::CONTACT_ANGLE] = iTinyLog_GetLogModuleIndex("Module2");
		m_idMap[(int)LogModule::CELL_SEG] = iTinyLog_GetLogModuleIndex("Module3");
		//m_idMap[(int)LogModule::Module2] = GetLogModuleIndex("Module2");
	}
	inline int getModuleIdx(LogModule module)
	{
		return m_idMap[(int)module];
	}
private:
	std::unordered_map<int, int> m_idMap;
};

#define _LOGE(_module, ...) LOGE(ModuleManager::getIns()->getModuleIdx(_module), __VA_ARGS__)
#define _LOGW(_module, ...) LOGW(ModuleManager::getIns()->getModuleIdx(_module), __VA_ARGS__)
#define _LOGI(_module, ...) LOGI(ModuleManager::getIns()->getModuleIdx(_module), __VA_ARGS__)
#define _LOGD(_module, ...) LOGD(ModuleManager::getIns()->getModuleIdx(_module), __VA_ARGS__)
#define _LOGV(_module, ...) LOGV(ModuleManager::getIns()->getModuleIdx(_module), __VA_ARGS__)

#define LOG_AND_RETURN_FALSE_IF_COND(logModule, cond,...) \
if(cond) { \
	_LOGE(logModule, __VA_ARGS__); \
	return false; \
}

#define LOG_AND_RETURN_RESULT_IF_COND(logModule,cond, result, ...) \
if(cond) { \
	_LOGE(logModule, __VA_ARGS__); \
	return result; \
}