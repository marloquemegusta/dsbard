#include <nds.h>
#include <calico/system/tick.h>
#include <stdio.h>
#include <string.h>
#include "loop_logic.h"

using loop_logic::NoteEvent;
using loop_logic::Track;

static constexpr int TRACKS = 3;
static constexpr int KEY_COUNT = 12;
static constexpr int MAX_VOICES = 4;
static constexpr uint32_t MIN_LOOP_MS = 120;
static const char* NOTE_NAMES[KEY_COUNT] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
static const DutyCycle TIMBRES[4] = {DutyCycle_50, DutyCycle_25, DutyCycle_12, DutyCycle_75};
static const char* TIMBRE_NAMES[4] = {"SQUARE", "PULSE", "THIN", "DARK"};

static Track tracks[TRACKS] = {};
static Track recording_track = {};
static Track undo_track = {};
static bool undo_available = false;
static int undo_slot = 0;
static int selected_track = 0;
static int octave = 4;
static int timbre = 0;
static int voice_ids[MAX_VOICES] = {-1, -1, -1, -1};
static int voice_notes[MAX_VOICES] = {-1, -1, -1, -1};
static int voice_timbres[MAX_VOICES] = {-1, -1, -1, -1};
static uint32_t transport_ms = 0;
static uint32_t previous_ms = 0;
static bool paused = false;
static bool recording = false;
static uint32_t record_start = 0;
static uint16_t record_count = 0;
static int recording_slot = 0;
static Track* recording_target = nullptr;
static int open_note = -1;
static int open_note_midi = -1;
static uint32_t open_note_start = 0;
static uint8_t open_note_timbre = 0;
static uint32_t b_hold_started = 0;
static bool b_hold_clear = false;
static int live_voice = -1;
static int live_note = -1;
static int live_target_note = -1;
static uint32_t live_transition_started = 0;
static bool live_glide = false;
static bool sustain_latched = false;
static bool vibrato_active = false;
static int bottom_bitmap_bg = -1;
static u16* bottom_vram = nullptr;
static u16 bottom_framebuffer[256 * 256] = {};
static u16* bottom_bitmap = bottom_framebuffer;
static int top_bitmap_bg = -1;
static u16* top_vram = nullptr;
static u16 top_framebuffer[256 * 256] = {};
static u16* top_bitmap = top_framebuffer;
static bool bottom_needs_redraw = true;
static int rendered_bottom_key = -2;
static int rendered_octave = -1;
static int rendered_track = -1;
static int rendered_timbre = -1;
static int touch_candidate_key = -1;
static int touch_candidate_frames = 0;
static int stable_touch_key = -1;
static int last_loop_note[TRACKS] = {-1, -1, -1};
static uint32_t last_phase[TRACKS] = {0, 0, 0};
static uint32_t flash_until = 0;

static uint32_t now_ms() {
    return (uint32_t)(((uint64_t)tickGetCount() * 1000u) / TICK_FREQ);
}

static uint16_t note_frequency(int midi) {
    return loop_logic::frequency_for_midi(midi);
}

static void kill_voice(int slot) {
    if (voice_ids[slot] >= 0) soundKill(voice_ids[slot]);
    voice_ids[slot] = -1;
    voice_notes[slot] = -1;
    voice_timbres[slot] = -1;
}

static void set_voice(int slot, int note, int voice_timbre) {
    if (note < 0) { kill_voice(slot); return; }
    if (voice_notes[slot] == note && voice_timbres[slot] == (voice_timbre & 3) && voice_ids[slot] >= 0) return;
    kill_voice(slot);
    voice_ids[slot] = soundPlayPSG(TIMBRES[voice_timbre & 3], note_frequency(note), 70, 64);
    voice_notes[slot] = note;
    voice_timbres[slot] = voice_timbre & 3;
}

static void stop_all_voices() {
    for (int i = 0; i < MAX_VOICES; ++i) kill_voice(i);
    kill_voice(3);
    live_voice = -1;
    live_note = -1;
    live_target_note = -1;
}

