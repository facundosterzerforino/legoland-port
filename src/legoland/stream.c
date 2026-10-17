#include <windows.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "imports.h"
#include "legoland.h"
#include "sound_music.h"

#include "stream.h"

// FUNCTION: LEGOLAND 0x00497f60
unsigned int FUN_00497f60(void) {
    int head;
    int tail;

    head = SpeechRawWritePos;
    tail = SpeechRawReadPos;
    if (head >= tail) {
        if (tail == 0) {
            return 0xffff - head;
        }
        return 0x10000 - head;
    }
    return tail - head - 1;
}

// FUNCTION: LEGOLAND 0x00497f90
int SpeechRawContiguousBytes(void) {
    int head;
    int tail;

    head = SpeechRawWritePos;
    tail = SpeechRawReadPos;
    if (head < tail) {
        head = 0x10000;
    }
    return head - tail;
}

// FUNCTION: LEGOLAND 0x00497fb0
int FUN_00497fb0(void) {
    int used;
    int limit;

    limit = SpeechChunkByteCounts[0];
    used = (SpeechRawWritePos - SpeechRawReadPos) & 0xffff;
    if (limit == 0) {
        memcpy(SpeechChunkByteCounts, &SpeechChunkByteCounts[1], 0x13 * sizeof(unsigned int));
        SpeechChunkByteCounts[19] = 0;
        if (SpeechChunkIndex != 0) {
            SpeechChunkIndex--;
        }
    }
    if (used <= limit) {
        return used;
    }
    return limit;
}

// FUNCTION: LEGOLAND 0x00498000
void SpeechFillRawBuffer(void) {
    int n;
    int r;
    unsigned int idx;
    unsigned char mask;

    if (SpeechBytesRemaining == 0) {
        return;
    }
    n = FUN_00497f60();
    if (n == 0) {
        return;
    }
    mask = 4;
    while (n != 0) {
        while (n != 0) {
            if (n > (int)SpeechBytesRemaining) {
                r = _read(SpeechFileHandle, &SpeechRawBuffer[SpeechRawWritePos], SpeechBytesRemaining);
            } else {
                r = _read(SpeechFileHandle, &SpeechRawBuffer[SpeechRawWritePos], n);
            }
            idx = SpeechChunkIndex;
            if (r != -1) {
                SpeechBytesRemaining -= r;
                SpeechChunkByteCounts[idx] += r;
                SpeechRawWritePos = (SpeechRawWritePos + r) & 0xffff;
            }
            if (r < n) {
                if (!(SpeechFlags & mask)) {
                    SpeechChunkIndex = idx + 1;
                    return;
                }
                if (SpeechBytesRemaining == 0) {
                    FUN_00498100();
                    SpeechChunkIndex++;
                }
            }
            n -= r;
        }
        n = FUN_00497f60();
    }
}

// FUNCTION: LEGOLAND 0x00498100
void FUN_00498100(void) {
    SpeechRewindToData();
    SpeechFlags |= 8;
}

// FUNCTION: LEGOLAND 0x00498120
void SpeechRewindToData(void) {
    _lseek(SpeechFileHandle, SpeechDataOffset, 0);
    SpeechBytesRemaining = SpeechDataSize;
}

// FUNCTION: LEGOLAND 0x00498150
int SpeechReadRawBuffer(unsigned char *dst, int count) {
    int avail;
    int chunk;
    int n;

    avail = FUN_00497fb0();
    n = count;
    if (n > avail) {
        SpeechFillRawBuffer();
        avail = FUN_00497fb0();
        if (n > avail) {
            n = avail;
        }
    }
    count = n;
    if (n != 0) {
        do {
            chunk = SpeechRawContiguousBytes();
            if (chunk > n) {
                chunk = n;
            }
            memcpy(dst, &SpeechRawBuffer[SpeechRawReadPos], chunk);
            n -= chunk;
            dst += chunk;
            SpeechRawReadPos = (SpeechRawReadPos + chunk) & 0xffff;
            SpeechChunkByteCounts[0] -= chunk;
        } while (n != 0);
    }
    SpeechFillRawBuffer();
    return count;
}

