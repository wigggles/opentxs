// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/Log.hpp"

#include "opentxs/api/Settings.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/util/OTPaths.hpp"
#include "opentxs/core/util/stacktrace.h"
#include "opentxs/core/String.hpp"
#include "opentxs/Types.hpp"

#include "Internal.hpp"
#include "Factory.hpp"

#ifndef _WIN32
#include <unistd.h>
#include <cerrno>
#endif

#include <sys/types.h>
#include <cxxabi.h>

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
#include <string>
#include <thread>
#include <typeinfo>

#define LOG_DEQUE_SIZE 1024

extern "C" {

#ifdef _WIN32

#include <sys/timeb.h>

void LogStackFrames(void* FaultAdress, char*);

#else  // else if NOT _WIN32

// These added for the signal handling:

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#if defined(__APPLE__)
// Fucking Apple!
struct sigcontext {
    // cppcheck-suppress unusedStructMember
    std::int32_t eip;
};
#endif  // defined __APPLE__

#if !defined(ANDROID)
#include <execinfo.h>
#include <signal.h>
#if not defined(__APPLE__)
#include <ucontext.h>
#endif
#endif

//#endif

#endif  // not _WIN32

}  // extern C

#ifdef ANDROID
#include <android/log.h>
#endif

#define LOGFILE_PRE "log-"
#define LOGFILE_EXT ".log"
#define GLOBAL_LOGNAME "init"
#define GLOBAL_LOGFILE "init.log"
#define CONFIG_LOG_SECTION "logging"
#define CONFIG_LOG_TO_FILE_KEY "log_to_file"

//  OTLog Static Variables and Constants.

