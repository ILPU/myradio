#ifndef SKIN_FILE_H
#define SKIN_FILE_H

#include "../common.h"
#include "mystring.h"
#include "myfile.h"

#define CPC_PKFILE_MAGIC  0x04034B50
#define CPC_PKFILE_DIRMAGIC  0x02014B50
#define CPC_PKFILE_BITS_ENCRYPTED  0x1
#define CPC_PKFILE_BITS_STREAMED  0x8

#define CPC_PKFILE_METHOD_STORED  0x0
#define CPC_PKFILE_METHOD_DEFLATED  0x8

#pragma pack(push, 1)
typedef struct _PKFILE_HEADER
{
    DWORD m_dwSig;
    WORD m_wVersion;
    WORD m_wBITs;
    WORD m_wMethod;
    WORD m_wModifyTime;
    WORD m_wModifyDate;
    DWORD m_dwCRC32;
    DWORD m_dwCompressedSize;
    DWORD m_dwDecompressedSize;
    WORD m_wFilenameLen;
    WORD m_wExtraFieldLen;
} PKFILE_HEADER;

typedef struct _PKFILE_DESCRIPTOR
{
    DWORD m_dwCRC32;
    DWORD m_dwCompressedSize;
    DWORD m_dwDecompressedSize;

} PKFILE_DESCRIPTOR;
#pragma pack(pop)

typedef struct _SubFile
{
    char* m_pcName;
    DWORD m_dwCRC32;
    unsigned int m_iFileOffset;
    unsigned int m_iCompressedSize;
    unsigned int m_iUncompressedSize;
    WORD m_wMethod;
    void* m_pNext;

} SubFile;

#define CPM_GET_BYTE(offset) (*(BYTE*)(pContext->m_pFileBase + (offset++) ))
#define CPM_GET_WORD(offset) (*(WORD*)(pContext->m_pFileBase + (offset+=2) - 2))
#define CPM_GET_DWORD(offset) (*(DWORD*)(pContext->m_pFileBase + (offset+=4) - 4))

class skin_file
{
public:
    skin_file();
    ~skin_file();

    bool OpenFile(mystring *pfile);
    bool OpenResource(HMODULE hModule, UINT uiResourceID, const wchar_t* pcResourceType);
    bool GetSubFile(const char* pcSubfilename, myfile *pfile);

private:

    HANDLE m_hFileMapping;
    BYTE* m_pFileBase;
    DWORD m_dwFileSize;
    bool m_bMemMappedFile;
    SubFile* m_pFirstSubFile;

    bool BuildDirectory();
    const SubFile* FindFile(const char* pcFilename);
};

#endif // SKIN_FILE_H
