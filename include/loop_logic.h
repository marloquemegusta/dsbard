#pragma once

#include <stdint.h>

namespace loop_logic {

struct NoteEvent {
    uint32_t start_ms;
    uint32_t end_ms;
    int16_t midi_note;
    uint8_t instrument_id;
    uint8_t sustain_on_release;
    uint8_t sustain_level;
};

struct Phrase {
    uint32_t start_ms;
    uint32_t duration_ms;
    uint16_t event_count;
    uint16_t repeat_count;
};

static constexpr uint16_t MAX_TRACK_EVENTS = 96;
static constexpr uint8_t MAX_TRACK_PHRASES = 16;

struct Track {
    bool occupied;
    bool muted;
    uint32_t duration_ms;
    uint16_t event_count;
    uint8_t phrase_count;
    NoteEvent events[MAX_TRACK_EVENTS];
    Phrase phrases[MAX_TRACK_PHRASES];
};

struct TimelineSelection {
    uint32_t cursor_ms;
    uint32_t range_start_ms;
    uint32_t range_end_ms;
    bool explicit_range;
};

struct PulseGrid {
    uint32_t pulse_ms;

    uint32_t time(uint32_t step) const {
        return (uint32_t)(((uint64_t)step * pulse_ms) / 4u);
    }

    uint32_t nearest_step(uint32_t value_ms) const {
        if (pulse_ms == 0) return 0;
        return (uint32_t)(((uint64_t)value_ms * 4u + pulse_ms / 2u) / pulse_ms);
    }

