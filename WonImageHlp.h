/* WonImageHlp.h --- Wonders API */
/* Copyright (C) 2017 Katayama Hirofumi MZ. License: GPLv3 */
/**************************************************************************/

#ifndef WONIMAGEHLP_H_
#define WONIMAGEHLP_H_     5   /* Version 5 */

#if !defined(_WIN32) || (defined(_WONVER) && _WONVER == 0)
    #include "wonnt.h"
#else
    #ifndef _INC_WINDOWS
        #include <windows.h>
    #endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************/

#ifndef RTL_IMAGE_NT_HEADER_EX_FLAG_NO_RANGE_CHECK
    #define RTL_IMAGE_NT_HEADER_EX_FLAG_NO_RANGE_CHECK 0x00000001
#endif

IMAGE_NT_HEADERS *NTAPI WonImageNtHeader(void *base);

IMAGE_SECTION_HEADER *NTAPI
WonImageRvaToSection(IMAGE_NT_HEADERS *nt, void *base, ULONG rva);

void *NTAPI WonImageRvaToVa(
    IMAGE_NT_HEADERS *nt,
    void *base,
    ULONG rva,
    IMAGE_SECTION_HEADER **ppSection);

void *WINAPI
WonImageDirectoryEntryToData(
    void *base,
    BOOLEAN image,
    USHORT dir,
    ULONG *size);

void *WINAPI
WonImageDirectoryEntryToDataEx(
    void *base,
    BOOLEAN image,
    USHORT dir,
    ULONG *size,
    IMAGE_SECTION_HEADER **section);

/**************************************************************************/

#ifdef __cplusplus
} // extern "C"
#endif

/**************************************************************************/

#if !defined(_WIN32) || (defined(_WONVER) && _WONVER == 0)
    #define ImageNtHeader WonImageNtHeader
    #define ImageRvaToSection WonImageRvaToSection
    #define ImageRvaToVa WonImageRvaToVa
    #define ImageDirectoryEntryToData WonImageDirectoryEntryToData
    #define ImageDirectoryEntryToDataEx WonImageDirectoryEntryToDataEx
#endif

#endif  /* ndef WONIMAGEHLP_H_ */
