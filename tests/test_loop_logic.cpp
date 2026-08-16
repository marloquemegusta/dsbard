#include <cassert>
#include "loop_logic.h"

int main() {
    using namespace loop_logic;
    assert(phase_ms(0, 1000) == 0);
    assert(phase_ms(1250, 1000) == 250);
    assert(phase_ms(50, 0) == 0);
    assert(wrap_track(0, -1) == 2);
    assert(wrap_track(2, 1) == 0);
    NoteEvent event{100, 200, 60, 0};
    assert(!event_active(event, 99));
    assert(event_active(event, 100));
    assert(!event_active(event, 200));
    Track track{};
    track.origin_ms = 5000;
    assert(phase_ms(7250 - track.origin_ms, 1000) == 250);
    int previous_midi = midi_for_key(4, key_for_x(0));
    for (int x = 1; x < 256; ++x) {
        int midi = midi_for_key(4, key_for_x((uint16_t)x));
        assert(midi >= previous_midi);
        previous_midi = midi;
    }
    assert(key_for_x(0) == 0);
    assert(key_for_x(21) == 0);
    assert(key_for_x(22) == 1);
    assert(key_for_x(255) == 11);
    int previous_frequency = frequency_for_midi(midi_for_key(4, 0));
    for (int key = 1; key < 12; ++key) {
        int frequency = frequency_for_midi(midi_for_key(4, key));
        assert(frequency > previous_frequency);
        previous_frequency = frequency;
    }
    assert(frequency_for_midi(midi_for_key(5, 0)) > frequency_for_midi(midi_for_key(4, 11)));
    assert(midi_for_key(4, 4) == 64);
    assert(midi_for_key(4, 5) == 65);
    assert(frequency_for_midi(64) == 329);
    assert(frequency_for_midi(65) == 349);
    assert(sizeof(track.events) / sizeof(track.events[0]) == 96);
    return 0;
}