// FUNCTION: LEGOLAND 0x004981e0
unsigned int FUN_004981e0(void) {
    int head;
    int tail;

    head = SpeechPcmWritePos;
    tail = SpeechPcmReadPos;
    if (head >= tail) {
        if (tail == 0) {
            return 0x1ffff - head;
        }
        return 0x20000 - head;
    }
    return tail - head - 1;
}

// FUNCTION: LEGOLAND 0x00498210
int SpeechPcmContiguousBytes(void) {
    int head;
    int tail;

    head = SpeechPcmWritePos;
    tail = SpeechPcmReadPos;
    if (head < tail) {
        head = 0x20000;
    }
    return head - tail;
}

// FUNCTION: LEGOLAND 0x00498230
unsigned int SpeechPcmUsedBytes(void) {
    return (SpeechPcmWritePos - SpeechPcmReadPos) & 0x1ffff;
}

// FUNCTION: LEGOLAND 0x00498250
void SpeechFillPcmBuffer(void) {
    int n;
    int chunk;
    int len;

    SpeechFillRawBuffer();
    n = FUN_004981e0();
    while (n != 0) {
        while (n != 0) {
            chunk = SpeechConvertedSize - SpeechConvertedUsed;
            if (chunk == 0) {
                if (n != 0) {
                    len = SpeechChunkByteCounts[0];
                    if (len >= SpeechAcmSrcSize) {
                        len = SpeechAcmSrcSize;
                    }
                    SpeechAcmHeader.cbSrcLength = len;
                    SpeechReadRawBuffer(SpeechAcmSrcBuffer, len);
                    acmStreamConvert(SpeechAcmStream, &SpeechAcmHeader, 0x10);
                    SpeechConvertedUsed = 0;
                    SpeechConvertedSize = SpeechAcmHeader.cbDstLengthUsed;
                    if (SpeechAcmHeader.cbDstLengthUsed < n) {
                        memcpy(&SpeechPcmBuffer[SpeechPcmWritePos], SpeechAcmDstBuffer, SpeechAcmHeader.cbDstLengthUsed);
                        SpeechConvertedUsed = SpeechAcmHeader.cbDstLengthUsed;
                        SpeechPcmWritePos += SpeechAcmHeader.cbDstLengthUsed;
                        return;
                    }
                    memcpy(&SpeechPcmBuffer[SpeechPcmWritePos], SpeechAcmDstBuffer, n);
                    SpeechConvertedUsed = n;
                    SpeechPcmWritePos = (SpeechPcmWritePos + n) & 0x1ffff;
                }
                break;
            }
            if (n < chunk) {
                chunk = n;
            }
            memcpy(&SpeechPcmBuffer[SpeechPcmWritePos], SpeechAcmDstBuffer + SpeechConvertedUsed, chunk);
            SpeechPcmWritePos = (SpeechPcmWritePos + chunk) & 0x1ffff;
            n -= chunk;
            SpeechConvertedUsed += chunk;
        }
        n = FUN_004981e0();
    }
    SpeechFillRawBuffer();
}

// FUNCTION: LEGOLAND 0x004983a0
int SpeechReadPcmBuffer(unsigned char *dst, int count) {
    int avail;
    int chunk;
    int n;

    avail = SpeechPcmUsedBytes();
    n = count;
    if (n > avail) {
        SpeechFillPcmBuffer();
        avail = SpeechPcmUsedBytes();
        if (n > avail) {
            n = avail;
        }
    }
    count = n;
    if (n != 0) {
        do {
            chunk = SpeechPcmContiguousBytes();
            if (chunk > n) {
                chunk = n;
            }
            memcpy(dst, &SpeechPcmBuffer[SpeechPcmReadPos], chunk);
            n -= chunk;
            dst += chunk;
            SpeechPcmReadPos = (SpeechPcmReadPos + chunk) & 0x1ffff;
        } while (n != 0);
    }
    SpeechFillPcmBuffer();
    return count;
}

