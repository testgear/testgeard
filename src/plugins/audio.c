/*
 * Audio plugin for Test Gear
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <alsa/asoundlib.h>
#include <alsa/pcm.h>
#include "testgear/plugin.h"

#define BUFFER_SIZE_MAX 48000

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


// Plugin commands
static struct plugin_command_table audio_commands[] =
{
    {   .name = "generate-tone",
        .function = audio_generate_tone,
        .description = "Generate sine tone" },

    { }
};


// Plugin variables
static struct plugin_var_table audio_vars[] =
{
    {   .name = "device",
        .type = STRING,
        .value = "default",
        .description = "Alsa audio device" },

    {   .name = "rate",
        .type = INT,
        .value = "44100",
        .description = "Audio device sampling rate [Hz]" },

    {   .name = "mode",
        .type = STRING,
        .value = "stereo",
        .description = "Audio mode (stereo, mono)" },

    {   .name = "tone-type",
        .type = STRING,
        .value = "sine",
        .description = "Tone type (sine, square, triangle, pulse, sawtooth, noise)" },

    {   .name = "tone-frequency",
        .type = INT,
        .value = "1000",
        .description = "Tone frequency [Hz]" },

    {   .name = "tone-time",
        .type = INT,
        .value = "2000",
        .description = "Tone duration [s]" },

    { }
};


// Plugin configuration
struct plugin audio =
{
    .name = "audio",
    .version = "0.1",
    .description = "Audio plugin",
    .author = "Martin Lund",
    .license = "Unknown",
    .commands = audio_commands,
    .vars = audio_vars,
};


plugin_register(audio);