static void close_open_note(uint32_t at) {
    if (!recording || recording_target == nullptr || open_note < 0 || record_count >= 96) { open_note = -1; return; }
    NoteEvent& event = recording_target->events[record_count++];
    event.start_ms = open_note_start;
    event.end_ms = at > open_note_start ? at : open_note_start + 1;
    event.midi_note = open_note_midi;
    event.timbre = open_note_timbre;
    open_note = -1;
}

static void begin_note(int key, uint32_t at, bool glide) {
    int midi = loop_logic::midi_for_key(octave, key);
    if (live_note != midi) {
        if (glide && live_voice >= 0 && live_note >= 0 && !(live_glide && live_target_note == midi)) {
            live_target_note = midi;
            live_transition_started = at;
            live_glide = true;
        } else {
            if (live_voice >= 0) soundKill(live_voice);
            live_voice = soundPlayPSG(TIMBRES[timbre], note_frequency(midi), 96, 64);
            live_note = midi;
            live_target_note = midi;
            live_glide = false;
        }
    }
    if (recording && open_note != key) {
        close_open_note(at - record_start);
        open_note = key;
        open_note_midi = midi;
        open_note_start = at - record_start;
        open_note_timbre = (uint8_t)timbre;
    }
}

static void end_live_note(uint32_t at) {
    if (live_voice >= 0) soundKill(live_voice);
    live_voice = -1;
    live_note = -1;
    live_target_note = -1;
    live_glide = false;
    vibrato_active = false;
    if (recording) close_open_note(at - record_start);
}

static void refresh_live_timbre() {
    if (live_voice < 0 || live_note < 0) return;
    soundKill(live_voice);
    live_voice = soundPlayPSG(TIMBRES[timbre], note_frequency(live_note), 96, 64);
}

static void clear_selected() {
    tracks[selected_track] = {};
    if (recording && recording_slot == selected_track) { recording = false; open_note = -1; recording_target = nullptr; }
    for (int i = 0; i < TRACKS; ++i) last_loop_note[i] = -1;
}

static void start_recording(uint32_t now) {
    recording_slot = selected_track;
    if (tracks[selected_track].occupied) {
        undo_track = tracks[selected_track];
        undo_available = true;
        undo_slot = selected_track;
        recording_track = {};
        recording_target = &recording_track;
        tracks[selected_track].muted = true;
    } else {
        undo_available = false;
        tracks[selected_track].event_count = 0;
        tracks[selected_track].occupied = false;
        recording_target = &tracks[selected_track];
    }
    recording = true;
    record_start = now;
    record_count = 0;
    open_note = -1;
    open_note_midi = -1;
}

static void stop_recording(uint32_t now) {
    close_open_note(now - record_start);
    uint32_t duration = now - record_start;
    recording = false;
    Track* target = recording_target;
    recording_target = nullptr;
    if (duration >= MIN_LOOP_MS && record_count > 0) {
        target->duration_ms = duration;
        target->origin_ms = transport_ms;
        target->event_count = record_count;
        target->occupied = true;
        target->muted = false;
        if (target == &recording_track) tracks[recording_slot] = recording_track;
    } else {
        if (target == &tracks[recording_slot]) {
            tracks[recording_slot].event_count = 0;
            tracks[recording_slot].occupied = false;
        } else {
            tracks[recording_slot].muted = false;
        }
    }
}

static void undo_recording() {
    if (!undo_available || recording) return;
    tracks[undo_slot] = undo_track;
    undo_available = false;
}

static void update_live_effects(uint32_t now, int held) {
    if (live_voice < 0 || live_note < 0) return;
    if (live_glide && live_target_note >= 0) {
        uint32_t elapsed = now - live_transition_started;
        if (elapsed >= 100) {
            live_note = live_target_note;
            live_glide = false;
        } else {
            int from = note_frequency(live_note);
            int to = note_frequency(live_target_note);
            int interpolated = from + ((to - from) * (int)elapsed) / 100;
            soundSetFreq(live_voice, (u16)interpolated);
            return;
        }
    }
    if (held & KEY_L) {
        int frequency = note_frequency(live_note);
        int vibrato = ((now / 30) & 1) ? 14 : -14;
        soundSetFreq(live_voice, (u16)(frequency + vibrato));
        vibrato_active = true;
    } else if (vibrato_active) {
        soundSetFreq(live_voice, note_frequency(live_note));
        vibrato_active = false;
    }
}