// Parses the RIFF/WAVE header of the open speech file: checks "RIFF" and "WAVE", reads the "fmt "
// chunk into SpeechSourceFormat (at least 18 bytes; cbSize cleared for a short WAVEFORMAT),
// skips other chunks up to "data", then stores the data size and its file offset. Returns 1 on
// success, 0 on any read error or unexpected tag.
//
// MSVC6 build: exact naked __asm transcription; the C in the #else branch is the same function and
// each asm block is commented with the C it implements. Why asm: the C was at best 93.19%: the
// original's chunk-skipping loop reads the next tag at the bottom and shares one return path;
// both loop shapes tried from C lay it out differently (87.43% / 68.69%). See
// docs/attempts/0x00498420-SpeechParseWavHeader.md.
// clang-format off
#if defined(_MSC_VER) && (_MSC_VER <= 1200) && defined(_M_IX86)
// FUNCTION: LEGOLAND 0x00498420
__declspec(naked) int SpeechParseWavHeader(void) {
    __asm {
        // Frame after `push esi`: [esp+4] size, [esp+8] tag.
        // _lseek(SpeechFileHandle, 0, 0);
        mov eax, dword ptr [SpeechFileHandle]
        sub esp, 8
        push esi
        push 0
        push 0
        push eax
        call _lseek
        // if (_read(SpeechFileHandle, &tag, 4) != 4) return 0;
        mov edx, dword ptr [SpeechFileHandle]
        lea ecx, [esp + 0x14] // &tag
        push 4
        push ecx
        push edx
        call _read
        add esp, 0x18
        cmp eax, 4
        je read_riff
        xor eax, eax
        pop esi
        add esp, 8
        ret
    read_riff:
        // if (tag != 'RIFF') return 0;
        cmp dword ptr [esp + 8], 0x46464952
        je riff_ok
        xor eax, eax
        pop esi
        add esp, 8
        ret
    riff_ok:
        // if (_read(SpeechFileHandle, &size, 4) != 4) return 0;   (RIFF size, unused)
        mov ecx, dword ptr [SpeechFileHandle]
        lea eax, [esp + 4] // &size
        push 4
        push eax
        push ecx
        call _read
        add esp, 0xc
        cmp eax, 4
        je read_wave
        xor eax, eax
        pop esi
        add esp, 8
        ret
    read_wave:
        // if (_read(SpeechFileHandle, &tag, 4) != 4) return 0; if (tag != 'WAVE') return 0;
        mov eax, dword ptr [SpeechFileHandle]
        lea edx, [esp + 8]
        push 4
        push edx
        push eax
        call _read
        add esp, 0xc
        cmp eax, 4
        je check_wave
        xor eax, eax
        pop esi
        add esp, 8
        ret
    check_wave:
        cmp dword ptr [esp + 8], 0x45564157
        je wave_ok
        xor eax, eax
        pop esi
        add esp, 8
        ret
    wave_ok:
        // if (_read(SpeechFileHandle, &tag, 4) != 4) return 0;   ("fmt ")
        mov edx, dword ptr [SpeechFileHandle]
        lea ecx, [esp + 8]
        push 4
        push ecx
        push edx
        call _read
        add esp, 0xc
        cmp eax, 4
        je read_fmt_size
        xor eax, eax
        pop esi
        add esp, 8
        ret
    read_fmt_size:
        // if (_read(SpeechFileHandle, &size, 4) != 4) return 0;
        mov ecx, dword ptr [SpeechFileHandle]
        lea eax, [esp + 4]
        push 4
        push eax
        push ecx
        call _read
        add esp, 0xc
        cmp eax, 4
        je alloc_format
        xor eax, eax
        pop esi
        add esp, 8
        ret
    alloc_format:
        // SpeechSourceFormat = malloc(size < 0x12 ? 0x12 : size);
        mov eax, dword ptr [esp + 4]
        cmp eax, 0x12
        jae format_size_ok
        push 0x12
        jmp format_malloc
    format_size_ok:
        push eax
    format_malloc:
        call malloc
        // if (_read(SpeechFileHandle, SpeechSourceFormat, size) != size) return 0;
        mov edx, dword ptr [esp + 8]
        add esp, 4
        mov dword ptr [SpeechSourceFormat], eax
        push edx
        push eax
        mov eax, dword ptr [SpeechFileHandle]
        push eax
        call _read
        mov ecx, dword ptr [esp + 0x10] // size
        add esp, 0xc
        cmp eax, ecx
        je format_read
        xor eax, eax
        pop esi
        add esp, 8
        ret
    format_read:
        // if (size <= 0x12) ((WAVEFORMATEX *)SpeechSourceFormat)->cbSize = 0;
        cmp ecx, 0x12
        ja format_done
        mov ecx, dword ptr [SpeechSourceFormat]
        mov word ptr [ecx + 0x10], 0
    format_done:
        // while (_read(SpeechFileHandle, &tag, 4) == 4) {
        mov eax, dword ptr [SpeechFileHandle]
        lea edx, [esp + 8]
        push 4
        push edx
        push eax
        call _read
        add esp, 0xc
        cmp eax, 4
        jne fail
    next_chunk:
        //     if (tag == 'data') break;
        mov eax, dword ptr [esp + 8]
        push 4 // (first argument of the data-size read below, or of the chunk-size read)
        cmp eax, 0x61746164
        je found_data
        //     if (_read(SpeechFileHandle, &size, 4) != 4) return 0;
        mov edx, dword ptr [SpeechFileHandle]
        lea ecx, [esp + 8]
        push ecx
        push edx
        call _read
        add esp, 0xc
        cmp eax, 4
        jne fail_after_data_check
        //     p = malloc(size); if (_read(SpeechFileHandle, p, size) != size) { free(p); return 0; }
        //     free(p);   (esi = p)
        mov eax, dword ptr [esp + 4]
        push eax
        call malloc
        mov ecx, dword ptr [esp + 8]
        mov edx, dword ptr [SpeechFileHandle]
        mov esi, eax
        push ecx
        push esi
        push edx
        call _read
        mov ecx, dword ptr [esp + 0x14]
        add esp, 0x10
        cmp eax, ecx
        push esi
        jne chunk_read_failed
        call free
        // (next tag read at the bottom of the loop)
        mov ecx, dword ptr [SpeechFileHandle]
        lea eax, [esp + 0xc]
        push 4
        push eax
        push ecx
        call _read
        add esp, 0x10
        cmp eax, 4
        je next_chunk
        // }
    fail:
        xor eax, eax
        pop esi
        add esp, 8
        ret
    chunk_read_failed:
        call free
        add esp, 4
        xor eax, eax
        pop esi
        add esp, 8
        ret
    found_data:
        // if (_read(SpeechFileHandle, &SpeechDataSize, 4) != 4) return 0;
        mov edx, dword ptr [SpeechFileHandle]
        push offset SpeechDataSize
        push edx
        call _read
        add esp, 0xc
        cmp eax, 4
        je data_size_read
    fail_after_data_check:
        xor eax, eax
        pop esi
        add esp, 8
        ret
    data_size_read:
        // SpeechDataOffset = _tell(SpeechFileHandle); return 1;
        mov eax, dword ptr [SpeechFileHandle]
        push eax
        call _tell
        add esp, 4
        mov dword ptr [SpeechDataOffset], eax
        mov eax, 1
        pop esi
        add esp, 8
        ret
    }
}
// clang-format on
#else
int SpeechParseWavHeader(void) {
    unsigned int size;
    unsigned int tag;
    unsigned int *p;

    _lseek(SpeechFileHandle, 0, 0);
    if (_read(SpeechFileHandle, &tag, 4) != 4) {
        return 0;
    }
    if (tag != 0x46464952) {
        return 0;
    }
    if (_read(SpeechFileHandle, &size, 4) != 4) {
        return 0;
    }
    if (_read(SpeechFileHandle, &tag, 4) != 4) {
        return 0;
    }
    if (tag != 0x45564157) {
        return 0;
    }
    if (_read(SpeechFileHandle, &tag, 4) != 4) {
        return 0;
    }
    if (_read(SpeechFileHandle, &size, 4) != 4) {
        return 0;
    }
    if (size < 0x12) {
        SpeechSourceFormat = malloc(0x12);
    } else {
        SpeechSourceFormat = malloc(size);
    }
    if (_read(SpeechFileHandle, SpeechSourceFormat, size) != (int)size) {
        return 0;
    }
    if (size <= 0x12) {
        *(short *)((char *)SpeechSourceFormat + 0x10) = 0;
    }
    while (_read(SpeechFileHandle, &tag, 4) == 4) {
        if (tag == 0x61746164) {
            break;
        }
        if (_read(SpeechFileHandle, &size, 4) != 4) {
            return 0;
        }
        p = (unsigned int *)malloc(size);
        if (_read(SpeechFileHandle, p, size) != (int)size) {
            free(p);
            return 0;
        }
        free(p);
    }
    if (tag != 0x61746164) {
        return 0;
    }
    if (_read(SpeechFileHandle, &SpeechDataSize, 4) != 4) {
        return 0;
    }
    SpeechDataOffset = _tell(SpeechFileHandle);
    return 1;
}
#endif

