
// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

#import <Foundation/Foundation.h>
#include "comm/xlogger/xlogger.h"
#include "comm/xlogger/loginfo_extract.h"
#import "comm/objc/scope_autoreleasepool.h"

void ConsoleLog(const XLoggerInfo* _info, const char* _log)
{
    SCOPE_POOL();

    if (NULL==_info || NULL==_log) return;
    
    static const char* levelStrings[] = {
        "V",
        "D",  // debug
        "I",  // info
        "W",  // warn
        "E",  // error
        "F"  // fatal
    };
    
    const char* strFuncName  = NULL == _info->func_name ? "" : _info->func_name;
    const char* file_name = ExtractFileName(_info->filename);

    char temp_time[64] = {0};

    if (0 != _info->timeval.tv_sec) {
        time_t sec = _info->timeval.tv_sec;
        tm tm = *localtime((const time_t*)&sec);
#ifdef ANDROID
            snprintf(temp_time, sizeof(temp_time), "%d-%02d-%02d %+.1f %02d:%02d:%02d.%.3ld", 1900 + tm.tm_year, 1 + tm.tm_mon, tm.tm_mday,
                     tm.tm_gmtoff / 3600.0, tm.tm_hour, tm.tm_min, tm.tm_sec, _info->timeval.tv_usec / 1000);
#elif _WIN32
            snprintf(temp_time, sizeof(temp_time), "%d-%02d-%02d %+.1f %02d:%02d:%02d.%.3d", 1900 + tm.tm_year, 1 + tm.tm_mon, tm.tm_mday,
                     (-_timezone) / 3600.0, tm.tm_hour, tm.tm_min, tm.tm_sec, _info->timeval.tv_usec / 1000);
#else
            snprintf(temp_time, sizeof(temp_time), "%d-%02d-%02d %+.1f %02d:%02d:%02d.%.3d", 1900 + tm.tm_year, 1 + tm.tm_mon, tm.tm_mday,
                     tm.tm_gmtoff / 3600.0, tm.tm_hour, tm.tm_min, tm.tm_sec, _info->timeval.tv_usec / 1000);
#endif
        }
    
    char log[256 * 1024] = {0};
    snprintf(log, sizeof(log), "[%s][%s][%s][%s, %s, %d][%s", levelStrings[_info->level], temp_time, NULL == _info->tag ? "" : _info->tag, file_name, strFuncName, _info->line, _log);
    
    printf("%s\n", log);
}
