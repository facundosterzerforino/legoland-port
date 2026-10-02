#pragma once

struct KLIBAUDIO_Vtbl {
    unsigned char pad_0[0x30];
    void(__stdcall *func_30)(void *self, int a1, int a2, int a3);
    unsigned char pad_34[0x3c - 0x34];
    void(__stdcall *func_3c)(void *self, unsigned int a1);
};

struct KLIBAUDIO_Stop {
    unsigned char pad_0[0x48];
    void(__stdcall *func_48)(struct KLIBAUDIO_Object *self);
};

struct KLIBAUDIO_Buf {
    unsigned char pad_0[0x10];
    int(__stdcall *GetCurrentPosition)(void *self, int *play, unsigned int *write);
    unsigned char pad_14[0x2c - 0x14];
    int(__stdcall *Lock)(void *self, unsigned int off, unsigned int bytes, void **p1, unsigned int *s1, void **p2, unsigned int *s2, unsigned int flags);
    unsigned char pad_30[4];
    void(__stdcall *SetPos)(void *self, unsigned int pos);
    unsigned char pad_38[0x48 - 0x38];
    void(__stdcall *Stop)(void *self);
    void(__stdcall *Unlock)(void *self, void *p1, unsigned int s1, void *p2, unsigned int s2);
};

struct KLIBAUDIO_Object {
    void *vtable;
};

int SpeechFillSoundBuffer(void);
void FUN_00498100(void);
void SpeechRewindToData(void);
int SpeechLoadWavFile(const char *param_1);
void SpeechResetBuffers(void);
int SpeechStop(void);
int SpeechCloseFile(void);
int FUN_00498b00(void);
int SpeechStreamUpdate(void);
int FUN_00498cf0(void);
void SpeechSetVolume(unsigned int param_1);