namespace opentxs
{

Log* Log::pLogger{nullptr};

const String Log::m_strVersion = OPENTXS_VERSION_STRING;
const String Log::m_strPathSeparator = "/";

OTLOG_IMPORT OTLogStream otErr(-1);  // logs using otErr << )
OTLOG_IMPORT OTLogStream otInfo(2);  // logs using OTLog::vOutput(2)
OTLOG_IMPORT OTLogStream otOut(0);   // logs using OTLog::vOutput(0)
OTLOG_IMPORT OTLogStream otWarn(1);  // logs using OTLog::vOutput(1)
OTLOG_IMPORT OTLogStream otLog3(3);  // logs using OTLog::vOutput(3)
OTLOG_IMPORT OTLogStream otLog4(4);  // logs using OTLog::vOutput(4)
OTLOG_IMPORT OTLogStream otLog5(5);  // logs using OTLog::vOutput(5)

OTLogStream::OTLogStream(int _logLevel)
    : std::ostream(this)
    , logLevel(_logLevel)
    , next(0)
    , pBuffer(new char[1024])
{
}

OTLogStream::~OTLogStream()
{
    delete[] pBuffer;
    pBuffer = nullptr;
}

int OTLogStream::overflow(int c)
{
    rLock lock(lock_);

    pBuffer[next++] = c;
    if (c != '\n' && next < 1000) { return 0; }

    pBuffer[next++] = '\0';
    next = 0;

    if (logLevel < 0) {
        Log::Error(pBuffer);
        return 0;
    }

    Log::Output(logLevel, pBuffer);
    return 0;
}

Log::Log(const api::Settings& config)
    : config_(config)
{
    bool notUsed{false};
    config_.Check_bool(
        CONFIG_LOG_SECTION, CONFIG_LOG_TO_FILE_KEY, write_log_file_, notUsed);
}

//  OTLog Init, must run this before using any OTLog function.

// static
bool Log::Init(
    const api::Settings& config,
    const OTString strThreadContext, //=String::Factory()
    const std::int32_t& nLogLevel) //=0
{
    if (nullptr == pLogger) {
        pLogger = new Log(config);
        pLogger->m_bInitialized = false;
    }

    if (strThreadContext->Compare(GLOBAL_LOGNAME)) return false;

    if (!pLogger->m_bInitialized) {
        pLogger->logDeque = std::deque<String*>();
        pLogger->m_strThreadContext.Set(strThreadContext);

        pLogger->m_nLogLevel = nLogLevel;

        if (!strThreadContext->Exists() ||
            strThreadContext->Compare(""))  // global
        {
            pLogger->m_strLogFileName.Set(GLOBAL_LOGFILE);
        } else  // not global
        {

            pLogger->m_strLogFileName.Format(
                "%s%s%s", LOGFILE_PRE, strThreadContext->Get(), LOGFILE_EXT);

            std::unique_ptr<api::Settings> config{
                Factory::Settings(OTPaths::GlobalConfigFile())};

            config->Reset();
            if (!config->Load()) { return false; }

            bool bIsNew(false);
            if (!config->CheckSet_str(
                    String::Factory("logfile"),
                    strThreadContext,
                    pLogger->m_strLogFileName,
                    pLogger->m_strLogFileName,
                    bIsNew)) {
                return false;
            }

            if (!config->Save()) { return false; }
            config->Reset();
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

        // Set the new log-assert function pointer.
        Assert* pLogAssert = new Assert(Log::logAssert);
        std::swap(pLogAssert, Assert::s_pAssert);
        delete pLogAssert;
        pLogAssert = nullptr;

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
bool Log::CheckLogger(Log* pLogger)
{

    if (nullptr != pLogger) { rLock lock(Log::pLogger->lock_); }

    if (nullptr != pLogger && pLogger->m_bInitialized) return true;

    OT_FAIL;
}

// OTLog Constants.

// Compiled into OTLog:

const char* Log::Version() { return Log::GetVersion().Get(); }
const String& Log::GetVersion() { return m_strVersion; }

const char* Log::PathSeparator() { return Log::GetPathSeparator().Get(); }
const String& Log::GetPathSeparator() { return m_strPathSeparator; }

// Set in constructor:

const String& Log::GetThreadContext() { return pLogger->m_strThreadContext; }

const char* Log::LogFilePath() { return Log::GetLogFilePath().Get(); }
const String& Log::GetLogFilePath() { return pLogger->m_strLogFilePath; }

// static
std::int32_t Log::LogLevel()
{
    if (nullptr != pLogger)
        return pLogger->m_nLogLevel;
    else
        return 0;
}

// static
bool Log::SetLogLevel(const std::int32_t& nLogLevel)
{
    if (nullptr == pLogger) {
        OT_FAIL;
    } else {
        pLogger->m_nLogLevel = nLogLevel;
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
        if ((strOutput.Exists()) && (Log::pLogger->m_strLogFilePath.Exists())) {
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

String Log::GetMemlogAtIndex(std::int32_t nIndex)
{
    // lets check if we are Initialized in this context
    CheckLogger(Log::pLogger);

    if (nullptr != pLogger) { rLock lock(Log::pLogger->lock_); }

    std::uint32_t uIndex = static_cast<uint32_t>(nIndex);

    if ((nIndex < 0) || (uIndex >= Log::pLogger->logDeque.size())) {
        otErr << __FUNCTION__ << ": index out of bounds: " << nIndex << "\n";
        return "";
    }

    if (nullptr != Log::pLogger->logDeque.at(uIndex))
        ;  // check for null
    else
        OT_FAIL;

    const String strLogEntry = *Log::pLogger->logDeque.at(uIndex);

    if (strLogEntry.Exists())
        return strLogEntry;
    else
        return "";
}

// We keep 1024 logs in memory, to make them available via the API.

std::int32_t Log::GetMemlogSize()
{
    // lets check if we are Initialized in this context
    CheckLogger(Log::pLogger);

    if (nullptr != pLogger) { rLock lock(Log::pLogger->lock_); }

    return static_cast<std::int32_t>(Log::pLogger->logDeque.size());
}

String Log::PeekMemlogFront()
{
    // lets check if we are Initialized in this context
    CheckLogger(Log::pLogger);

    if (nullptr != pLogger) { rLock lock(Log::pLogger->lock_); }

    if (Log::pLogger->logDeque.size() <= 0) return nullptr;

    if (nullptr != Log::pLogger->logDeque.front())
        ;  // check for null
    else
        OT_FAIL;

    const String strLogEntry = *Log::pLogger->logDeque.front();

    if (strLogEntry.Exists())
        return strLogEntry;
    else
        return "";
}

String Log::PeekMemlogBack()
{
    // lets check if we are Initialized in this context
    CheckLogger(Log::pLogger);

    if (nullptr != pLogger) { rLock lock(Log::pLogger->lock_); }

    if (Log::pLogger->logDeque.size() <= 0) return nullptr;

    if (nullptr != Log::pLogger->logDeque.back())
        ;  // check for null
    else
        OT_FAIL;

    const String strLogEntry = *Log::pLogger->logDeque.back();

    if (strLogEntry.Exists())
        return strLogEntry;
    else
        return "";
}

// static
bool Log::PopMemlogFront()
{
    // lets check if we are Initialized in this context
    CheckLogger(Log::pLogger);

    if (nullptr != pLogger) { rLock lock(Log::pLogger->lock_); }

    if (Log::pLogger->logDeque.size() <= 0) return false;

    String* strLogFront = Log::pLogger->logDeque.front();
    if (nullptr != strLogFront) delete strLogFront;
    strLogFront = nullptr;

    Log::pLogger->logDeque.pop_front();

    return true;
}

// static
bool Log::PopMemlogBack()
{
    // lets check if we are Initialized in this context
    CheckLogger(Log::pLogger);

    if (nullptr != pLogger) { rLock lock(Log::pLogger->lock_); }

    if (Log::pLogger->logDeque.size() <= 0) return false;

    String* strLogBack = Log::pLogger->logDeque.back();
    if (nullptr != strLogBack) delete strLogBack;
    strLogBack = nullptr;

    Log::pLogger->logDeque.pop_back();

    return true;
}

// static
bool Log::PushMemlogFront(const String& strLog)
{
    // lets check if we are Initialized in this context
    CheckLogger(Log::pLogger);

    if (nullptr != pLogger) { rLock lock(Log::pLogger->lock_); }

    OT_ASSERT(strLog.Exists());

    Log::pLogger->logDeque.push_front(new String(strLog));

    if (Log::pLogger->logDeque.size() > LOG_DEQUE_SIZE) {
        Log::PopMemlogBack();  // We start removing from the back when it
                               // reaches this size.
    }

    return true;
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

// This function is for things that should NEVER happen.
// In fact you should never even call it -- use the OT_ASSERT() macro instead.
// This Function is now only for logging, you
// static private
size_t Log::logAssert(
    const char* szFilename,
    size_t nLinenumber,
    const char* szMessage)
{
    if (nullptr != szMessage) {
#ifndef ANDROID  // if NOT android
        std::cerr << szMessage << "\n";

        LogToFile(szMessage);
        LogToFile("\n");

#else  // if Android
        __android_log_write(
            ANDROID_LOG_FATAL, "OT Assert (or Fail)", szMessage);
#endif

        print_stacktrace();
    }

    if ((nullptr != szFilename)) {
#ifndef ANDROID  // if NOT android

        // Pass it to LogToFile, as this always logs.
        //
        String strTemp;
        strTemp.Format(
            "\nOT_ASSERT in %s at line %" PRI_SIZE "\n",
            szFilename,
            nLinenumber);
        LogToFile(strTemp.Get());

#else  // if Android
        String strAndroidAssertMsg;
        strAndroidAssertMsg.Format(
            "\nOT_ASSERT in %s at line %d\n", szFilename, nLinenumber);
        __android_log_write(
            ANDROID_LOG_FATAL, "OT Assert", strAndroidAssertMsg.Get());
#endif
    }

    print_stacktrace();

    return 1;  // normal
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

    // We store the last 1024 logs so programmers can access them via the API.
    if (bHaveLogger) Log::PushMemlogFront(szOutput);

#ifndef ANDROID  // if NOT android

    LogToFile(szOutput);

#else  // if IS Android
    /*
    typedef enum android_LogPriority {
    ANDROID_LOG_UNKNOWN = 0,
    ANDROID_LOG_DEFAULT,    // only for SetMinPriority()
    ANDROID_LOG_VERBOSE,
    ANDROID_LOG_DEBUG,
    ANDROID_LOG_INFO,
    ANDROID_LOG_WARN,
    ANDROID_LOG_ERROR,
    ANDROID_LOG_FATAL,
    ANDROID_LOG_SILENT,     // only for SetMinPriority(); must be last
    } android_LogPriority;
    */
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

    // We store the last 1024 logs so programmers can access them via the API.
    if (bHaveLogger) Log::PushMemlogFront(szError);

#ifndef ANDROID  // if NOT android

    LogToFile(szError);

#else  // if Android
    __android_log_write(ANDROID_LOG_ERROR, "OT Error", szError);
#endif
}

// NOTE: if you have problems compiling on certain platforms, due to the use
// of errno, then just use preprocessor directives to carve those portions out
// of this function, replacing with a message about the unavailability of errno.
//
// static
void Log::Errno(const char* szLocation)  // stderr
{
    bool bHaveLogger(false);
    if (nullptr != pLogger)
        if (pLogger->IsInitialized()) bHaveLogger = true;

    // lets check if we are Initialized in this context
    if (bHaveLogger) CheckLogger(Log::pLogger);

    if (nullptr != pLogger) { rLock lock(Log::pLogger->lock_); }

    const std::int32_t errnum = errno;
    char buf[128];
    buf[0] = '\0';

    std::int32_t nstrerr = 0;
    char* szErrString = nullptr;

#if defined(_GNU_SOURCE) && defined(__linux__) && !defined(ANDROID)
    szErrString = strerror_r(errnum, buf, 127);
#elif defined(_POSIX_C_SOURCE)
#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600)
    // (strerror_r is threadsafe version of strerror)
    nstrerr = strerror_r(errnum, buf, 127);
#endif
#endif

    const char* szFunc = "OTLog::Errno";
    const char* sz_location = (nullptr == szLocation) ? "" : szLocation;

    if (nullptr == szErrString) szErrString = buf;

    if (0 == nstrerr)
        otErr << szFunc << " " << sz_location << ": errno " << errnum << ": "
              << (szErrString[0] != '\0' ? szErrString : "") << ".\n";
    else
        otErr << szFunc << " " << sz_location << ": errno: " << errnum
              << ". (Unable to retrieve error string for that number.)\n";
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

// SIGNALS
//
// To get the most mileage out of this signal handler,
// compile it with the options:  -g -rdynamic
//
//  Signal Handler
//
//

void ot_terminate(void);

namespace
{

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4800)  // warning C4800: forcing constant value.
#endif

// invoke set_terminate as part of global constant initialization
static const bool SET_TERMINATE __attribute__((unused)) =
    std::set_terminate(ot_terminate);

#ifdef _MSC_VER
#pragma warning(pop)
#endif
}  // namespace

// This is our custom std::terminate(). Also called for uncaught exceptions.
void ot_terminate()
{
// exception header is needed, on latter android NDK the header is not available
// see https://code.google.com/p/android/issues/detail?id=62648
#if !defined(ANDROID) || defined(_EXCEPTION_PTR_H)
    if (auto e = std::current_exception()) {
        try {
            std::rethrow_exception(e);
        } catch (const std::exception& e2) {
            std::cerr << "ot_terminate: " << __FUNCTION__
                      << " caught unhandled exception."
                      << " type: " << typeid(e2).name()
                      << " what(): " << e2.what() << std::endl;
        } catch (...) {
            std::cerr << "ot_terminate: " << __FUNCTION__
                      << " caught unknown/unhandled exception." << std::endl;
        }
    }
#endif
    print_stacktrace();

    // Call the default std::terminate() handler.
    std::abort();
}
}  // namespace opentxs

#ifdef _WIN32  // Windows SIGNALS
/////////////////////////////////////////////////////////////////////////////
// Unwind the stack and save its return addresses to the logfile
/////////////////////////////////////////////////////////////////////////////

void LogStackFrames(void* FaultAdress, char* eNextBP)

{
#if defined(_WIN64)

    typedef USHORT(WINAPI * CaptureStackBackTraceType)(
        __in ULONG, __in ULONG, __out PVOID*, __out_opt PULONG);

    HMODULE lLoadedLib = LoadLibrary(L"kernel32.dll");
    if (nullptr == lLoadedLib) OT_FAIL;
    CaptureStackBackTraceType func = (CaptureStackBackTraceType)(
        GetProcAddress(lLoadedLib, "RtlCaptureStackBackTrace"));

    if (func == nullptr) return;

    // Quote from Microsoft Documentation:
    // ## Windows Server 2003 and Windows XP:
    // ## The sum of the FramesToSkip and FramesToCapture parameters must be
    // less than 63.
    const std::int32_t kMaxCallers = 62;

    void* callers[kMaxCallers];
    std::int32_t count = (func)(0, kMaxCallers, callers, nullptr);
    for (std::int32_t i = 0; i < count; i++)
        fprintf(stderr, "*** %d called from %p\n", i, callers[i]);

#elif defined(_WIN32)  // not _WIN64 ? Must be _WIN32

    char* pBP = nullptr;
    std::uint32_t i = 0, x = 0, BpPassed = 0;
    static std::int32_t CurrentlyInTheStackDump = 0;

    if (CurrentlyInTheStackDump) {
        fprintf(stderr, "\n***\n*** Recursive Stack Dump skipped\n***\n");
        return;
    }

    fprintf(stderr, "****************************************************\n");
    fprintf(stderr, "*** CallStack:\n");
    fprintf(stderr, "****************************************************\n");

    /* ====================================================================== */
    /*                                                                        */
    /*      BP +x ...    -> == SP (current top of stack)                      */
    /*            ...    -> Local data of current function                    */
    /*      BP +4 0xabcd -> 32 address of calling function                    */
    /*  +<==BP    0xabcd -> Stack address of next stack frame (0, if end)     */
    /*  |   BP -1 ...    -> Aruments of function call                         */
    /*  Y                                                                     */
    /*  |   BP -x ...    -> Local data of calling function                    */
    /*  |                                                                     */
    /*  Y  (BP)+4 0xabcd -> 32 address of calling function                    */
    /*  +==>BP)   0xabcd -> Stack address of next stack frame (0, if end)     */
    /*            ...                                                         */
    /* ====================================================================== */
    CurrentlyInTheStackDump = 1;

    BpPassed = (eNextBP != nullptr);

    if (!eNextBP) {
        _asm mov eNextBP, eBp
    } else
        fprintf(
            stderr,
            "\n  Fault Occured At $ADDRESS:%08LX\n",
            (std::int32_t)FaultAdress);

    // prevent infinite loops
    for (i = 0; eNextBP && i < 100; i++) {
        pBP = eNextBP;           // keep current BasePointer
        eNextBP = *(char**)pBP;  // dereference next BP

        char* p = pBP + 8;

        // Write 20 Bytes of potential arguments
        fprintf(stderr, "         with ");
        for (x = 0; p < eNextBP && x < 20; p++, x++)
            fprintf(stderr, "%02X ", *(std::uint8_t*)p);

        fprintf(stderr, "\n\n");

        if (i == 1 && !BpPassed)
            fprintf(
                stderr,
                "****************************************************\n"
                "         Fault Occured Here:\n");

        // Write the backjump address
        fprintf(
            stderr,
            "*** %2d called from $ADDRESS:%08X\n",
            i,
            *(char**)(pBP + 4));

        if (*(char**)(pBP + 4) == nullptr) break;
    }

    fprintf(
        stderr,
        "************************************************************\n");
    fprintf(stderr, "\n\n");

    CurrentlyInTheStackDump = 0;

    fflush(stderr);
#endif                 // _WIN64 else (_WIN32) endif
}

#endif