static int active_track_event(const Track& track, uint32_t phase) {
    for (uint16_t i = 0; i < track.event_count; ++i)
        if (loop_logic::event_active(track.events[i], phase)) return i;
    return -1;
}

static bool bucket_has_activity(const Track& track, int bucket) {
    if (!track.occupied || track.duration_ms == 0) return false;
    uint32_t begin = (track.duration_ms * (uint32_t)bucket) / 20u;
    uint32_t end = (track.duration_ms * (uint32_t)(bucket + 1)) / 20u;
    for (uint16_t i = 0; i < track.event_count; ++i) {
        if (track.events[i].start_ms < end && track.events[i].end_ms > begin) return true;
    }
    return false;
}

static void draw_text_bitmap(u16* target, int x, int y, const char* text, u16 color, int scale = 2);
static void fill_rect(u16* target, int x0, int y0, int x1, int y1, u16 color);

static void update_loops() {
    for (int i = 0; i < TRACKS; ++i) {
        int event_index = -1;
        if (tracks[i].occupied && !paused) {
            uint32_t phase = loop_logic::phase_ms(transport_ms - tracks[i].origin_ms, tracks[i].duration_ms);
            event_index = active_track_event(tracks[i], phase);
        }
        int note = event_index >= 0 ? tracks[i].events[event_index].midi_note : -1;
        int event_timbre = event_index >= 0 ? tracks[i].events[event_index].timbre : 0;
        if (tracks[i].occupied && tracks[i].duration_ms > 0) {
            uint32_t phase = loop_logic::phase_ms(transport_ms - tracks[i].origin_ms, tracks[i].duration_ms);
            if (!paused && phase < last_phase[i]) flash_until = now_ms() + 80;
            last_phase[i] = phase;
        }
        set_voice(i, tracks[i].muted ? -1 : note, event_timbre);
        last_loop_note[i] = note;
    }
}

