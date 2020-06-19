// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

/*
 * @Author: garryyan
 * @LastEditors: garryyan
 * @Date: 2019-03-05 15:10:24
 * @LastEditTime: 2019-03-05 15:10:50
 */


#include "mars/log/xlogger_interface.h"

#include <functional>
#include <map>
#include "mars/comm/thread/mutex.h"
#include "mars/comm/thread/lock.h"
#include "mars/comm/xlogger/xlogger_category.h"
#include "mars/log/appender.h"

using namespace mars::comm;
namespace mars {
namespace xlog {

static Mutex sg_mutex;
static std::map<std::string, XloggerCategory*> sg_map;
XloggerCategory* NewXloggerInstance(TLogLevel _level, TAppenderMode _mode, const char* _cachedir,
                                    const char* _logdir, const char* _nameprefix, int _cache_days,
                                    const char* _pub_key) {

    if (nullptr == _logdir || nullptr == _nameprefix) {
        return nullptr;
    }

    ScopedLock lock(sg_mutex);
    auto it = sg_map.find(_nameprefix);
    if (it != sg_map.end()) {
        return it->second;
    }

    XloggerAppender* appender = XloggerAppender::NewInstance(_mode, _cachedir,
                                                            _logdir, _nameprefix,
                                                            _cache_days, _pub_key);

    using namespace std::placeholders;
    XloggerCategory* category = XloggerCategory::NewInstance(reinterpret_cast<uintptr_t>(appender),
                                                                std::bind(&XloggerAppender::Write, appender, _1, _2));
    category->SetLevel(_level);
    sg_map[_nameprefix] = category;
    return category;
}

mars::comm::XloggerCategory* GetXloggerInstance(const char* _nameprefix) {
    if (nullptr == _nameprefix) {
        return nullptr;
    }

    ScopedLock lock(sg_mutex);
    auto it = sg_map.find(_nameprefix);
    if (it != sg_map.end()) {
        return it->second;
    }

    return nullptr;
}

void ReleaseXloggerInstance(const char* _nameprefix) {
    if (nullptr == _nameprefix) {
        return;
    }

    ScopedLock lock(sg_mutex);
    auto it = sg_map.find(_nameprefix);
    if (it == sg_map.end()) {
        return;
    }

    XloggerCategory* category  = it->second;
    XloggerAppender* appender = reinterpret_cast<XloggerAppender*>(category->GetAppender());
    XloggerAppender::DelayRelease(appender);
    XloggerCategory::DelayRelease(category);
    sg_map.erase(it);
}

void XloggerWrite(int64_t _instance_ptr, const XLoggerInfo* _info, const char* _log) {
    if (_instance_ptr < 0) {
        return;
    }

    if (0 == _instance_ptr) {
        xlogger_Write(_info, _log);
    } else {
        XloggerCategory* category  = reinterpret_cast<XloggerCategory*>(_instance_ptr);
        category->Write(_info, _log);
    }
}

bool IsEnabledFor(int64_t _instance_ptr, TLogLevel _level) {
    if (_instance_ptr < 0) {
        return false;
    }

    if (0 == _instance_ptr) {
        return xlogger_IsEnabledFor(_level);
    } else {
        XloggerCategory* category  = reinterpret_cast<XloggerCategory*>(_instance_ptr);
        return category->IsEnabledFor(_level);
    }
}

TLogLevel GetLevel(int64_t _instance_ptr) {
    if (_instance_ptr < 0) {
        return kLevelNone;
    }

    if (0 == _instance_ptr) {
        return xlogger_Level();
    } else {
        XloggerCategory* category  = reinterpret_cast<XloggerCategory*>(_instance_ptr);
        TLogLevel level = category->GetLevel();
        return level;
    }
}

void SetLevel(int64_t _instance_ptr, TLogLevel _level) {
    if (_instance_ptr < 0) {
        return;
    }

    if (0 == _instance_ptr) {
        xlogger_SetLevel(_level);
    } else {
        XloggerCategory* category  = reinterpret_cast<XloggerCategory*>(_instance_ptr);
        return category->SetLevel(_level);
    }
}

void SetAppenderMode(int64_t _instance_ptr, TAppenderMode _mode) {
    if (_instance_ptr < 0) {
        return;
    }

    if (0 == _instance_ptr) {
        appender_setmode(_mode);
    } else {
        XloggerCategory* category  = reinterpret_cast<XloggerCategory*>(_instance_ptr);
        XloggerAppender* appender = reinterpret_cast<XloggerAppender*>(category->GetAppender());
        return appender->SetMode(_mode);
    }
}

void Flush(int64_t _instance_ptr, bool _is_sync) {
    if (_instance_ptr < 0) {
        return;
    }

    if (0 == _instance_ptr) {
        _is_sync ? appender_flush_sync() : appender_flush();
    } else {
        XloggerCategory* category  = reinterpret_cast<XloggerCategory*>(_instance_ptr);
        XloggerAppender* appender = reinterpret_cast<XloggerAppender*>(category->GetAppender());
        _is_sync ? appender->FlushSync() : appender->Flush();
    }
}

void SetConsoleLogOpen(int64_t _instance_ptr, bool _is_open) {
    if (_instance_ptr < 0) {
        return;
    }

    if (0 == _instance_ptr) {
       appender_set_console_log(_is_open);
    } else {
        XloggerCategory* category  = reinterpret_cast<XloggerCategory*>(_instance_ptr);
        XloggerAppender* appender = reinterpret_cast<XloggerAppender*>(category->GetAppender());
        return appender->SetConsoleLog(_is_open);
    }
}

void SetMaxFileSize(int64_t _instance_ptr, long _max_file_size) {
    if (_instance_ptr < 0) {
        return;
    }

    if (0 == _instance_ptr) {
       appender_set_max_file_size(_max_file_size);
    } else {
        XloggerCategory* category  = reinterpret_cast<XloggerCategory*>(_instance_ptr);
        XloggerAppender* appender = reinterpret_cast<XloggerAppender*>(category->GetAppender());
        return appender->SetMaxFileSize(_max_file_size);
    }
}

void SetMaxAliveTime(int64_t _instance_ptr, long _alive_seconds) {
    if (_instance_ptr < 0) {
        return;
    }

    if (0 == _instance_ptr) {
       appender_set_max_alive_duration(_alive_seconds);
    } else {
        XloggerCategory* category  = reinterpret_cast<XloggerCategory*>(_instance_ptr);
        XloggerAppender* appender = reinterpret_cast<XloggerAppender*>(category->GetAppender());
        return appender->SetMaxAliveDuration(_alive_seconds);
    }
}

bool Getfilepath_from_timespan(int64_t _instance_ptr, int _timespan, const char* _prefix, std::vector<std::string>& _filepath_vec)
{
    if (_instance_ptr < 0) {
        return false;
    }

    if (0 == _instance_ptr) {
       return appender_getfilepath_from_timespan(_timespan, _prefix, _filepath_vec);
    } else {
        XloggerCategory* category  = reinterpret_cast<XloggerCategory*>(_instance_ptr);
        XloggerAppender* appender = reinterpret_cast<XloggerAppender*>(category->GetAppender());
        return appender->GetfilepathFromTimespan(_timespan, _prefix, _filepath_vec);
    }
}

}
}

