#pragma once

#include <stdint.h>

namespace loop_logic {

struct NoteEvent {
    uint32_t start_ms;
    uint32_t end_ms;
    int16_t midi_note;
    uint8_t instrument_id;
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

inline uint32_t snap_duration_to_master(uint32_t elapsed_ms, uint32_t master_ms) {
    if (master_ms == 0) return elapsed_ms;
    uint32_t multiples = (elapsed_ms + master_ms / 2u) / master_ms;
    if (multiples == 0) multiples = 1;
    return multiples * master_ms;
}

inline uint32_t next_cycle_start(uint32_t transport_ms, uint32_t origin_ms, uint32_t master_ms) {
    if (master_ms == 0) return transport_ms;
    uint32_t elapsed = transport_ms - origin_ms;
    uint32_t remainder = elapsed % master_ms;
    return transport_ms + (remainder == 0 ? 0 : master_ms - remainder);
}

inline uint32_t current_cycle_start(uint32_t transport_ms, uint32_t origin_ms, uint32_t master_ms) {
    if (master_ms == 0) return transport_ms;
    return transport_ms - ((transport_ms - origin_ms) % master_ms);
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

} // namespace loop_logic