static void draw_top() {
    const u16 opaque = BIT(15);
    const u16 background = opaque | RGB15(1, 1, 2);
    const u16 panel = opaque | RGB15(8, 12, 18);
    const u16 panel_alt = opaque | RGB15(11, 16, 23);
    const u16 line = opaque | RGB15(10, 16, 24);
    const u16 text = opaque | RGB15(31, 31, 31);
    const u16 accent = opaque | RGB15(20, 25, 31);
    const u16 green = opaque | RGB15(20, 31, 22);
    for (int y = 0; y < 192; ++y)
        for (int x = 0; x < 256; ++x)
            top_bitmap[y * 256 + x] = background;

    fill_rect(top_bitmap, 4, 4, 251, 25, panel);
    for (int x = 4; x <= 251; ++x) {
        top_bitmap[4 * 256 + x] = line;
        top_bitmap[25 * 256 + x] = line;
    }
    draw_text_bitmap(top_bitmap, 9, 9, "STYLOPHONE", text, 1);
    char header_hz[12];
    if (live_note >= 0) snprintf(header_hz, sizeof(header_hz), "HZ %d", note_frequency(live_note));
    else snprintf(header_hz, sizeof(header_hz), "HZ --");
    draw_text_bitmap(top_bitmap, 146, 9, header_hz, live_note >= 0 ? green : text, 1);
    draw_text_bitmap(top_bitmap, 178, 9, paused ? "PAUSED" : (recording ? "REC" : "RUN"), recording ? green : text, 1);

    if (recording && recording_target != nullptr) {
        recording_target->occupied = true;
        recording_target->duration_ms = (uint32_t)(now_ms() - record_start);
        if (recording_target->duration_ms == 0) recording_target->duration_ms = 1;
        recording_target->origin_ms = 0;
    }

    for (int i = 0; i < TRACKS; ++i) {
        int y = 31 + i * 47;
        bool selected = selected_track == i;
        bool is_recording = recording && recording_slot == i;
        const Track& track = tracks[i];
        const Track& visual_track = is_recording && recording_target != nullptr ? *recording_target : track;
        const char* state = is_recording ? "REC" : (track.occupied ? (track.muted ? "MUTE" : "PLAY") : "EMPTY");
        u16 card = selected ? panel_alt : panel;
        fill_rect(top_bitmap, 4, y, 251, y + 39, card);
        for (int x = 4; x <= 251; ++x) top_bitmap[y * 256 + x] = selected ? accent : line;
        draw_text_bitmap(top_bitmap, 9, y + 6, "TRACK", text, 1);
        char track_number[2] = {(char)('1' + i), '\0'};
        draw_text_bitmap(top_bitmap, 38, y + 6, track_number, text, 1);
        draw_text_bitmap(top_bitmap, 52, y + 6, state, is_recording ? green : text, 1);
        for (int bucket = 0; bucket < 20; ++bucket) {
            int x = 91 + bucket * 8;
            bool active = bucket_has_activity(visual_track, bucket);
            u16 bar = active ? (is_recording || track.muted ? green : accent) : line;
            fill_rect(top_bitmap, x, y + 24, x + 5, y + (active ? 31 : 27), bar);
        }
        if ((track.occupied || is_recording) && visual_track.duration_ms > 0) {
            uint32_t phase = is_recording
                ? loop_logic::phase_ms(now_ms() - record_start, visual_track.duration_ms)
                : loop_logic::phase_ms(transport_ms - visual_track.origin_ms, visual_track.duration_ms);
            int playhead = 91 + (int)((phase * 160u) / visual_track.duration_ms);
            fill_rect(top_bitmap, playhead, y + 21, playhead + 1, y + 34, green);
        }
    }

    fill_rect(top_bitmap, 4, 177, 251, 188, panel);
    draw_text_bitmap(top_bitmap, 9, 180, "OCT", text, 1);
    char octave_text[2] = {(char)('0' + octave), '\0'};
    draw_text_bitmap(top_bitmap, 25, 180, octave_text, text, 1);
    draw_text_bitmap(top_bitmap, 39, 180, TIMBRE_NAMES[timbre], text, 1);
    draw_text_bitmap(top_bitmap, 82, 180, sustain_latched ? "SUSON" : "SUSOFF", sustain_latched ? green : text, 1);
    char live_text[28];
    if (live_note >= 0) {
        int live_key = live_note % 12;
        snprintf(live_text, sizeof(live_text), "%s%d M%d %dHZ%s", NOTE_NAMES[live_key], live_note / 12 - 1,
                 live_note, note_frequency(live_note), vibrato_active ? " V" : "");
    } else {
        snprintf(live_text, sizeof(live_text), "LIVE --");
    }
    draw_text_bitmap(top_bitmap, 133, 180, live_text, text, 1);
    dmaCopy(top_framebuffer, top_vram, 256 * 192 * sizeof(u16));
}

