#ifndef INCLUDED_LOG_H__
#define INCLUDED_LOG_H__

#include <stdio.h>

enum
{
    LogMain    = 0,
    LogDecode  = 1,
    LogInput   = 2,
    LogOutput  = 3,
    LogPerf    = 4,

    LogLast    = 5,
    LogMax     = 32
};

class LogFile
{
public:

    LogFile(const wchar_t *szLogFileArg);
    virtual ~LogFile(void);

    bool     Open(void);
    bool     Close(void);
    void     ClearLogLevel(void)
    {
        m_iLogLevelFlags = 0;
    };
    void     AddLogLevel(int iLogLevelFlags);

    void     Error(const wchar_t *Format, ...);
    void     Log(int Section, const wchar_t *Format, ...);

private:

    FILE *m_fpLog;
    wchar_t *m_szLogFile;
    int   m_iLogLevelFlags;
};

#endif
