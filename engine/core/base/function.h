#pragma once

#include "echo_def.h"

// 自定义sprintf,移除平台相关警告
int lord_sprintf(char* buff, const int bufflen, const char* const format, ...);

// 自定义sscanf,移除平台相关警告
int lord_sscanf(const char* buff, const char* format, ...);