static const uint8_t* glyph(char c) {
    static const uint8_t blank[5] = {0, 0, 0, 0, 0};
    static const uint8_t glyphs[][6] = {
        {'A', 7, 5, 7, 5, 5}, {'B', 6, 5, 6, 5, 6}, {'C', 7, 4, 4, 4, 7},
        {'D', 6, 5, 5, 5, 6}, {'E', 7, 4, 6, 4, 7}, {'F', 7, 4, 6, 4, 4},
        {'G', 7, 4, 5, 5, 7}, {'H', 5, 5, 7, 5, 5}, {'I', 7, 2, 2, 2, 7},
        {'K', 5, 5, 6, 5, 5}, {'L', 4, 4, 4, 4, 7}, {'M', 5, 7, 7, 5, 5}, {'N', 5, 7, 7, 7, 5},
        {'O', 7, 5, 5, 5, 7}, {'P', 6, 5, 6, 4, 4}, {'Q', 7, 5, 7, 2, 1},
        {'R', 6, 5, 6, 5, 5}, {'S', 7, 4, 7, 1, 7}, {'T', 7, 2, 2, 2, 2},
        {'U', 5, 5, 5, 5, 7}, {'V', 5, 5, 5, 5, 2}, {'0', 7, 5, 5, 5, 7},
        {'1', 2, 6, 2, 2, 7}, {'2', 6, 1, 2, 4, 7}, {'3', 6, 1, 6, 1, 6},
        {'4', 5, 5, 7, 1, 1}, {'5', 7, 4, 6, 1, 6}, {'6', 3, 4, 7, 5, 7},
        {'7', 7, 1, 2, 2, 2}, {'8', 7, 5, 7, 5, 7}, {'9', 7, 5, 7, 1, 6},
        {'Y', 5, 5, 2, 2, 2},
        {'Z', 7, 1, 2, 4, 7}, {'-', 0, 0, 7, 0, 0}, {'#', 5, 7, 5, 7, 5}
    };
    for (unsigned i = 0; i < sizeof(glyphs) / sizeof(glyphs[0]); ++i)
        if (glyphs[i][0] == (uint8_t)c) return &glyphs[i][1];
    return blank;
}

static void draw_glyph_bitmap(u16* target, int x, int y, const uint8_t* rows, u16 color, int scale) {
    for (int row = 0; row < 5; ++row)
        for (int col = 0; col < 3; ++col)
            if (rows[row] & (1 << (2 - col)))
                for (int dy = 0; dy < scale; ++dy)
                    for (int dx = 0; dx < scale; ++dx)
                        target[(y + row * scale + dy) * 256 + x + col * scale + dx] = color;
}

static void draw_text_bitmap(u16* target, int x, int y, const char* text, u16 color, int scale) {
    for (; *text; ++text) {
        draw_glyph_bitmap(target, x, y, glyph(*text), color, scale);
        x += 4 * scale;
    }
}

static void draw_note_label(int index, int x, int y, u16 color) {
    static const uint8_t note_rows[KEY_COUNT][2][5] = {
        {{7, 4, 4, 4, 7}, {0, 0, 0, 0, 0}},
        {{7, 4, 4, 4, 7}, {5, 7, 5, 7, 5}},
        {{6, 5, 5, 5, 6}, {0, 0, 0, 0, 0}},
        {{6, 5, 5, 5, 6}, {5, 7, 5, 7, 5}},
        {{7, 4, 6, 4, 7}, {0, 0, 0, 0, 0}},
        {{7, 4, 6, 4, 4}, {0, 0, 0, 0, 0}},
        {{7, 4, 6, 4, 4}, {5, 7, 5, 7, 5}},
        {{7, 4, 5, 5, 7}, {0, 0, 0, 0, 0}},
        {{7, 4, 5, 5, 7}, {5, 7, 5, 7, 5}},
        {{7, 5, 7, 5, 5}, {0, 0, 0, 0, 0}},
        {{7, 5, 7, 5, 5}, {5, 7, 5, 7, 5}},
        {{6, 5, 5, 5, 6}, {0, 0, 0, 0, 0}}
    };
    draw_glyph_bitmap(bottom_bitmap, x, y, note_rows[index][0], color, 2);
    if (NOTE_NAMES[index][1] == '#') draw_glyph_bitmap(bottom_bitmap, x + 8, y, note_rows[index][1], color, 2);
}

static void present_bottom_full() {
    dmaCopy(bottom_framebuffer, bottom_vram, 256 * 192 * sizeof(u16));
}

static void present_bottom_band(int index) {
    if (index < 0 || index >= KEY_COUNT) return;
    int left = (index * 256) / KEY_COUNT;
    int right = ((index + 1) * 256) / KEY_COUNT;
    for (int y = 36; y < 192; ++y)
        dmaCopy(&bottom_framebuffer[y * 256 + left], &bottom_vram[y * 256 + left], (right - left) * sizeof(u16));
}

static void fill_rect(u16* target, int x0, int y0, int x1, int y1, u16 color) {
    for (int y = y0; y <= y1; ++y)
        for (int x = x0; x <= x1; ++x)
            target[y * 256 + x] = color;
}

