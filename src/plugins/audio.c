/*
 * Audio test plugin for Test Gear
 *
 * For WAV file playback libaudiofile is used for parsing the WAV file.
 *
 * Copyright (c) 2012-2014, Martin Lund
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT
 * HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <linux/soundcard.h>
#include <alsa/asoundlib.h>
#include <alsa/pcm.h>
#include <audiofile.h>
#include "testgear/plugin.h"

static int audio_load(void)
{
    // Set defaults
    set_string("device", "default");
    set_int("rate", 44100);
    set_string("mode", "stereo");
    set_string("tone-type", "sine");
    set_int("tone-frequency", 1000);
    set_int("tone-time", 2000);
    set_string("wav-file", "test.wav");

    return 0;
}

static int audio_play_wav(void)
{
    int error;
    snd_pcm_t *pcm_handle;
    snd_pcm_hw_params_t *hw_params;
    snd_pcm_uframes_t frames;
    int sample_format, sample_width;
    double frequency;
    unsigned int rate;
    float frame_size;
    int channels;

    char *device = get_string("device");
    char *wav_file = get_string("wav-file");

    // Open audio file
    AFfilehandle filehandle = afOpenFile(wav_file, "r", NULL);
    if (filehandle == AF_NULL_FILEHANDLE)
    {
        printf("Error: Could not open audio file\n");
        return EXIT_FAILURE;
    }

    // Parse WAV file meta data
    AFframecount framecount = afGetFrameCount(filehandle, AF_DEFAULT_TRACK);
    channels = afGetVirtualChannels(filehandle, AF_DEFAULT_TRACK);
    afGetVirtualSampleFormat(filehandle, AF_DEFAULT_TRACK, &sample_format, &sample_width);
    rate = frequency = afGetRate(filehandle, AF_DEFAULT_TRACK);
    frame_size = afGetVirtualFrameSize(filehandle, AF_DEFAULT_TRACK, 1);

    // WAV file sanity checks
    if ((sample_format != AF_SAMPFMT_TWOSCOMP) && (sample_format != AF_SAMPFMT_UNSIGNED))
    {
        printf("Error: The audio file must contain integer data in two's complement or unsigned format.\n");
        return EXIT_FAILURE;
    }

    if (channels > 2)
    {
        printf("Error: More than 2 channels is not supported.\n");
        return EXIT_FAILURE;
    }

    // Open audio device
    if ((error = snd_pcm_open(&pcm_handle, device, SND_PCM_STREAM_PLAYBACK, 0)) < 0)
    {
        printf("Error: %s\n", snd_strerror(error));
        return EXIT_FAILURE;
    }

    // Configure audio hardware
    snd_pcm_hw_params_alloca(&hw_params);
    snd_pcm_hw_params_any(pcm_handle, hw_params);

    if ((error = snd_pcm_hw_params_set_access(pcm_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
        printf("Error: %s\n", snd_strerror(error));

    if ((error = snd_pcm_hw_params_set_channels(pcm_handle, hw_params, channels)) < 0)
        printf("Error: %s\n", snd_strerror(error));

    if ((error = snd_pcm_hw_params_set_rate_near(pcm_handle, hw_params, &rate, 0)) < 0)
        printf("Error: %s\n", snd_strerror(error));

    switch (sample_width)
    {
        case 8:
            switch(sample_format)
            {
                case AF_SAMPFMT_TWOSCOMP:
                    // Signed 8-bit
                    if ((error = snd_pcm_hw_params_set_format(pcm_handle, hw_params, SND_PCM_FORMAT_S8)) < 0)
                        printf("Error: %s\n", snd_strerror(error));
                    break;
                case AF_SAMPFMT_UNSIGNED:
                    // Unsigned 8-bit
                    if ((error = snd_pcm_hw_params_set_format(pcm_handle, hw_params, SND_PCM_FORMAT_U8)) < 0)
                        printf("Error: %s\n", snd_strerror(error));
                    break;
            }
            break;
        case 16:
            switch(sample_format)
            {
                case AF_SAMPFMT_TWOSCOMP:
                    // Signed 16-bit
                    if ((error = snd_pcm_hw_params_set_format(pcm_handle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0)
                        printf("Error: %s\n", snd_strerror(error));
                    break;
                case AF_SAMPFMT_UNSIGNED:
                    // Unsigned 16-bit
                    if ((error = snd_pcm_hw_params_set_format(pcm_handle, hw_params, SND_PCM_FORMAT_U8)) < 0)
                        printf("Error: %s\n", snd_strerror(error));
                    break;
            }
            break;
    }

    if ((error = snd_pcm_hw_params(pcm_handle, hw_params)) < 0)
        printf("Error: %s\n", snd_strerror(error));

    snd_pcm_hw_params_get_period_size(hw_params, &frames, 0);

    // Allocate audio buffer
    void *buffer = malloc(frames * frame_size);
    if (buffer == NULL)
    {
        printf("Error: Could not allocate audio buffer.\n");
        return EXIT_FAILURE;
    }

    // Play audio file
    while (1)
    {
        AFframecount frames_read = afReadFrames(filehandle, AF_DEFAULT_TRACK, buffer, frames);
        if (frames_read <= 0)
            break;

        if ((error = snd_pcm_writei(pcm_handle, buffer, frames_read)) == -EPIPE)
        {
            printf("Audio buffer Underrun!\n");
            snd_pcm_prepare(pcm_handle);
        } else if (error < 0)
        {
            printf("Error: %s\n", snd_strerror(error));
            break;
        }
    }

    // Cleanup
    snd_pcm_drain(pcm_handle);
    snd_pcm_close(pcm_handle);
    free(buffer);
    afCloseFile(filehandle);

    return 0;
}

static int audio_generate_tone(void)
{
    int i, error;
    int buffer_size;
    int format;
    snd_pcm_t *handle;
    snd_pcm_sframes_t frames;
    snd_output_t *output = NULL;

    // Audio device settings
    char *device = get_string("device");
    char *mode = get_string("mode");
    int rate = get_int("rate");

    // Tone settings
    char *tone_type = get_string("tone-type");
    int tone_frequency = get_int("tone-frequency");
    int tone_time = get_int("tone-time");

    // Allocate buffer for 1 sec tone sample
    buffer_size = rate;
    float buffer[buffer_size];

    if (strcmp(tone_type,"sine") == 0)
    {
        // Generate sine tone sample
        for (i=0; i<buffer_size; i++)
            buffer[i] = (sin(2*M_PI*tone_frequency/rate*i));

        format = SND_PCM_FORMAT_FLOAT;
    }
    else if (strcmp(tone_type,"noise") == 0)
    {
        // Generate white noise
        srandom(time(NULL));
        for (i=0; i<buffer_size; i++)
            buffer[i] = random() & 0xff;

        format = SND_PCM_FORMAT_U8;
    }
    else
    {
        printf("Unknown tone type\n");
        return -1;
    }

    // Open audio device
    if ((error = snd_pcm_open(&handle, device, SND_PCM_STREAM_PLAYBACK, 0)) < 0)
    {
        printf("Playback open error: %s\n", snd_strerror(error));
        exit(EXIT_FAILURE);
    }

    // Configure audio device
    if ((error = snd_pcm_set_params(handle,
                    format,
                    SND_PCM_ACCESS_RW_INTERLEAVED,
                    1, // channels
                    rate,
                    1, // Soft resample
                    500000)) < 0)
    {
        printf("Playback open error: %s\n", snd_strerror(error));
        return EXIT_FAILURE;
    }

    // Write tone buffer to audio device
    for (i=0; i<tone_time; i++)
    {
        frames = snd_pcm_writei(handle, buffer, buffer_size);

        if (frames < 0)
            error = snd_pcm_recover(handle, frames, 0);

        if (error < 0)
        {
            printf("snd_pcm_writei() failed: %s\n", snd_strerror(error));
            break;
        }
    }

    // Wait for all pending frames to be played
    snd_pcm_drain(handle);

    // Close audio device
    snd_pcm_close(handle);

    return 0;
}

// Plugin properties
static struct plugin_properties audio_properties[] =
{
    {   .name = "device",
        .type = STRING,
        .description = "Alsa audio device" },

    {   .name = "rate",
        .type = INT,
        .description = "Audio device sampling rate [Hz]" },

    {   .name = "mode",
        .type = STRING,
        .description = "Audio mode (stereo, mono)" },

    {   .name = "tone-type",
        .type = STRING,
        .description = "Tone type (sine, square, triangle, pulse, sawtooth, noise)" },

    {   .name = "tone-frequency",
        .type = INT,
        .description = "Tone frequency [Hz]" },

    {   .name = "tone-time",
        .type = INT,
        .description = "Tone duration [s]" },

    {   .name = "wav-file",
        .type = STRING,
        .description = "WAV file for playback" },

    {   .name = "generate-tone",
        .type = COMMAND,
        .function = audio_generate_tone,
        .description = "Generate sine tone" },

    {   .name = "play-wav",
        .type = COMMAND,
        .function = audio_play_wav,
        .description = "Generate wav file" },

    { }
};

// Plugin configuration
struct plugin audio =
{
    .name = "audio",
    .version = "0.1",
    .description = "Audio plugin",
    .author = "Martin Lund",
    .license = "BSD-3",
    .properties = audio_properties,
    .load = audio_load,
};

plugin_register(audio);
