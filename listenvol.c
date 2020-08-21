#include <stdio.h>
#include <pulse/pulseaudio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

void sink_info_callback(pa_context *c, const pa_sink_info *i, int eol, void *userdata)
{
    if (i)
    {
        float volume = (float)pa_cvolume_avg(&(i->volume)) / (float)PA_VOLUME_NORM;
        fprintf(stdout, "%.0f\n", volume * 100.0f);
        fflush(stdout);
    }
}

void subscribe_callback(pa_context *c, pa_subscription_event_type_t t, uint32_t idx, void *userdata)
{
    if ((t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK) == PA_SUBSCRIPTION_EVENT_SINK)
    {
        pa_context_get_sink_info_by_index(c, idx, sink_info_callback, userdata);
    }
}

void context_state_callback(pa_context *c, void *userdata)
{
    switch (pa_context_get_state(c))
    {
    case PA_CONTEXT_CONNECTING:
    case PA_CONTEXT_AUTHORIZING:
    case PA_CONTEXT_SETTING_NAME:
        break;

    case PA_CONTEXT_READY:
        // printf("PulseAudio connection established.\n");

        pa_context_set_subscribe_callback(c, subscribe_callback, userdata);
        pa_context_subscribe(c, PA_SUBSCRIPTION_MASK_SINK, NULL, NULL);
        break;

    case PA_CONTEXT_TERMINATED:
        // pa->quit(0);
        printf("PulseAudio connection terminated.\n");
        break;

    case PA_CONTEXT_FAILED:
    default:
        fprintf(stderr, "Connection failure: %s\n", pa_strerror(pa_context_errno(c)));
        // pa->quit(1);
        break;
    }
}

int main()
{
    pa_mainloop *_mainloop = pa_mainloop_new();
    if (!_mainloop)
    {
        fprintf(stderr, "pa_mainloop_new() failed.\n");
        return 1;
    }

    pa_mainloop_api *_mainloop_api = pa_mainloop_get_api(_mainloop);

    if (pa_signal_init(_mainloop_api) != 0)
    {
        fprintf(stderr, "pa_signal_init() failed\n");
        return 1;
    }
    pa_context *_context = pa_context_new(_mainloop_api, "PulseAudio Test");
    if (!_context)
    {
        fprintf(stderr, "pa_context_new() failed\n");
        return 1;
    }

    pa_context_set_state_callback(_context, context_state_callback, NULL);

    pa_context_connect(_context, NULL, PA_CONTEXT_NOAUTOSPAWN, NULL);

    int ret = 1;
    pa_mainloop_run(_mainloop, &ret);

    return 0;
}