static void paint_bottom_band(int index, bool active) {
    if (index < 0 || index >= KEY_COUNT) return;
    const u16 opaque = BIT(15);
    const u16 natural = opaque | RGB15(5, 8, 14);
    const u16 sharp = opaque | RGB15(3, 4, 8);
    const u16 separator = opaque | RGB15(14, 18, 24);
    const u16 active_color = opaque | RGB15(20, 25, 31);
    int left = (index * 256) / KEY_COUNT;
    int right = ((index + 1) * 256) / KEY_COUNT;
    u16 fill = active ? active_color : ((index == 1 || index == 3 || index == 6 || index == 8 || index == 10) ? sharp : natural);
    for (int y = 38; y < 192; ++y)
        for (int x = left + 1; x < right; ++x)
            bottom_bitmap[y * 256 + x] = fill;
    for (int y = 36; y < 192; ++y) bottom_bitmap[y * 256 + left] = separator;
    if (active) {
        for (int x = left + 1; x < right; ++x) {
            bottom_bitmap[36 * 256 + x] = separator;
            bottom_bitmap[37 * 256 + x] = separator;
        }
    }
}

static void draw_bottom_full(int key) {
    // 16-bit bitmap backgrounds use bit 15 as the opaque flag.
    const u16 opaque = BIT(15);
    const u16 background = opaque | RGB15(1, 1, 2);
    const u16 separator = opaque | RGB15(14, 18, 24);
    const u16 label = opaque | RGB15(31, 31, 31);
    const u16 label_panel = opaque | RGB15(11, 16, 23);
    for (int y = 0; y < 192; ++y)
        for (int x = 0; x < 256; ++x)
            bottom_bitmap[y * 256 + x] = background;

    const u16 panel = opaque | RGB15(8, 12, 18);
    const u16 line = opaque | RGB15(10, 16, 24);
    fill_rect(bottom_bitmap, 3, 2, 52, 19, panel);
    fill_rect(bottom_bitmap, 56, 2, 133, 19, panel);
    fill_rect(bottom_bitmap, 137, 2, 252, 19, panel);
    for (int x = 3; x <= 52; ++x) { bottom_bitmap[2 * 256 + x] = line; bottom_bitmap[19 * 256 + x] = line; }
    for (int x = 56; x <= 133; ++x) { bottom_bitmap[2 * 256 + x] = line; bottom_bitmap[19 * 256 + x] = line; }
    for (int x = 137; x <= 252; ++x) { bottom_bitmap[2 * 256 + x] = line; bottom_bitmap[19 * 256 + x] = line; }
    draw_text_bitmap(bottom_bitmap, 6, 5, "OCT", label);
    const char* octave_label = octave == 2 ? "2" : (octave == 3 ? "3" : (octave == 4 ? "4" : (octave == 5 ? "5" : "6")));
    draw_text_bitmap(bottom_bitmap, 34, 5, octave_label, label);
    draw_text_bitmap(bottom_bitmap, 58, 5, "TRACK", label);
    draw_text_bitmap(bottom_bitmap, 112, 5, selected_track == 0 ? "1" : (selected_track == 1 ? "2" : "3"), label);
    draw_text_bitmap(bottom_bitmap, 140, 5, TIMBRE_NAMES[timbre], label, 1);

    for (int i = 0; i < KEY_COUNT; ++i) paint_bottom_band(i, i == key);
    for (int i = 0; i < KEY_COUNT; ++i) {
        int left = (i * 256) / KEY_COUNT;
        int right = ((i + 1) * 256) / KEY_COUNT;
        int label_width = (NOTE_NAMES[i][1] == '\0' ? 6 : 14);
        int label_x = left + ((right - left) - label_width) / 2;
        fill_rect(bottom_bitmap, left + 2, 21, right - 3, 35, label_panel);
        draw_note_label(i, label_x, 24, label);
        int marker_x = left + (right - left) / 2;
        for (int y = 20; y <= 22; ++y)
            for (int x = marker_x - 1; x <= marker_x + 1; ++x)
                bottom_bitmap[y * 256 + x] = separator;
    }
    bottom_bitmap[191 * 256 + 255] = separator;
    rendered_bottom_key = key;
    rendered_octave = octave;
    rendered_track = selected_track;
    rendered_timbre = timbre;
    bottom_needs_redraw = false;
    present_bottom_full();
}

