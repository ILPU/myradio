#include "skin_file.h"
#include "zlib.h"

skin_file::skin_file()
{
    m_bMemMappedFile = false;
    m_dwFileSize = 0;
    m_hFileMapping = 0;
    m_pFileBase = NULL;
    m_pFirstSubFile = NULL;
}

skin_file::~skin_file()
{
    if(m_hFileMapping)
    {
        UnmapViewOfFile(m_pFileBase);
        CloseHandle(m_hFileMapping);
        m_hFileMapping = 0;
    }
    SubFile* pSubFile_Cursor;
    SubFile* pSubFile_Next;
    for(pSubFile_Cursor = m_pFirstSubFile; pSubFile_Cursor; pSubFile_Cursor = pSubFile_Next)
    {
        pSubFile_Next = (SubFile*)pSubFile_Cursor->m_pNext;
        free(pSubFile_Cursor->m_pcName);
        free(pSubFile_Cursor);
    }
    m_dwFileSize = 0;
}

bool skin_file::GetSubFile(const char* pcSubfilename, myfile *pfile)
{
    const SubFile* pSubFile;

    pSubFile = FindFile(pcSubfilename);
    if(!pSubFile)
    {
        return false;
    }

    DWORD piSubFile_Length = pSubFile->m_iUncompressedSize;

    if(piSubFile_Length)
    {
        pfile->open_memory(piSubFile_Length);
        pfile->fMemory = malloc(piSubFile_Length);

        if(pSubFile->m_wMethod == CPC_PKFILE_METHOD_STORED)
        {
            memcpy(pfile->fMemory, m_pFileBase + pSubFile->m_iFileOffset, piSubFile_Length);
        }
        else if(pSubFile->m_wMethod == CPC_PKFILE_METHOD_DEFLATED)
        {
            z_stream zStream;
            zStream.zalloc = Z_NULL;
            zStream.zfree = Z_NULL;
            zStream.opaque = Z_NULL;
            zStream.avail_in = 0;
            zStream.data_type = Z_BINARY;
            inflateInit2(&zStream, -15); // 15bit window (32Kb window size)
            // Decompress
            zStream.next_out = (BYTE*)pfile->fMemory;
            zStream.avail_out = piSubFile_Length;
            zStream.next_in = m_pFileBase + pSubFile->m_iFileOffset;
            zStream.avail_in = pSubFile->m_iCompressedSize;
            inflate(&zStream, Z_FINISH);
            // Cleanup
            inflateEnd(&zStream);
        }

        DWORD dwCRC32;
        dwCRC32 = crc32(0, Z_NULL, 0);
        dwCRC32 = crc32(dwCRC32, (const Byte*)pfile->fMemory, piSubFile_Length);
        if(dwCRC32 != pSubFile->m_dwCRC32)
        {
            // CRC32 does not match
            pfile->close();
            return false;
        }
        return true;
    }
    else return false;
}

bool skin_file::BuildDirectory()
{
    unsigned int iOffset = 0;
    while (((iOffset + sizeof(PKFILE_HEADER)) < m_dwFileSize)
            && *(DWORD*)(m_pFileBase + iOffset) != CPC_PKFILE_DIRMAGIC)
    {
        PKFILE_HEADER* pHeader = (PKFILE_HEADER*)(m_pFileBase + iOffset);
        SubFile* pNewSubFile;

        if (pHeader->m_dwSig != CPC_PKFILE_MAGIC
                || (pHeader->m_wBITs & CPC_PKFILE_BITS_ENCRYPTED)
                || (pHeader->m_wBITs & CPC_PKFILE_BITS_STREAMED)
                || (pHeader->m_wMethod != CPC_PKFILE_METHOD_STORED && pHeader->m_wMethod != CPC_PKFILE_METHOD_DEFLATED))
        {
            return false; // not support zip format
        }

        pNewSubFile = static_cast<SubFile*>(malloc(sizeof(*pNewSubFile)));
        pNewSubFile->m_pNext = m_pFirstSubFile;
        m_pFirstSubFile = pNewSubFile;

        pNewSubFile->m_pcName = static_cast<char*>(malloc(pHeader->m_wFilenameLen + 1));
        memcpy(pNewSubFile->m_pcName, m_pFileBase + iOffset + sizeof(*pHeader), pHeader->m_wFilenameLen);

        pNewSubFile->m_pcName[pHeader->m_wFilenameLen] = '\0';
        pNewSubFile->m_wMethod = pHeader->m_wMethod;
        pNewSubFile->m_dwCRC32 = pHeader->m_dwCRC32;
        pNewSubFile->m_iCompressedSize = pHeader->m_dwCompressedSize;
        pNewSubFile->m_iUncompressedSize = pHeader->m_dwDecompressedSize;
        pNewSubFile->m_iFileOffset = iOffset + sizeof(*pHeader) + pHeader->m_wFilenameLen + pHeader->m_wExtraFieldLen;

        iOffset += sizeof(*pHeader)
                   + pHeader->m_dwCompressedSize
                   + pHeader->m_wFilenameLen
                   + pHeader->m_wExtraFieldLen;
    }
    return true;
}

const SubFile* skin_file::FindFile(const char* pcFilename)
{
    const SubFile* pSubFile_Cursor;
    for(pSubFile_Cursor = m_pFirstSubFile; pSubFile_Cursor; pSubFile_Cursor = (const SubFile*)pSubFile_Cursor->m_pNext)
    {
        if (stricmp(pSubFile_Cursor->m_pcName, pcFilename) == 0)
            return pSubFile_Cursor;
    }
    return NULL;
}

bool skin_file::OpenFile(mystring *pfile)
{
    HANDLE hFile = CreateFile(pfile->w_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        DWORD dwFileSize = GetFileSize(hFile, NULL);
        if(dwFileSize < 0x00800000)
        {
            m_dwFileSize = dwFileSize;
            m_hFileMapping = CreateFileMapping(hFile, 0, PAGE_READONLY, 0, 0, 0);
            CloseHandle(hFile);
            if(m_hFileMapping != INVALID_HANDLE_VALUE)
            {
                m_pFileBase = static_cast<BYTE*>(MapViewOfFile(m_hFileMapping, FILE_MAP_READ, 0, 0, 0));
                if(m_pFileBase)
                    return BuildDirectory();
            }
        }
    }
    return false;
}

bool skin_file::OpenResource(HMODULE hModule, UINT uiResourceID, const wchar_t* pcResourceType)
{
    HRSRC hResource = FindResource(hModule, MAKEINTRESOURCE(uiResourceID), pcResourceType);
    if(hResource)
    {
        HGLOBAL hResourceData = LoadResource(hModule, hResource);
        if(hResourceData)
        {
            m_dwFileSize = SizeofResource(hModule, hResource);
            m_hFileMapping = 0;
            m_pFileBase = static_cast<BYTE*>(LockResource(hResourceData));
            if(m_pFileBase)
                return BuildDirectory();
        }
    }
    return false;
}
