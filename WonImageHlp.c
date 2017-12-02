/* WonDbgHelp.c --- Wonders ImageDirectoryEntryToData */
/* Copyright (C) 2017 Katayama Hirofumi MZ. License: GPLv3 */
/**************************************************************************/

#include "WonImageHlp.h"

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************/

typedef LONG NTSTATUS;

#ifndef STATUS_SUCCESS
    #define STATUS_SUCCESS ((NTSTATUS)0x00000000)
#endif
#ifndef STATUS_INVALID_PARAMETER
    #define STATUS_INVALID_PARAMETER ((NTSTATUS)0xC000000D)
#endif
#ifndef STATUS_INVALID_IMAGE_FORMAT
    #define STATUS_INVALID_IMAGE_FORMAT ((NTSTATUS)0xC000007B)
#endif

static NTSTATUS NTAPI
WonImageNtHeaderEx(
    ULONG flags,
    void *base,
    ULONGLONG size,
    IMAGE_NT_HEADERS **out_nt)
{
    IMAGE_DOS_HEADER *dos;
    IMAGE_NT_HEADERS *nt;
    DWORD nt_offset = 0, file_offset;
    BOOLEAN needs_check;

    if (!out_nt)
    {
        return STATUS_INVALID_PARAMETER;
    }

    *out_nt = NULL;

    if ((flags & ~RTL_IMAGE_NT_HEADER_EX_FLAG_NO_RANGE_CHECK) ||
        !base || base == (void *)-1)
    {
        return STATUS_INVALID_PARAMETER;
    }

    needs_check = !(flags & RTL_IMAGE_NT_HEADER_EX_FLAG_NO_RANGE_CHECK);
    if (needs_check)
    {
        /* Make sure the image size is at least big enough for the DOS header */
        if (size < sizeof(IMAGE_DOS_HEADER))
        {
            return STATUS_INVALID_IMAGE_FORMAT;
        }
    }

    dos = (IMAGE_DOS_HEADER *)base;
    if (dos->e_magic == IMAGE_DOS_SIGNATURE)
        nt_offset = dos->e_lfanew;

    if (nt_offset >= 256 * 1024 * 1024)
        return STATUS_INVALID_IMAGE_FORMAT;

    if (needs_check)
    {
        file_offset = RTL_SIZEOF_THROUGH_FIELD(IMAGE_NT_HEADERS, FileHeader);
        if (nt_offset + file_offset >= size)
        {
            return STATUS_INVALID_IMAGE_FORMAT;
        }
    }

    nt = (IMAGE_NT_HEADERS *)((ULONG_PTR)base + nt_offset);
    if (nt->Signature != IMAGE_NT_SIGNATURE)
        return STATUS_INVALID_IMAGE_FORMAT;

    *out_nt = nt;
    return STATUS_SUCCESS;
}

/**************************************************************************/

IMAGE_NT_HEADERS *NTAPI WonImageNtHeader(void *base)
{
    IMAGE_NT_HEADERS *nt;

    WonImageNtHeaderEx(RTL_IMAGE_NT_HEADER_EX_FLAG_NO_RANGE_CHECK,
                       base,
                       0,
                       &nt);
    return nt;
}

IMAGE_SECTION_HEADER *NTAPI
WonImageRvaToSection(IMAGE_NT_HEADERS *nt, void *base, ULONG rva)
{
    IMAGE_SECTION_HEADER *section;
    ULONG va;
    ULONG count;

    count = nt->FileHeader.NumberOfSections;
    section = IMAGE_FIRST_SECTION(nt);

    while (count--)
    {
        va = section->VirtualAddress;
        if (va <= rva && rva < va + section->SizeOfRawData)
            return section;
        section++;
    }

    return NULL;
}

void *NTAPI WonImageRvaToVa(
    IMAGE_NT_HEADERS *nt,
    void *base,
    ULONG rva,
    IMAGE_SECTION_HEADER **ppSection)
{
    IMAGE_SECTION_HEADER *pSection = NULL;

    if (ppSection)
        pSection = *ppSection;

    if (pSection == NULL ||
        rva < pSection->VirtualAddress ||
        rva >= pSection->VirtualAddress + pSection->SizeOfRawData)
    {
        pSection = WonImageRvaToSection(nt, base, rva);
        if (pSection == NULL)
            return NULL;

        if (ppSection)
            *ppSection = pSection;
    }

    return (void *)((ULONG_PTR)base + rva +
                    (ULONG_PTR)pSection->PointerToRawData -
                    (ULONG_PTR)pSection->VirtualAddress);
}

/**************************************************************************/

void *WINAPI
WonImageDirectoryEntryToDataEx(
    void *base,
    BOOLEAN image,
    USHORT dir,
    ULONG *size,
    IMAGE_SECTION_HEADER **section)
{
    IMAGE_NT_HEADERS *nt;
    DWORD addr;

    *size = 0;
    if (section)
        *section = NULL;

    if (!(nt = WonImageNtHeader(base)))
        return NULL;
    if (dir >= nt->OptionalHeader.NumberOfRvaAndSizes)
        return NULL;
    if (!(addr = nt->OptionalHeader.DataDirectory[dir].VirtualAddress))
        return NULL;

    *size = nt->OptionalHeader.DataDirectory[dir].Size;
    if (image || addr < nt->OptionalHeader.SizeOfHeaders)
        return (BYTE *)base + addr;

    return WonImageRvaToVa(nt, base, addr, section);
}

void *WINAPI
WonImageDirectoryEntryToData(
    void *base,
    BOOLEAN image,
    USHORT dir,
    ULONG *size)
{
    return WonImageDirectoryEntryToDataEx(base, image, dir, size, NULL);
}

/**************************************************************************/

#ifdef __cplusplus
} // extern "C"
#endif