// FUNCTION: LEGOLAND 0x00498630
int SpeechLoadWavFile(const char *param_1) {
    char path[0x400];
    unsigned int out_size;

    if (SoundAvailable != 0) {
        // STRING: LEGOLAND 0x004bfeec
        strcpy(path, "speech\\");
        strcat(path, param_1);
        if (SpeechState == 0) {
            SpeechFileHandle = _open(path, 0x8000);
            if (SpeechFileHandle == -1) {
                // STRING: LEGOLAND 0x004bfee4
                sprintf(path, "%s%s%s", CdDrivePath, "speech\\", param_1);
                SpeechFileHandle = _open(path, 0x8000);
                if (SpeechFileHandle == -1) {
                    return 0;
                }
            }
            if (SpeechParseWavHeader() != 0) {
                SpeechRewindToData();
                SpeechAcmSrcSize = SpeechSourceFormat->nBlockAlign * 10;
                SpeechAcmSrcBuffer = malloc(SpeechAcmSrcSize);
                SpeechAcmDstBuffer = NULL;
                SpeechPcmFormat.wFormatTag = 1;
                SpeechPcmFormat.nChannels = SpeechSourceFormat->nChannels;
                SpeechPcmFormat.nSamplesPerSec = SpeechSourceFormat->nSamplesPerSec;
                SpeechPcmFormat.wBitsPerSample = 16;
                SpeechPcmFormat.nBlockAlign = SpeechPcmFormat.nChannels * 2;
                SpeechPcmFormat.nAvgBytesPerSec = SpeechPcmFormat.nBlockAlign * SpeechPcmFormat.nSamplesPerSec;
                SpeechPcmFormat.cbSize = 0;
                acmStreamOpen(&SpeechAcmStream, NULL, SpeechSourceFormat, &SpeechPcmFormat, NULL, 0, 0, 4);
                acmStreamSize(SpeechAcmStream, SpeechAcmSrcSize, &out_size, 0);
                SpeechAcmDstBuffer = malloc(out_size);
                SpeechAcmHeader.cbStruct = sizeof(SpeechAcmHeader);
                SpeechAcmHeader.fdwStatus = 0;
                SpeechAcmHeader.pbSrc = SpeechAcmSrcBuffer;
                SpeechAcmHeader.cbSrcLength = SpeechAcmSrcSize;
                SpeechAcmHeader.pbDst = SpeechAcmDstBuffer;
                SpeechAcmHeader.cbDstLength = out_size;
                acmStreamPrepareHeader(SpeechAcmStream, &SpeechAcmHeader, 0);
                SpeechSoundBuffer = KLIBAUDIO_CreateAVISoundBuffer(&SpeechPcmFormat, 0xa000);
                ((struct KLIBAUDIO_Vtbl *)((struct KLIBAUDIO_Object *)SpeechSoundBuffer)->vtable)->func_3c(SpeechSoundBuffer, SpeechVolume);
                SpeechState = 1;
                SpeechResetBuffers();
                return 1;
            }
            _close(SpeechFileHandle);
        }
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x00498870
void SpeechResetBuffers(void) {
    SpeechRawReadPos = 0;
    SpeechRawWritePos = 0;
    SpeechChunkIndex = 0;
    memset(SpeechChunkByteCounts, 0, sizeof(SpeechChunkByteCounts));
    SpeechPcmReadPos = 0;
    SpeechPcmWritePos = 0;
    SpeechPageIndex = 0;
    DAT_0079a844 = 0;
    SpeechConvertedSize = 0;
    SpeechConvertedUsed = 0;
}
// FUNCTION: LEGOLAND 0x004988c0
int SpeechStop(void) {
    if (SpeechState == 0 || SpeechState == 1) {
        return 0;
    }
    ((struct KLIBAUDIO_Stop *)((struct KLIBAUDIO_Object *)SpeechSoundBuffer)->vtable)->func_48(SpeechSoundBuffer);
    SpeechResetBuffers();
    SpeechRewindToData();
    SpeechState = 1;
    return 1;
}

// FUNCTION: LEGOLAND 0x00498900
void SpeechSetVolume(unsigned int param_1) {
    SpeechVolume = param_1;
    if (SpeechSoundBuffer != NULL) {
        ((struct KLIBAUDIO_Vtbl *)((struct KLIBAUDIO_Object *)SpeechSoundBuffer)->vtable)->func_3c(SpeechSoundBuffer, param_1);
    }
}

// FUNCTION: LEGOLAND 0x00498920
int SpeechCloseFile(void) {
    if (SpeechState == 0) {
        return 0;
    }
    SpeechStop();
    acmStreamUnprepareHeader(SpeechAcmStream, &SpeechAcmHeader, 0);
    acmStreamClose(SpeechAcmStream, 0);
    _close(SpeechFileHandle);
    KLIBAUDIO_DestroyAVISoundBuffer((struct AVISoundBuffer *)SpeechSoundBuffer);
    SpeechSoundBuffer = NULL;
    free(SpeechAcmSrcBuffer);
    free(SpeechAcmDstBuffer);
    free(SpeechSourceFormat);
    SpeechState = 0;
    return 1;
}

// FUNCTION: LEGOLAND 0x004989b0
int SpeechFillSoundBuffer(void) {
    unsigned char buf[0x1000];
    void *ptr1;
    unsigned int size1;
    int n;

    if (SpeechState == 2 || SpeechState == 0) {
        return 0;
    }
    ((struct KLIBAUDIO_Buf *)((struct KLIBAUDIO_Object *)SpeechSoundBuffer)->vtable)->Stop(SpeechSoundBuffer);
    ((struct KLIBAUDIO_Buf *)((struct KLIBAUDIO_Object *)SpeechSoundBuffer)->vtable)->SetPos(SpeechSoundBuffer, 0);
    SpeechPageIndex = 0;
    if (((struct KLIBAUDIO_Buf *)((struct KLIBAUDIO_Object *)SpeechSoundBuffer)->vtable)->Lock(SpeechSoundBuffer, 0, 0xa000, &ptr1, &size1, NULL, NULL, 0) == 0) {
        if (SpeechPageIndex < 10) {
            do {
                n = SpeechReadPcmBuffer(buf, 0x1000);
                memcpy((unsigned char *)ptr1 + SpeechPageIndex * 0x1000, buf, n);
                if (n < 0x1000) {
                    memset((unsigned char *)ptr1 + SpeechPageIndex * 0x1000 + n, 0, 0x1000 - n);
                }
                if (n != 0) {
                    DAT_0079a844++;
                }
                SpeechPageIndex++;
            } while (SpeechPageIndex < 10);
        }
        SpeechPageIndex = 0;
        ((struct KLIBAUDIO_Buf *)((struct KLIBAUDIO_Object *)SpeechSoundBuffer)->vtable)->Unlock(SpeechSoundBuffer, ptr1, size1, NULL, 0);
    }
    SpeechState = 2;
    return 1;
}

// FUNCTION: LEGOLAND 0x00498b00
int SpeechPlay(void) {
    if (SpeechState == 0 || SpeechState == 3) {
        return 0;
    }
    SpeechFillSoundBuffer();
    ((struct KLIBAUDIO_Vtbl *)((struct KLIBAUDIO_Object *)SpeechSoundBuffer)->vtable)->func_30(SpeechSoundBuffer, 0, 0, 1);
    SpeechState = 3;
    return 1;
}

// FUNCTION: LEGOLAND 0x00498b40
int SpeechStreamUpdate(void) {
    unsigned char buf[0x1000];
    void *ptr1;
    int play;
    unsigned int size1;
    unsigned int write;
    int n;

    if (SpeechState == 0 || SpeechState == 1) {
        return 0;
    }
    if (SpeechState == 2) {
        SpeechFillPcmBuffer();
        return 0;
    }
    if (!(((struct KLIBAUDIO_Buf *)((struct KLIBAUDIO_Object *)SpeechSoundBuffer)->vtable)->GetCurrentPosition(SpeechSoundBuffer, &play, &write) != 0 || play < SpeechPageIndex * 0x1000 || play >= (SpeechPageIndex + 1) * 0x1000)) {
        return 1;
    }
    do {
        if (((struct KLIBAUDIO_Buf *)((struct KLIBAUDIO_Object *)SpeechSoundBuffer)->vtable)->Lock(SpeechSoundBuffer, SpeechPageIndex * 0x1000, 0x1000, &ptr1, &size1, NULL, NULL, 0) == 0) {
            n = SpeechReadPcmBuffer(buf, size1);
            if (n == 0x1000) {
                memcpy(ptr1, buf, 0x1000);
                DAT_0079a844++;
            } else {
                memcpy(ptr1, buf, n);
                memset((unsigned char *)ptr1 + n, 0, 0x1000 - n);
                if (n != 0) {
                    DAT_0079a844++;
                }
            }
            ((struct KLIBAUDIO_Buf *)((struct KLIBAUDIO_Object *)SpeechSoundBuffer)->vtable)->Unlock(SpeechSoundBuffer, ptr1, size1, NULL, 0);
            SpeechPageIndex++;
            if (SpeechPageIndex >= 10) {
                SpeechPageIndex = 0;
            }
        }
        DAT_0079a844--;
        if (DAT_0079a844 == 0) {
            SpeechStop();
            return 0;
        }
    } while (((struct KLIBAUDIO_Buf *)((struct KLIBAUDIO_Object *)SpeechSoundBuffer)->vtable)->GetCurrentPosition(SpeechSoundBuffer, &play, &write) != 0 || play < SpeechPageIndex * 0x1000 || play >= (SpeechPageIndex + 1) * 0x1000);
    return 1;
}

// FUNCTION: LEGOLAND 0x00498cf0
int SpeechIsPlaying(void) {
    return SpeechState == 3;
}
