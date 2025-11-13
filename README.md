# Log_cpp
A highly flexible log library, implemented purely in C++. Configurable log levels (Error Warning Info Debug Verbose). The log output location can be configured (supporting output to the console, windows debugger, and in files). When outoutput to local files, it can also be configured to store by day, by level, by module, or by a fixed size. Which modules can be independently configured for output at different levels? For details, please refer to log.config


## How to use
First, based on the project, you can compile a dll. You need to copy log.config to the running directory, and configure the modules you need in the logModules tag, separated by "|". These names also need to correspond to the names in ModuleManager::init in logModule.h. Configure the LogModule enumeration simultaneously. At this point, the log library knows all the currently supported modules. Then, you still need to configure which levels of modules need to be enabled, such as logModules=Module1|Module2|Module3, logErrorMask=Module2. At this point, if LOGE is passed into Module1, it will not output. The modules supported in Mask are also separated by "|"

Suppose it is run with the following configuration and all modules are opened
``` cpp
logTag=vision
logRootFolder=.\log
//1 console, 2 system debugger, 4 file, use or to combine, for example, 3 means output to console and system debugger
logOutputMode=7
//0 error, 1 warrning, 2 info, 3 debug , 4 verbose, larger means more log detail
logLevel=4

//reference the module define in logModule.h
logErrorMask=Module1|Module2|Module3
logWarningMask=Module1|Module2|Module3
logInfoMask=Module1|Module2|Module3
logDebugMask=Module1|Module2|Module3
logVerboseMask=Module1|Module2|Module3

//0 perday, 1 perlevel, 2 per module, 3 perfixed size
logWriterType=0
//if use perfixed size to write, then use the logPerFixSize to control the file size, (unit is MB)
logPerFixSize=1

// config modules, need exist in logMudule.h ModuleManager
logModules=Module1|Module2|Module3

```

```cpp
int i = 10;
float j = 1.2;
_LOGI(LogModule::Module1, "this is a log sample,i: %d , j:%f", i, j);
```

you will get the log bellow
```cpp
2025-11-13 17:38:52:023 vision [I] [Module1] main.cpp testFunc:76 [24452] | this is a log sample,i: 10 , j:1.200000
```
the log format is: 
Time   logTag [logLevel] [logModule] fileName funcName:logLine [thread number] | ....log content... 

