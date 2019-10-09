// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/core/Log.hpp"

#include "opentxs/api/Settings.hpp"
#include "opentxs/core/util/OTPaths.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/Types.hpp"

#ifdef ANDROID
#include <android/log.h>
#endif
#include <sys/types.h>

#include <cstdarg>
#include <cstdint>
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <exception>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <typeinfo>

#define LOGFILE_PRE "log-"
#define LOGFILE_EXT ".log"
#define GLOBAL_LOGNAME "init"
#define GLOBAL_LOGFILE "init.log"
#define CONFIG_LOG_SECTION String::Factory("logging")
#define CONFIG_LOG_TO_FILE_KEY String::Factory("log_to_file")

namespace opentxs
{
Log* Log::pLogger{nullptr};
const OTString Log::m_strVersion = String::Factory(OPENTXS_VERSION_STRING);
const OTString Log::m_strPathSeparator = String::Factory("/");
LogSource LogOutput{-1};
LogSource LogNormal{0};
LogSource LogDetail{1};
LogSource LogVerbose{2};
LogSource LogDebug{3};
LogSource LogTrace{4};
LogSource LogInsane{5};

Log::Log(const api::Settings& config)
    : config_(config)
    , m_nLogLevel(0)
    , m_bInitialized(false)
    , write_log_file_(false)
    , m_strLogFileName(String::Factory())
    , m_strLogFilePath(String::Factory())
    , lock_()

{
    bool notUsed{false};
    config_.Check_bool(
        CONFIG_LOG_SECTION, CONFIG_LOG_TO_FILE_KEY, write_log_file_, notUsed);
}

//  OTLog Init, must run this before using any OTLog function.

// static
bool Log::Init(
    const api::Settings& config,
    const OTString strThreadContext,  //=String::Factory()
    const std::int32_t& nLogLevel)    //=0
{
    if (nullptr == pLogger) {
        pLogger = new Log(config);
        pLogger->m_bInitialized = false;
    }

    if (strThreadContext->Compare(GLOBAL_LOGNAME)) return false;

    if (!pLogger->m_bInitialized) {
        pLogger->m_nLogLevel = nLogLevel;
        LogSource::SetVerbosity(nLogLevel);

        if (!strThreadContext->Exists() ||
            strThreadContext->Compare(""))  // global
        {
            pLogger->m_strLogFileName->Set(GLOBAL_LOGFILE);
        } else  // not global
        {

            pLogger->m_strLogFileName->Format(
                "%s%s%s", LOGFILE_PRE, strThreadContext->Get(), LOGFILE_EXT);

            std::unique_ptr<api::Settings> globalConfig{
                Factory::Settings(OTPaths::GlobalConfigFile())};

            globalConfig->Reset();
            if (!globalConfig->Load()) { return false; }

            bool bIsNew(false);
            if (!globalConfig->CheckSet_str(
                    String::Factory("logfile"),
                    strThreadContext,
                    pLogger->m_strLogFileName,
                    pLogger->m_strLogFileName,
                    bIsNew)) {
                return false;
            }

            if (!globalConfig->Save()) { return false; }
            globalConfig->Reset();
        }

#ifdef ANDROID
        if (OTPaths::HomeFolder().Exists())
#endif
            if (!OTPaths::AppendFile(
                    pLogger->m_strLogFilePath,
                    OTPaths::AppDataFolder(),
                    pLogger->m_strLogFileName)) {
                return false;
            }

        pLogger->m_bInitialized = true;

        return true;
    } else {
        return false;
    }
}

// static
bool Log::IsInitialized()
{
    return nullptr != pLogger && pLogger->m_bInitialized;
}

// static
bool Log::Cleanup()
{
    if (nullptr != pLogger) {
        delete pLogger;
        pLogger = nullptr;
        return true;
    }
    return false;
}

// static
bool Log::CheckLogger(Log* logger)
{

    if (nullptr != logger) { rLock lock(Log::pLogger->lock_); }

    if (nullptr != logger && logger->m_bInitialized) { return true; }

    OT_FAIL;
}

// OTLog Constants.

// Compiled into OTLog:

const char* Log::Version() { return Log::GetVersion().Get(); }
const String& Log::GetVersion() { return m_strVersion; }

const char* Log::PathSeparator() { return Log::GetPathSeparator().Get(); }
const String& Log::GetPathSeparator() { return m_strPathSeparator; }

const char* Log::LogFilePath() { return Log::GetLogFilePath().Get(); }
const String& Log::GetLogFilePath() { return pLogger->m_strLogFilePath; }

// static
std::int32_t Log::LogLevel()
{
    if (nullptr != pLogger) {

        return pLogger->m_nLogLevel;
    } else {

        return 0;
    }
}

// static
bool Log::SetLogLevel(const std::int32_t& nLogLevel)
{
    if (nullptr == pLogger) {
        OT_FAIL;
    } else {
        pLogger->m_nLogLevel = nLogLevel;
        LogSource::SetVerbosity(nLogLevel);

        return true;
    }
}

//  OTLog Functions

// If there's no logfile, then send it to stderr.
// (So we can still see it on the screen, but it doesn't interfere with any
// command line utilities who might otherwise interpret it as their own input,
// if I was actually writing to stdout.)
//
// static
bool Log::LogToFile(const String& strOutput)
{
    // We now do this either way.
    {
        std::cerr << strOutput;
        std::cerr.flush();
    }

    // now log to file, if we can.

    bool bHaveLogger(false);
    if (nullptr != pLogger)
        if (pLogger->IsInitialized()) bHaveLogger = true;

    // lets check if we are Initialized in this context
    if (bHaveLogger) CheckLogger(Log::pLogger);

    if (nullptr != pLogger) { rLock lock(Log::pLogger->lock_); }

    bool bSuccess = false;

    if (bHaveLogger) {
        if (false == pLogger->write_log_file_) { return true; }

        // Append to logfile
        if ((strOutput.Exists()) &&
            (Log::pLogger->m_strLogFilePath->Exists())) {
            std::ofstream logfile;
            logfile.open(Log::LogFilePath(), std::ios::app);

            if (!logfile.fail()) {
                logfile << strOutput;
                logfile.close();
                bSuccess = true;
            }
        }
    }

    return bSuccess;
}

// static
bool Log::Sleep(const std::chrono::microseconds us)
{
    auto start = std::chrono::high_resolution_clock::now();
    auto end = start + us;
    do {
        std::this_thread::yield();
        std::this_thread::sleep_for(us);
    } while (std::chrono::high_resolution_clock::now() < end);
    return true;
}

// For normal output. The higher the verbosity, the less important the message.
// (Verbose level 0 ALWAYS logs.) Currently goes to stdout.

void Log::Output(std::int32_t nVerbosity, const char* szOutput)
{
    bool bHaveLogger(false);
    if (nullptr != pLogger)
        if (pLogger->IsInitialized()) bHaveLogger = true;

    // lets check if we are Initialized in this context
    if (bHaveLogger) CheckLogger(Log::pLogger);

    // If log level is 0, and verbosity of this message is 2, don't bother
    // logging it.
    //    if (nVerbosity > OTLog::__CurrentLogLevel || (nullptr == szOutput))
    if ((nVerbosity > LogLevel()) || (nullptr == szOutput) ||
        (LogLevel() == (-1)))
        return;

#ifndef ANDROID  // if NOT android
    LogToFile(String::Factory(szOutput));
#else  // if IS Android

    switch (nVerbosity) {
        case 0:
        case 1:
            __android_log_write(ANDROID_LOG_INFO, "OT Output", szOutput);
            break;
        case 2:
        case 3:
            __android_log_write(ANDROID_LOG_DEBUG, "OT Debug", szOutput);
            break;
        case 4:
        case 5:
            __android_log_write(ANDROID_LOG_VERBOSE, "OT Verbose", szOutput);
            break;
        default:
            __android_log_write(ANDROID_LOG_UNKNOWN, "OT Unknown", szOutput);
            break;
    }
#endif
}

// the vOutput is to avoid name conflicts.
void Log::vOutput(std::int32_t nVerbosity, const char* szOutput, ...)
{
    bool bHaveLogger(false);
    if (nullptr != pLogger)
        if (pLogger->IsInitialized()) bHaveLogger = true;

    // lets check if we are Initialized in this context
    if (bHaveLogger) CheckLogger(Log::pLogger);

    if (nullptr != pLogger) { rLock lock(Log::pLogger->lock_); }

    // If log level is 0, and verbosity of this message is 2, don't bother
    // logging it.
    if (((0 != LogLevel()) && (nVerbosity > LogLevel())) ||
        (nullptr == szOutput))
        return;

    va_list args;
    va_start(args, szOutput);

    std::string strOutput;

    const bool bFormatted = String::vformat(szOutput, &args, strOutput);

    va_end(args);

    if (bFormatted)
        Log::Output(nVerbosity, strOutput.c_str());
    else
        OT_FAIL;
    return;
}

// the vError name is to avoid name conflicts
void Log::vError(const char* szError, ...)
{
    bool bHaveLogger(false);
    if (nullptr != pLogger)
        if (pLogger->IsInitialized()) bHaveLogger = true;

    // lets check if we are Initialized in this context
    if (bHaveLogger) CheckLogger(Log::pLogger);

    if (nullptr != pLogger) { rLock lock(Log::pLogger->lock_); }

    if ((nullptr == szError)) return;

    va_list args;
    va_start(args, szError);

    std::string strOutput;

    const bool bFormatted = String::vformat(szError, &args, strOutput);

    va_end(args);

    if (bFormatted)
        Log::Error(strOutput.c_str());
    else
        OT_FAIL;
}

// An error has occurred, that somehow doesn't match the Assert or Output
// functions.
// So use this one instead.  This ALWAYS logs and currently it all goes to
// stderr.

void Log::Error(const char* szError)
{
    bool bHaveLogger(false);
    if (nullptr != pLogger)
        if (pLogger->IsInitialized()) bHaveLogger = true;

    // lets check if we are Initialized in this context
    if (bHaveLogger) CheckLogger(Log::pLogger);

    if (nullptr != pLogger) { rLock lock(Log::pLogger->lock_); }

    if ((nullptr == szError)) return;

#ifndef ANDROID  // if NOT android

    LogToFile(String::Factory(szError));

#else  // if Android
    __android_log_write(ANDROID_LOG_ERROR, "OT Error", szError);
#endif
}

// String Helpers

bool Log::StringFill(
    String& out_strString,
    const char* szString,
    std::int32_t iLength,
    const char* szAppend)
{
    std::string strString(szString);

    if (nullptr != szAppend) strString.append(szAppend);

    for (; (static_cast<std::int32_t>(strString.length()) < iLength);
         strString.append(" "))
        ;

    out_strString.Set(strString.c_str());

    return true;
}
}  // namespace opentxs
