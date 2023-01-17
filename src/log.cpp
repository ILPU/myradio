#undef __STRICT_ANSI__
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "log.h"
#include "utils_str.h"

const int iMaxLogLineLen = 2048;

const wchar_t *szLogLevelNames[] =
{
    _T("Main"),
    _T("Decode"),
    _T("Input"),
    _T("Output"),
    _T("Perf")
};

LogFile::LogFile(const wchar_t *m_szLogFileArg)
{
    m_szLogFile = strdup_new(m_szLogFileArg);
    m_fpLog = NULL;
    m_iLogLevelFlags = 0x0;
}

LogFile::~LogFile(void)
{
    if (m_fpLog)
        fclose(m_fpLog);
    if (m_szLogFile)
        delete [] m_szLogFile;
}

bool LogFile::Open(void)
{
    m_fpLog = _wfopen(m_szLogFile, _T("a"));
    return m_fpLog != NULL;
}

bool LogFile::Close(void)
{
    if (m_fpLog == NULL)
        return false;
    else
    {
        fclose(m_fpLog);
        m_fpLog = NULL;
    }

    return true;
}

void LogFile::AddLogLevel(int iLogLevelFlags)
{
    if (iLogLevelFlags < 0 || iLogLevelFlags >= LogLast)
        return;

    m_iLogLevelFlags |= 1 << iLogLevelFlags;
}

void LogFile::Error(const wchar_t *format, ...)
{
    wchar_t *szBuffer;
    va_list argptr;

    if (m_fpLog == NULL)
        return;

    szBuffer = new wchar_t[iMaxLogLineLen];

    va_start(argptr, format);
    vswprintf(szBuffer, format, argptr);
    va_end(argptr);

    fwprintf(m_fpLog, _T("Error: %s"), szBuffer);
    fflush(m_fpLog);

    delete [] szBuffer;
}

void LogFile::Log(int iLogLevel, const wchar_t *format, ...)
{
    wchar_t *szBuffer;
    va_list argptr;

    if (m_fpLog == NULL || (m_iLogLevelFlags & ( 1 << iLogLevel)) == 0)
        return;

    szBuffer = new wchar_t[iMaxLogLineLen];

    va_start(argptr, format);
    vswprintf(szBuffer, format, argptr);
    va_end(argptr);

    fwprintf(m_fpLog, _T("%s: %s"), szLogLevelNames[iLogLevel], szBuffer);
    fflush(m_fpLog);

    delete [] szBuffer;
}
