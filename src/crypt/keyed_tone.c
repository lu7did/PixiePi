/*

    1/7/2017, Rick Koch / N1GP adapted from midicw.c
    found here: https://github.com/recri/keyer

*/

extern "C" {
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <libgen.h>
#include <jack/jack.h>
#include "../lib/keyed_tone.h"

int keyed_tone_mute = 0;

/*Our output port*/
jack_port_t *output_port;

typedef jack_default_audio_sample_t sample_t;

/*The current sample rate*/
jack_nframes_t sr;

keyed_tone_t tone;
struct {
    int	freq;	/* frequency of keyed tone */
    int	gain;	/* gain in decibels of keyed tone */
    int	rise;	/* rise time in milliseconds */
    int	fall;	/* fall time in milliseconds */
    int	srate;	/* samples per second */
} tone_opts = {
    700, -6, 5, 5, 48000
};

static jack_client_t *client;

int process (jack_nframes_t nframes, void *arg) {
    /*grab our output buffer*/
    sample_t *out = (sample_t *) jack_port_get_buffer (output_port, nframes);
    jack_nframes_t i;

    /*For each required sample*/
    for(i=0; i<nframes; i++) {
        if(keyed_tone_mute == 1) {
            keyed_tone_mute = 0;
            keyed_tone_off(&tone);
        }
        else if(keyed_tone_mute == 2) {
            keyed_tone_mute = 0;
            keyed_tone_on(&tone);
        }

        out[i] = keyed_tone_process(&tone);
    }

    return 0;
}

int srate (jack_nframes_t nframes, void *arg) {
    printf ("the sample rate is now %lu/sec\n", nframes);
    sr=nframes;
    return 0;
}

void error (const char *desc) {
    fprintf (stderr, "JACK error: %s\n", desc);
}

void jack_shutdown (void *arg) {
    exit (1);
}

void keyed_tone_close() {
    jack_client_close (client);
}

int keyed_tone_start(long volume, double freq, int envelope) {
    const char **ports;
    const char *clientname = "iambic-keyer";

    tone_opts.freq = freq;
    tone_opts.gain = volume;
    tone_opts.rise = tone_opts.fall = envelope;

    jack_set_error_function (error);

    /* Connect to the JACK daemon */
    if ((client = jack_client_open (clientname, JackNullOption, NULL)) == 0) {
        fprintf(stderr, "jack server not running?\n");
        return 1;
    }

    /* tell the JACK server to call `process()' whenever
       there is work to be done.  */
    jack_set_process_callback (client, process, 0);

    /* tell the JACK server to call `srate()' whenever
       the sample rate of the system changes.  */
    jack_set_sample_rate_callback (client, srate, 0);

    /* tell the JACK server to call `jack_shutdown()' if
       it ever shuts down, either entirely, or if it
       just decides to stop calling us.  */
    jack_on_shutdown (client, jack_shutdown, 0);

    /* display the current sample rate. once the client is activated
       (see below), you should rely on your own sample rate
       callback (see above) for this value.  */
    printf ("engine sample rate: %lu\n", jack_get_sample_rate (client));

    output_port = jack_port_register (client, "output", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

    tone_opts.srate = jack_get_sample_rate (client);
    keyed_tone_init(&tone, tone_opts.gain, tone_opts.freq, tone_opts.rise, tone_opts.fall, tone_opts.srate);

    /* tell the JACK server that we are ready to roll */
    if (jack_activate (client)) {
        fprintf (stderr, "cannot activate client\n");
        return 1;
    }

    if ((ports = jack_get_ports (client, NULL, NULL, JackPortIsPhysical|JackPortIsInput)) == NULL) {
        fprintf(stderr, "Cannot find any physical playback ports\n");
        return 1;
    }

    if (jack_connect (client, jack_port_name (output_port), ports[0])) {
        fprintf (stderr, "cannot connect output ports\n");
    }

    free (ports);
    return 0;
}
}
