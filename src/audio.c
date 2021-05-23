#include "spacerocks/audio.h"

typedef void (*ALC_DEVICE_PAUSE_SOFT)(ALCdevice *);
typedef void (*ALC_DEVICE_RESUME_SOFT)(ALCdevice *);

static ALC_DEVICE_PAUSE_SOFT alcDevicePauseSOFT;
static ALC_DEVICE_RESUME_SOFT alcDeviceResumeSOFT;

bool init_audio(Audio *audio) {
    ALCcontext *ctx = alcGetCurrentContext();
    if (ctx == NULL) {
        return false;
    }

    audio->device = alcGetContextsDevice(ctx);
    SDL_assert(alGetError() == AL_NO_ERROR);

    if (alcIsExtensionPresent(audio->device, "ALC_SOFT_pause_device") ==
        AL_TRUE) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
        alcDevicePauseSOFT =
            alcGetProcAddress(audio->device, "alcDevicePauseSOFT");
        alcDeviceResumeSOFT =
            alcGetProcAddress(audio->device, "alcDeviceResumeSOFT");
#pragma GCC diagnostic pop
        SDL_assert(alcDevicePauseSOFT && alcDeviceResumeSOFT);
    } else {
        SDL_assert(alGetError() == AL_NO_ERROR);
    }

    Sounds sounds = {0};
    if (!init_sounds(&sounds)) {
        return false;
    }

    alGenBuffers(NUM_AUDIO_BUFFERS, audio->buffers);
    SDL_assert(alGetError() == AL_NO_ERROR);

    ALuint sounds_buffer[NUM_SOUNDS] = {
        [SOUND_EXPLOSION_BIG] = AUDIO_BUFFER_EXPLOSION_BIG,
        [SOUND_EXPLOSION_MEDIUM] = AUDIO_BUFFER_EXPLOSION_MEDIUM,
        [SOUND_EXPLOSION_SMALL] = AUDIO_BUFFER_EXPLOSION_SMALL,
        [SOUND_PULSE_HIGH] = AUDIO_BUFFER_PULSE_HIGH,
        [SOUND_PULSE_LOW] = AUDIO_BUFFER_PULSE_LOW,
        [SOUND_SAUCER_FIRE] = AUDIO_BUFFER_SAUCER_FIRE,
        [SOUND_WARNING_BIG_SAUCER] = AUDIO_BUFFER_WARNING_BIG_SAUCER,
        [SOUND_WARNING_SMALL_SAUCER] = AUDIO_BUFFER_WARNING_SMALL_SAUCER,
        [SOUND_SHIP_FIRE] = AUDIO_BUFFER_SHIP_FIRE,
        [SOUND_SHIP_THRUST] = AUDIO_BUFFER_SHIP_THRUST,
    };
    for (int i = 0; i < NUM_SOUNDS; i++) {
        ALuint buffer = sounds_buffer[i];

        // expected_format should reflect AUDIO_FORMAT.
        SDL_AudioFormat expected_format = AUDIO_S16LSB;
        SDL_assert(sounds.data[i].spec.format == expected_format);
        if (sounds.data[i].spec.format != expected_format) {
            continue;
        }

        alBufferData(audio->buffers[buffer], AL_FORMAT_MONO16,
                     sounds.data[i].buffer, sounds.data[i].buffer_size,
                     sounds.data[i].spec.freq);
        SDL_assert(alGetError() == AL_NO_ERROR);
    }

    free_sounds(&sounds);

    alGenSources(NUM_AUDIO_SOURCES, audio->sources);
    SDL_assert(alGetError() == AL_NO_ERROR);

    return true;
}

void mute_audio(Audio *audio) {
    audio->muted = true;
    alListenerf(AL_GAIN, 0.0);
}

void unmute_audio(Audio *audio) {
    audio->muted = false;
    alListenerf(AL_GAIN, 1.0);
}

bool pause_audio_device(Audio *audio) {
    if (alcDevicePauseSOFT == NULL) {
        return false;
    }
    alcDevicePauseSOFT(audio->device);
    return true;
}

bool unpause_audio_device(Audio *audio) {
    if (alcDeviceResumeSOFT == NULL) {
        return false;
    }
    alcDeviceResumeSOFT(audio->device);
    return true;
}

void stop_all_audio_sources(Audio *audio) {
    for (int i = 0; i < NUM_AUDIO_SOURCES; i++) {
        alSourceStop(audio->sources[i]);
    }
}

void deinit_audio(Audio *audio) {
    alDeleteSources(NUM_AUDIO_SOURCES, audio->sources);
    alDeleteBuffers(NUM_AUDIO_BUFFERS, audio->buffers);
}
