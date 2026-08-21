#include <cassert>
#include "loop_logic.h"

int main() {
    using namespace loop_logic;
    assert(phase_ms(0, 1000) == 0);
    assert(phase_ms(1250, 1000) == 250);
    assert(phase_ms(50, 0) == 0);
    assert(wrap_track(0, -1) == 2);
    assert(wrap_track(2, 1) == 0);
    NoteEvent event{100, 200, 60, 0, 1};
    assert(!event_active(event, 99));
    assert(event_active(event, 100));
    assert(!event_active(event, 200));
    assert(event.sustain_on_release == 1);
    assert(should_toggle_loop_on_release(true, false, true));
    assert(!should_toggle_loop_on_release(true, true, true));
    assert(!should_toggle_loop_on_release(true, false, false));
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
    TimelineSelection selection{500, 900, 100, true};
    normalize_selection(selection, 1000);
    assert(selection.range_start_ms == 100 && selection.range_end_ms == 900);
    TimelineSelection clamped{0, 2000, 3000, true};
    normalize_selection(clamped, 1000);
    assert(clamped.range_start_ms == 0 && clamped.range_end_ms == 1000);
    assert(seek_selection(selection, 1200, 1000) == 1000);
    assert(restart_selection(selection) == 100);
    bool stopped = false;
    assert(advance_transport(850, 275, 100, 900, true, stopped) == 325);
    assert(!stopped);
    assert(advance_transport(850, 275, 100, 900, false, stopped) == 900);
    assert(stopped);
    Phrase repeated{0, 1000, 1, 3};
    assert(expanded_phrase_start(repeated, 2) == 2000);
    assert(expanded_phrase_end(repeated, 2) == 3000);
    PulseGrid grid{1000};
    assert(grid.time(7) == 1750);
    assert(grid.snap(1124) == 1000);
    assert(grid.snap(1126) == 1250);
    assert(snap_loop_boundary(149, 1000) == 0);
    assert(snap_loop_boundary(501, 1000) == 1000);
    assert(snap_loop_duration(1, 1000) == 1000);
    assert(snap_loop_duration(2100, 1000) == 3000);
    assert(snap_loop_duration(3000, 1000) == 3000);
    uint32_t edges[] = {0, 1000, 2000, 3000};
    assert(snap_timeline_time(1010, 1000, edges, 4) == 1000);
    Track editable{};
    editable.event_count = 3;
    editable.events[0] = {0, 500, 60, 2, 1};
    editable.events[1] = {600, 1800, 61, 3, 0};
    editable.events[2] = {1900, 2400, 62, 4, 1};
    editable.events[1].sustain_level = 3;
    assert(edit_note_event(editable, 1, NOTE_END, 1300, 0, 3000, 1000));
    assert(editable.events[1].end_ms == 1250);
    assert(edit_note_event(editable, 1, NOTE_MOVE, 1000, 0, 3000, 1000));
    assert(editable.events[1].start_ms == 1000 && editable.events[1].end_ms == 1650);
    assert(editable.events[1].midi_note == 61 && editable.events[1].instrument_id == 3);
    assert(editable.events[1].sustain_on_release == 0);
    assert(editable.events[1].sustain_level == 3);
    assert(!edit_note_event(editable, 1, NOTE_START, 2000, 0, 3000, 1000));
    Track snapped{};
    snapped.event_count = 3;
    snapped.events[0] = {700, 800, 60, 2, 0};
    snapped.events[1] = {900, 2900, 61, 3, 0};
    snapped.events[2] = {3100, 3200, 62, 4, 1};
    snap_note_events(snapped, 1000);
    assert(snapped.events[0].start_ms == 750 && snapped.events[0].end_ms == 1000);
    assert(snapped.events[1].start_ms == 1000 && snapped.events[1].end_ms == 3000);
    assert(snapped.events[2].start_ms == 3000 && snapped.events[2].end_ms == 3250);
    assert(snapped.events[2].sustain_on_release == 1);
    return 0;
}
