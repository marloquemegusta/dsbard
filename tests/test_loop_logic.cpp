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
    track.duration_ms = 900;
    track.phrase_count = 1;
    track.phrases[0] = {0, 900, 1, 3};
    assert(track.phrases[0].start_ms == 0);
    assert(track.phrases[0].repeat_count == 3);
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
    assert(journal_value_for_key(0) == 0);
    assert(journal_value_for_key(1) == 1);
    assert(journal_value_for_key(11) == 11);
    assert(frequency_for_midi(64) == 329);
    assert(frequency_for_midi(65) == 349);
    assert(psg_frequency_for_midi(64) == 329 * 8);
    assert(psg_frequency_for_midi(65) == 349 * 8);
    assert(frequency_for_midi(midi_for_key(2, 0)) == 65);
    assert(frequency_for_midi(midi_for_key(3, 0)) == 130);
    assert(frequency_for_midi(midi_for_key(4, 0)) == 261);
    assert(frequency_for_midi(midi_for_key(5, 0)) == 522);
    int previous_octave_frequency = frequency_for_midi(midi_for_key(2, 0));
    for (int test_octave = 2; test_octave <= 6; ++test_octave) {
        for (int test_key = 0; test_key < 12; ++test_key) {
            int frequency = frequency_for_midi(midi_for_key(test_octave, test_key));
            if (test_octave == 2 && test_key == 0) {
                assert(frequency == previous_octave_frequency);
            } else {
                assert(frequency > previous_octave_frequency);
            }
            previous_octave_frequency = frequency;
        }
        assert(frequency_for_midi(midi_for_key(test_octave, 5)) >
               frequency_for_midi(midi_for_key(test_octave, 4)));
    }
    assert(sizeof(track.events) / sizeof(track.events[0]) == MAX_TRACK_EVENTS);
    return 0;
}
