#pragma once

#include "loop_logic.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace song_storage {

static constexpr uint8_t FORMAT_VERSION = 1;
static constexpr uint8_t SONG_TRACKS = 4;
static constexpr uint8_t LABEL_WIDTH = 192;
static constexpr uint8_t LABEL_HEIGHT = 24;
static constexpr std::size_t LABEL_BYTES = (LABEL_WIDTH * LABEL_HEIGHT) / 8;

struct HandwrittenLabel {
    uint8_t width = 0;
    uint8_t height = 0;
    std::array<uint8_t, LABEL_BYTES> pixels{};
};

struct SongDocument {
    uint32_t pulse_ms = 500;
    HandwrittenLabel label{};
    std::array<loop_logic::Track, SONG_TRACKS> tracks{};
};

inline void put_u16(std::vector<uint8_t>& out, uint16_t value) {
    out.push_back((uint8_t)(value & 0xff));
    out.push_back((uint8_t)(value >> 8));
}

inline void put_u32(std::vector<uint8_t>& out, uint32_t value) {
    for (int shift = 0; shift < 32; shift += 8) out.push_back((uint8_t)(value >> shift));
}

inline bool get_u16(const uint8_t*& cursor, const uint8_t* end, uint16_t& value) {
    if ((std::size_t)(end - cursor) < 2) return false;
    value = (uint16_t)cursor[0] | ((uint16_t)cursor[1] << 8);
    cursor += 2;
    return true;
}

inline bool get_u32(const uint8_t*& cursor, const uint8_t* end, uint32_t& value) {
    if ((std::size_t)(end - cursor) < 4) return false;
    value = (uint32_t)cursor[0] | ((uint32_t)cursor[1] << 8) |
            ((uint32_t)cursor[2] << 16) | ((uint32_t)cursor[3] << 24);
    cursor += 4;
    return true;
}

inline uint32_t checksum(const uint8_t* data, std::size_t size) {
    uint32_t result = 2166136261u;
    for (std::size_t i = 0; i < size; ++i) {
        result ^= data[i];
        result *= 16777619u;
    }
    return result;
}

inline std::vector<uint8_t> serialize(const SongDocument& song) {
    std::vector<uint8_t> out;
    out.reserve(7000);
    out.insert(out.end(), {'D', 'S', 'S', 'G'});
    out.push_back(FORMAT_VERSION);
    put_u32(out, song.pulse_ms);
    out.push_back(song.label.width);
    out.push_back(song.label.height);
    out.insert(out.end(), song.label.pixels.begin(), song.label.pixels.end());
    for (const loop_logic::Track& track : song.tracks) {
        out.push_back(track.occupied ? 1 : 0);
        out.push_back(track.muted ? 1 : 0);
        put_u32(out, track.duration_ms);
        put_u16(out, track.event_count);
        out.push_back(track.phrase_count);
        out.push_back(0);
        for (uint16_t i = 0; i < loop_logic::MAX_TRACK_EVENTS; ++i) {
            const loop_logic::NoteEvent& event = track.events[i];
            put_u32(out, event.start_ms);
            put_u32(out, event.end_ms);
            put_u16(out, (uint16_t)event.midi_note);
            out.push_back(event.instrument_id);
            out.push_back(event.sustain_on_release);
            out.push_back(event.sustain_level);
            out.push_back(0);
        }
        for (uint8_t i = 0; i < loop_logic::MAX_TRACK_PHRASES; ++i) {
            const loop_logic::Phrase& phrase = track.phrases[i];
            put_u32(out, phrase.start_ms);
            put_u32(out, phrase.duration_ms);
            put_u16(out, phrase.event_count);
            put_u16(out, phrase.repeat_count);
        }
    }
    put_u32(out, checksum(out.data(), out.size()));
    return out;
}