    uint32_t snap(uint32_t value_ms) const {
        return time(nearest_step(value_ms));
    }
};

inline uint32_t snap_note_duration(uint32_t duration_ms, uint32_t pulse_ms) {
    if (pulse_ms == 0) return duration_ms;
    PulseGrid grid{pulse_ms};
    uint32_t quarter = grid.time(1);
    if (quarter == 0) return duration_ms;
    uint32_t snapped = grid.snap(duration_ms);
    return snapped < quarter ? quarter : snapped;
}

// Experimental round-1 composer state. It is deliberately separate from
// Phrase so the old capture/accept path remains available for comparison.
struct PendingNoteInsertion {
    bool active;
    NoteEvent event;
};

inline void begin_note_insertion(PendingNoteInsertion& pending, int16_t midi_note,
                                 uint8_t instrument_id, uint32_t requested_ms,
                                 uint32_t pulse_ms, uint32_t range_start_ms = 0) {
    PulseGrid grid{pulse_ms};
    uint32_t start = pulse_ms == 0 ? requested_ms : grid.snap(requested_ms);
    if (start < range_start_ms) start = range_start_ms;
    uint32_t duration = pulse_ms == 0 ? 1 : snap_note_duration(grid.time(1), pulse_ms);
    pending.event = {start, start + duration, midi_note, instrument_id, 0, 0};
    pending.active = true;
}

inline void cancel_note_insertion(PendingNoteInsertion& pending) {
    pending = {};
}

inline bool commit_note_insertion(Track& track, PendingNoteInsertion& pending) {
    if (!pending.active || track.event_count >= MAX_TRACK_EVENTS ||
        pending.event.end_ms <= pending.event.start_ms) return false;
    for (uint16_t i = 0; i < track.event_count; ++i) {
        const NoteEvent& current = track.events[i];
        if (pending.event.start_ms < current.end_ms && current.start_ms < pending.event.end_ms)
            return false;
    }
    uint16_t insert_at = 0;
    while (insert_at < track.event_count &&
           track.events[insert_at].start_ms < pending.event.start_ms) ++insert_at;
    for (uint16_t i = track.event_count; i > insert_at; --i)
        track.events[i] = track.events[i - 1];
    track.events[insert_at] = pending.event;
    ++track.event_count;
    if (track.duration_ms < pending.event.end_ms) track.duration_ms = pending.event.end_ms;
    track.occupied = true;
    pending = {};
    return true;
}

// Loop boundaries use complete metronome pulses. Note events may still use
// the finer four-step grid; only the cycle length must be phase-stable.
inline uint32_t snap_loop_boundary(uint32_t value_ms, uint32_t pulse_ms) {
    if (pulse_ms == 0) return value_ms;
    return (uint32_t)(((uint64_t)value_ms + pulse_ms / 2u) / pulse_ms * pulse_ms);
}

inline uint32_t snap_loop_duration(uint32_t duration_ms, uint32_t pulse_ms) {
    if (pulse_ms == 0) return duration_ms;
    uint32_t beats = (duration_ms + pulse_ms - 1u) / pulse_ms;
    if (beats == 0) beats = 1;
    return beats * pulse_ms;
}

inline void snap_note_events(Track& track, uint32_t pulse_ms,
                             uint32_t range_start_ms = 0) {
    if (pulse_ms == 0) return;
    PulseGrid grid{pulse_ms};
    uint32_t previous_end = range_start_ms;
    for (uint16_t i = 0; i < track.event_count; ++i) {
        NoteEvent& event = track.events[i];
        uint32_t raw_duration = event.end_ms > event.start_ms
            ? event.end_ms - event.start_ms : 1;
        uint32_t start = grid.snap(event.start_ms);
        uint32_t duration = snap_note_duration(raw_duration, pulse_ms);
        if (start < previous_end) start = previous_end;
        event.start_ms = start;
        event.end_ms = start + duration;
        previous_end = event.end_ms;
    }
}

inline uint32_t clamp_time(uint32_t value, uint32_t end_ms) {
    return value > end_ms ? end_ms : value;
}

inline void normalize_selection(TimelineSelection& selection, uint32_t song_duration_ms) {
    selection.range_start_ms = clamp_time(selection.range_start_ms, song_duration_ms);
    selection.range_end_ms = clamp_time(selection.range_end_ms, song_duration_ms);
    if (selection.range_end_ms < selection.range_start_ms) {
        uint32_t swap = selection.range_start_ms;
        selection.range_start_ms = selection.range_end_ms;
        selection.range_end_ms = swap;
    }
    if (selection.range_end_ms == selection.range_start_ms && song_duration_ms > 0) {
        selection.range_end_ms = song_duration_ms;
        selection.range_start_ms = 0;
    }
    selection.cursor_ms = clamp_time(selection.cursor_ms, song_duration_ms);
}

inline uint32_t seek_selection(TimelineSelection& selection, uint32_t cursor_ms,
                               uint32_t song_duration_ms) {
    selection.cursor_ms = clamp_time(cursor_ms, song_duration_ms);
    return selection.cursor_ms;
}

inline uint32_t restart_selection(const TimelineSelection& selection) {
    return selection.range_start_ms;
}

inline uint32_t advance_transport(uint32_t transport_ms, uint32_t delta_ms,
                                  uint32_t range_start_ms, uint32_t range_end_ms,
                                  bool loop_enabled, bool& stopped) {
    stopped = false;
    if (range_end_ms <= range_start_ms) {
        stopped = true;
        return range_start_ms;
    }
    if (transport_ms < range_start_ms) transport_ms = range_start_ms;
    if (transport_ms > range_end_ms) transport_ms = range_end_ms;
    uint64_t candidate = (uint64_t)transport_ms + delta_ms;
    uint32_t span = range_end_ms - range_start_ms;
    if (loop_enabled) {
        return range_start_ms + (uint32_t)((candidate - range_start_ms) % span);
    }
    if (candidate >= range_end_ms) {
        stopped = true;
        return range_end_ms;
    }
    return (uint32_t)candidate;
}

inline uint32_t expanded_phrase_start(const Phrase& phrase, uint16_t repetition) {
    return phrase.start_ms + phrase.duration_ms * (uint32_t)repetition;
}

inline uint32_t expanded_phrase_end(const Phrase& phrase, uint16_t repetition) {
    return expanded_phrase_start(phrase, repetition) + phrase.duration_ms;
}

inline uint32_t snap_timeline_time(uint32_t value_ms, uint32_t pulse_ms,
                                   const uint32_t* box_edges, uint16_t edge_count) {
    uint32_t best_edge = value_ms;
    uint32_t best_distance = UINT32_MAX;
    for (uint16_t i = 0; i < edge_count; ++i) {
        uint32_t edge = box_edges[i];
        uint32_t distance = edge > value_ms ? edge - value_ms : value_ms - edge;
        if (distance < best_distance) {
            best_edge = edge;
            best_distance = distance;
        }
    }
    if (edge_count > 0 && best_distance * 8u <= pulse_ms) return best_edge;
    return PulseGrid{pulse_ms}.snap(value_ms);
}

enum NoteEditMode : uint8_t {
    NOTE_MOVE = 0,
    NOTE_START = 1,
    NOTE_END = 2
};

inline bool edit_note_event(Track& track, uint16_t index, NoteEditMode mode,
                            uint32_t requested_ms, uint32_t range_start_ms,
                            uint32_t range_end_ms, uint32_t pulse_ms,
                            bool snap_to_grid = true) {
    if (index >= track.event_count || range_end_ms <= range_start_ms) return false;
    NoteEvent& event = track.events[index];
    PulseGrid grid{pulse_ms};
    uint32_t target = snap_to_grid ? grid.snap(requested_ms) : requested_ms;
    uint32_t previous_end = index == 0 ? range_start_ms : track.events[index - 1].end_ms;
    uint32_t next_start = index + 1 >= track.event_count ? range_end_ms : track.events[index + 1].start_ms;
    if (previous_end > next_start) return false;
    if (mode == NOTE_START) {
        if (target < previous_end) target = previous_end;
        if (target >= event.end_ms || target >= next_start) return false;
        event.start_ms = target;
        return true;
    }
    if (mode == NOTE_END) {
        if (target <= event.start_ms) return false;
        if (target > next_start) target = next_start;
        if (target <= event.start_ms) return false;
        event.end_ms = target;
        return true;
    }
    uint32_t duration = event.end_ms > event.start_ms ? event.end_ms - event.start_ms : 1;
    if (target < previous_end) target = previous_end;
    if (target + duration > next_start) {
        if (next_start < duration) return false;
        target = next_start - duration;
    }
    if (target < range_start_ms || target + duration > range_end_ms) return false;
    event.start_ms = target;
    event.end_ms = target + duration;
    return true;
}

inline uint32_t phase_ms(uint32_t elapsed_ms, uint32_t duration_ms) {
    return duration_ms == 0 ? 0 : elapsed_ms % duration_ms;
}

inline int key_for_x(uint16_t x) {
    return x >= 256 ? 11 : (int)(((uint32_t)x * 12u) / 256u);
}

inline int midi_for_key(int octave, int key) {
    return 12 * (octave + 1) + key;
}

inline int journal_value_for_key(int key) {
    return key;
}

inline uint16_t frequency_for_midi(int midi) {
    static const uint16_t c4[12] = {261, 277, 293, 311, 329, 349, 369, 392, 415, 440, 466, 493};
    int octave_delta = midi / 12 - 5;
    int key = midi % 12;
    uint32_t frequency = c4[key < 0 ? key + 12 : key];
    if (octave_delta > 0) for (int i = 0; i < octave_delta; ++i) frequency *= 2u;
    if (octave_delta < 0) for (int i = 0; i > octave_delta; --i) frequency /= 2u;
    if (frequency < 40u) frequency = 40u;
    if (frequency > 16000u) frequency = 16000u;
    return (uint16_t)frequency;
}

// libnds PSG frequency arguments use eight times the musical frequency.
inline uint16_t psg_frequency_for_midi(int midi) {
    uint32_t frequency = (uint32_t)frequency_for_midi(midi) * 8u;
    return (uint16_t)(frequency > 65535u ? 65535u : frequency);
}

inline int wrap_track(int track, int delta) {
    int value = (track + delta) % 3;
    return value < 0 ? value + 3 : value;
}

inline bool event_active(const NoteEvent& event, uint32_t phase) {
    return event.start_ms <= phase && phase < event.end_ms;
}

inline bool should_toggle_loop_on_release(bool pending, bool direction_used, bool released) {
    return pending && released && !direction_used;
}

} // namespace loop_logic
