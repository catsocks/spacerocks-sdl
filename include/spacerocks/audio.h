#pragma once

#include <AL/al.h>
#include <AL/alc.h>

#include "spacerocks/assets.h"

enum {
    AUDIO_FREQUENCY = 44100,
    AUDIO_FORMAT = AL_FORMAT_MONO16,
};

enum AudioBuffer {
    AUDIO_BUFFER_EXPLOSION_BIG,
    AUDIO_BUFFER_EXPLOSION_MEDIUM,
    AUDIO_BUFFER_EXPLOSION_SMALL,
    AUDIO_BUFFER_PULSE_HIGH,
    AUDIO_BUFFER_PULSE_LOW,
    AUDIO_BUFFER_PULSE_SILENCE_1,
    AUDIO_BUFFER_PULSE_SILENCE_2,
    AUDIO_BUFFER_SAUCER_FIRE,
    AUDIO_BUFFER_SHIP_FIRE,
    AUDIO_BUFFER_SHIP_THRUST,
    AUDIO_BUFFER_WARNING_BIG_SAUCER,
    AUDIO_BUFFER_WARNING_SMALL_SAUCER,
    NUM_AUDIO_BUFFERS,
};

typedef enum AudioBuffer AudioBuffer;

enum AudioSource {
    AUDIO_SOURCE_EXPLOSION,
    AUDIO_SOURCE_PULSE,
    AUDIO_SOURCE_SAUCER_FIRE,
    AUDIO_SOURCE_SAUCER_WARNING,
    AUDIO_SOURCE_SHIP_FIRE,
    AUDIO_SOURCE_SHIP_THRUST,
    NUM_AUDIO_SOURCES,
};

typedef enum AudioSource AudioSource;

struct Audio {
    ALCdevice *device;
    ALuint buffers[NUM_AUDIO_BUFFERS];
    ALuint sources[NUM_AUDIO_SOURCES];
    bool muted;
};

typedef struct Audio Audio;

bool init_audio(Audio *audio);
void mute_audio(Audio *audio);
void unmute_audio(Audio *audio);
bool pause_audio_device(Audio *audio);
bool unpause_audio_device(Audio *audio);
void stop_all_audio_sources(Audio *audio);
void deinit_audio(Audio *audio);