inline bool deserialize(const uint8_t* data, std::size_t size, SongDocument& song) {
    if (!data || size < 9 || size < 4) return false;
    if (data[0] != 'D' || data[1] != 'S' || data[2] != 'S' || data[3] != 'G' ||
        data[4] != FORMAT_VERSION || checksum(data, size - 4) !=
        ((uint32_t)data[size - 4] | ((uint32_t)data[size - 3] << 8) |
         ((uint32_t)data[size - 2] << 16) | ((uint32_t)data[size - 1] << 24))) return false;
    const uint8_t* cursor = data + 5;
    const uint8_t* end = data + size - 4;
    SongDocument decoded{};
    if (!get_u32(cursor, end, decoded.pulse_ms) || cursor + 2 > end) return false;
    decoded.label.width = *cursor++;
    decoded.label.height = *cursor++;
    if (decoded.label.width > LABEL_WIDTH || decoded.label.height > LABEL_HEIGHT ||
        cursor + LABEL_BYTES > end) return false;
    for (std::size_t i = 0; i < LABEL_BYTES; ++i) decoded.label.pixels[i] = *cursor++;
    for (loop_logic::Track& track : decoded.tracks) {
        if (cursor + 2 > end) return false;
        track.occupied = *cursor++ != 0;
        track.muted = *cursor++ != 0;
        if (!get_u32(cursor, end, track.duration_ms) || !get_u16(cursor, end, track.event_count) ||
            cursor + 2 > end) return false;
        track.phrase_count = *cursor++;
        ++cursor;
        if (track.event_count > loop_logic::MAX_TRACK_EVENTS ||
            track.phrase_count > loop_logic::MAX_TRACK_PHRASES) return false;
        for (uint16_t i = 0; i < loop_logic::MAX_TRACK_EVENTS; ++i) {
            loop_logic::NoteEvent& event = track.events[i];
            uint16_t midi = 0;
            if (!get_u32(cursor, end, event.start_ms) || !get_u32(cursor, end, event.end_ms) ||
                !get_u16(cursor, end, midi) || cursor + 4 > end || midi > 127) return false;
            event.midi_note = (int16_t)midi;
            event.instrument_id = *cursor++;
            event.sustain_on_release = *cursor++;
            event.sustain_level = *cursor++;
            ++cursor;
        }
        for (uint8_t i = 0; i < loop_logic::MAX_TRACK_PHRASES; ++i) {
            loop_logic::Phrase& phrase = track.phrases[i];
            if (!get_u32(cursor, end, phrase.start_ms) || !get_u32(cursor, end, phrase.duration_ms) ||
                !get_u16(cursor, end, phrase.event_count) || !get_u16(cursor, end, phrase.repeat_count)) return false;
            if (phrase.event_count > loop_logic::MAX_TRACK_EVENTS) return false;
        }
    }
    if (cursor != end) return false;
    song = decoded;
    return true;
}

inline bool ensure_default_label(SongDocument& song) {
    if (song.label.width != 0 && song.label.height != 0) return false;
    song.label = {};
    song.label.width = LABEL_WIDTH;
    song.label.height = LABEL_HEIGHT;
    return true;
}

inline void draw_label_segment(HandwrittenLabel& label, int x0, int y0, int x1, int y1) {
    if (label.width == 0 || label.height == 0) return;
    int dx = x1 >= x0 ? x1 - x0 : x0 - x1;
    int sx = x0 < x1 ? 1 : -1;
    int dy = y1 >= y0 ? y1 - y0 : y0 - y1;
    int sy = y0 < y1 ? 1 : -1;
    int error = dx - dy;
    while (true) {
        for (int brush_y = y0 - 1; brush_y <= y0 + 1; ++brush_y)
            for (int brush_x = x0 - 1; brush_x <= x0 + 1; ++brush_x)
                if (brush_x >= 0 && brush_x < label.width && brush_x < LABEL_WIDTH &&
                    brush_y >= 0 && brush_y < label.height && brush_y < LABEL_HEIGHT) {
                    std::size_t bit = (std::size_t)brush_y * LABEL_WIDTH + (std::size_t)brush_x;
                    label.pixels[bit / 8] |= (uint8_t)(1u << (bit % 8));
                }
        if (x0 == x1 && y0 == y1) break;
        int twice = error * 2;
        if (twice > -dy) { error -= dy; x0 += sx; }
        if (twice < dx) { error += dx; y0 += sy; }
    }
}

} // namespace song_storage
