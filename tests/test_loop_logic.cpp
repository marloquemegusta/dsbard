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

    PendingNoteInsertion pending{};
    begin_note_insertion(pending, 640, 3, 1120, 1000, 0);
    assert(pending.active);
    assert(pending.event.start_ms == 1000);
    assert(pending.event.end_ms == 1250);
    assert(pending.event.midi_note == 640);
    assert(pending.event.instrument_id == 3);

    PendingNoteInsertion at_cycle_end{};
    begin_note_insertion(at_cycle_end, 65, 3, 2000, 1000, 0, 2000);
    assert(at_cycle_end.event.start_ms == 1750);
    assert(at_cycle_end.event.end_ms == 2000);

    Track composed{};
    composed.occupied = true;
    composed.duration_ms = 2000;
    composed.event_count = 1;
    composed.events[0] = {0, 500, 60, 1, 0};
    assert(commit_note_insertion(composed, pending));
    assert(!pending.active);
    assert(composed.event_count == 2);
    assert(composed.events[1].start_ms == 1000);
    assert(composed.events[1].end_ms == 1250);

    PendingNoteInsertion cancelled{};
    begin_note_insertion(cancelled, 62, 4, 500, 1000, 0);
    cancel_note_insertion(cancelled);
    assert(!cancelled.active);
    assert(cancelled.event.start_ms == 0 && cancelled.event.end_ms == 0);

    assert(!timeline_x_is_valid(90));
    assert(timeline_x_is_valid(91));
    assert(timeline_x_is_valid(251));
    assert(!timeline_x_is_valid(252));
    assert(timeline_ms_for_x(91, 2000) == 0);
    assert(timeline_ms_for_x(251, 2000) == 2000);

    Track mixed{};
    mixed.occupied = true;
    mixed.duration_ms = 3000;
    mixed.phrase_count = 2;
    mixed.phrases[0] = {0, 1000, 1, 1};
    mixed.phrases[1] = {2000, 1000, 1, 1};
    mixed.event_count = 3;
    mixed.events[0] = {100, 300, 60, 1, 0, 0};
    mixed.events[1] = {1250, 1500, 61, 2, 0, 0};
    mixed.events[2] = {2100, 2300, 62, 3, 0, 0};
    assert(find_direct_event_at(mixed, 1300) == 1);
    assert(find_direct_event_at(mixed, 200) == -1);

    PendingNoteInsertion inside_phrase{};
    begin_note_insertion(inside_phrase, 63, 6, 500, 1000, 0, mixed.duration_ms);
    assert(!commit_note_insertion(mixed, inside_phrase));
    assert(inside_phrase.active);
    assert(mixed.event_count == 3);

    Track rebuilt{};
    Track edited_phrase{};
    edited_phrase.occupied = true;
    edited_phrase.duration_ms = 1000;
    edited_phrase.event_count = 1;
    edited_phrase.events[0] = {200, 450, 60, 9, 0, 0};
    assert(rebuild_track_with_phrase_preserving_direct(mixed, 0, edited_phrase, 1, rebuilt));
    assert(rebuilt.phrase_count == 2);
    assert(rebuilt.event_count == 3);
    assert(rebuilt.events[0].start_ms == 200 && rebuilt.events[0].end_ms == 450);
    assert(find_direct_event_at(rebuilt, 1300) == 1);
    assert(rebuilt.events[1].midi_note == 61);
    assert(rebuilt.events[2].start_ms == 2100 && rebuilt.events[2].midi_note == 62);

    Track phrase_only = mixed;
    phrase_only.event_count = 2;
    phrase_only.events[0] = mixed.events[0];
    phrase_only.events[1] = mixed.events[2];
    PendingNoteInsertion between{};
    begin_note_insertion(between, 67, 5, 1300, 1000, 0);
    assert(commit_note_insertion(phrase_only, between));
    Track rebuilt_after_insert{};
    assert(rebuild_track_with_phrase_preserving_direct(
        phrase_only, 1, edited_phrase, 1, rebuilt_after_insert));
    assert(find_direct_event_at(rebuilt_after_insert, 1300) >= 0);
    assert(rebuilt_after_insert.phrase_count == 2);

    Track replaced = mixed;
    NoteEvent moved_direct{1600, 1800, 61, 2, 0, 0};
    assert(replace_note_event(replaced, 1, moved_direct));
    assert(find_direct_event_at(replaced, 1700) == 1);
    assert(!replace_note_event(replaced, 1, NoteEvent{200, 400, 61, 2, 0, 0}));

    Track rejected_inside_phrase = mixed;
    NoteEvent original = rejected_inside_phrase.events[1];
    assert(!replace_note_event(rejected_inside_phrase, 1, NoteEvent{2400, 2500, 61, 2, 0, 0}));
    assert(rejected_inside_phrase.event_count == mixed.event_count);
    assert(rejected_inside_phrase.events[1].start_ms == original.start_ms);
    assert(rejected_inside_phrase.events[1].end_ms == original.end_ms);
    assert(rejected_inside_phrase.events[1].midi_note == original.midi_note);

    PendingNoteInsertion overlapping{};
    begin_note_insertion(overlapping, 64, 2, 1200, 1000, 0);
    Track occupied{};
    occupied.event_count = 1;
    occupied.events[0] = {1000, 1500, 60, 1, 0, 0};
    assert(!commit_note_insertion(occupied, overlapping));
    assert(occupied.event_count == 1 && occupied.events[0].midi_note == 60);
    Track full{};
    full.event_count = MAX_TRACK_EVENTS;
    assert(!commit_note_insertion(full, overlapping));
    return 0;
}
