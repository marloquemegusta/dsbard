#pragma once

#include <stdint.h>

namespace loop_logic {

struct NoteEvent {
    uint32_t start_ms;
    uint32_t end_ms;
    int16_t midi_note;
    uint8_t timbre;
};

struct Track {
    bool occupied;
    bool muted;
    uint32_t duration_ms;
    uint32_t origin_ms;
    uint16_t event_count;
    NoteEvent events[96];
};

inline uint32_t phase_ms(uint32_t elapsed_ms, uint32_t duration_ms) {
    return duration_ms == 0 ? 0 : elapsed_ms % duration_ms;
}

inline int key_for_x(uint16_t x) {
    return x >= 256 ? 11 : (x * 12) / 256;
}

inline int midi_for_key(int octave, int key) {
    return 12 * (octave + 1) + key;
}

inline int wrap_track(int track, int delta) {
    int value = (track + delta) % 3;
    return value < 0 ? value + 3 : value;
}

inline bool event_active(const NoteEvent& event, uint32_t phase) {
    return event.start_ms <= phase && phase < event.end_ms;
}

} // namespace loop_logic
