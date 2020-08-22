#include <stdio.h>
#include <pulse/pulseaudio.h>

/** Gets called when PulseAudio successfully fetched sink information. */
static void sink_info_callback(pa_context* c, const pa_sink_info* i, int eol, void* userdata)
{
    if (!i) {
        return;
    }

    pa_volume_t* last_volume = (pa_volume_t*)userdata;

    pa_volume_t new_volume = pa_cvolume_avg(&(i->volume));

    // Skip events that don't reflect actual volume updates.
    if (*last_volume == new_volume) {
        return;
    }
    *last_volume = new_volume;

    float volume = (float)new_volume / (float)PA_VOLUME_NORM;

    fprintf(stdout, "%.0f\n", volume * 100.0f);
    fflush(stdout);
}

/** Gets called when some sink event occurred. */
static void sink_update_handler(pa_context* c, pa_subscription_event_type_t t, uint32_t idx, void* userdata) {
    if ((t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK) == PA_SUBSCRIPTION_EVENT_SINK) {
        pa_context_get_sink_info_by_index(c, idx, sink_info_callback, userdata);
    }
}

/** Gets called when PulseAudio context state gets updated. */
static void context_state_update_handler(pa_context* c, void* userdata) {
    switch (pa_context_get_state(c)) {
    case PA_CONTEXT_CONNECTING:
    case PA_CONTEXT_AUTHORIZING:
    case PA_CONTEXT_SETTING_NAME:
        break;

    case PA_CONTEXT_READY:
        fprintf(stderr, "PulseAudio connection established.\n");

        // Set callback for sink updates.
        pa_context_set_subscribe_callback(c, sink_update_handler, userdata);

        // Subscribe to sink updates.
        pa_context_subscribe(c, PA_SUBSCRIPTION_MASK_SINK, NULL, NULL);
        break;

    case PA_CONTEXT_TERMINATED:
        fprintf(stderr, "PulseAudio connection terminated.\n");
        break;

    case PA_CONTEXT_FAILED:
    default:
        fprintf(stderr, "Connection failure: %s\n", pa_strerror(pa_context_errno(c)));
        break;
    }
}

int main() {
    pa_mainloop* _mainloop = pa_mainloop_new();
    if (!_mainloop) {
        fprintf(stderr, "pa_mainloop_new() failed.\n");
        return 1;
    }

    pa_context* _context = pa_context_new(pa_mainloop_get_api(_mainloop), "listenvol");
    if (!_context) {
        fprintf(stderr, "pa_context_new() failed\n");
        return 1;
    }

    pa_volume_t last_volume;

    // Set callback for state updates.
    pa_context_set_state_callback(_context, context_state_update_handler, &last_volume);

    // Subscribe to context state updates.
    pa_context_connect(_context, NULL, PA_CONTEXT_NOAUTOSPAWN, NULL);

    // Run PulseAudio main loop.
    int ret = 1;
    pa_mainloop_run(_mainloop, &ret);

    pa_context_unref(_context);
    pa_mainloop_free(_mainloop);

    return ret;
}
