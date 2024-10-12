/*  
*   File: gpt.h
*
*   Author: Garnek
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef GPT_H
#define GPT_H

#include <garn/types.h>
#include <garn/dal/dal.h>

struct _drive;

typedef struct {
    char magic[8];
    uint32_t revision;
    uint32_t headerSize;
    uint32_t CRC32;
    uint32_t reserved;
    uint64_t headerLBA;
    uint64_t alternateLBA;
    uint64_t firstUsableBlock;
    uint64_t lastUsableBlock;
    uint64_t GUID[2];
    uint64_t gptArrayStartLBA;
    uint32_t partCount;
    uint32_t entrySize;
    uint32_t PartEntryArrayCRC32;
} gpt_header_t;

typedef struct {
    uint64_t typeGUID[2];
    uint64_t partGUID[2];
    uint64_t startLBA;
    uint64_t endLBA;
    uint64_t attribs;
    char* partName;
} gpt_entry_t;


//TODO: having drive_t* here gives an error. Why?

bool gpt_validate_drive(struct _drive* drive);
bool gpt_initialise_drive(struct _drive* drive);

#endif //GPT_H