static void draw_bottom(int key) {
    if (bottom_needs_redraw || rendered_octave != octave || rendered_track != selected_track || rendered_timbre != timbre) {
        draw_bottom_full(key);
        return;
    }
    if (key == rendered_bottom_key) return;
    int previous_key = rendered_bottom_key;
    paint_bottom_band(previous_key, false);
    paint_bottom_band(key, true);
    rendered_bottom_key = key;
    present_bottom_band(previous_key);
    present_bottom_band(key);
}

int main() {
    videoSetMode(MODE_5_2D | DISPLAY_BG3_ACTIVE);
    videoSetModeSub(MODE_5_2D | DISPLAY_BG3_ACTIVE);
    vramSetBankA(VRAM_A_MAIN_BG);
    vramSetBankC(VRAM_C_SUB_BG);
    top_bitmap_bg = bgInit(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
    bgSetPriority(top_bitmap_bg, 0);
    bottom_bitmap_bg = bgInitSub(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
    bgSetPriority(bottom_bitmap_bg, 0);
    REG_DISPCNT_SUB |= DISPLAY_BG3_ACTIVE;
    top_vram = bgGetGfxPtr(top_bitmap_bg);
    bottom_vram = bgGetGfxPtr(bottom_bitmap_bg);
    soundEnable();
    touchPosition touch = {};
    previous_ms = now_ms();
    while (pmMainLoop()) {
        swiWaitForVBlank();
        scanKeys();
        uint32_t now = now_ms();
        uint32_t delta = now - previous_ms;
        previous_ms = now;
        if (!paused) transport_ms += delta;

        int down = keysDown();
        int held = keysHeld();
        if (down & KEY_UP) if (octave < 6) ++octave;
        if (down & KEY_DOWN) if (octave > 2) --octave;
        if (down & KEY_LEFT) selected_track = loop_logic::wrap_track(selected_track, -1);
        if (down & KEY_RIGHT) selected_track = loop_logic::wrap_track(selected_track, 1);
        if (down & KEY_START) { paused = !paused; if (paused) update_loops(); }
        if (down & KEY_SELECT) tracks[selected_track].muted = !tracks[selected_track].muted;
        if (down & KEY_X) { timbre = (timbre + 1) & 3; refresh_live_timbre(); }
        if (down & KEY_Y) sustain_latched = !sustain_latched;
        if (down & KEY_A) { if (recording) stop_recording(now); else start_recording(now); }
        if (down & KEY_B) { b_hold_started = now; b_hold_clear = true; }
        if (!(held & KEY_B) && b_hold_clear) { if (now - b_hold_started < 850) undo_recording(); b_hold_clear = false; }
        if ((held & KEY_B) && now - b_hold_started >= 850) { clear_selected(); b_hold_clear = false; }

        int key = -1;
        touchRead(&touch);
        if ((held & KEY_TOUCH) && touch.py < 192) {
            int sampled_key = loop_logic::key_for_x(touch.px);
            if (sampled_key == touch_candidate_key) {
                if (touch_candidate_frames < 2) ++touch_candidate_frames;
            } else {
                touch_candidate_key = sampled_key;
                touch_candidate_frames = 1;
            }
            if (touch_candidate_frames >= 2) stable_touch_key = touch_candidate_key;
            key = stable_touch_key;
        } else {
            touch_candidate_key = -1;
            touch_candidate_frames = 0;
            stable_touch_key = -1;
        }
        if (key >= 0) begin_note(key, now, (held & KEY_R) != 0);
        else if (!(held & KEY_TOUCH) && !sustain_latched && !(held & KEY_Y)) end_live_note(now);
        update_live_effects(now, held);
        update_loops();
        draw_top();
        draw_bottom(key);
    }
    stop_all_voices();
    return 0;
}
