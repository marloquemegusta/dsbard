#include <nds.h>
#include <calico/system/tick.h>
#include <stdio.h>
#include <math.h>
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
static PrintConsole top_console;
static PrintConsole bottom_console;
static int last_loop_note[TRACKS] = {-1, -1, -1};

static uint32_t now_ms() {
    return (uint32_t)(((uint64_t)tickGetCount() * 1000u) / TICK_FREQ);
}

static uint16_t note_frequency(int midi) {
    double frequency = 440.0 * pow(2.0, (midi - 69) / 12.0);
    if (frequency < 40.0) frequency = 40.0;
    if (frequency > 16000.0) frequency = 16000.0;
    return (uint16_t)frequency;
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
    int midi = 12 * (octave + 1) + key;
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
        close_open_note(at);
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
    if (recording) close_open_note(at - record_start);
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
    } else {
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
        target->event_count = record_count;
        target->occupied = true;
        target->muted = false;
        if (target == &recording_track) tracks[recording_slot] = recording_track;
    } else {
        if (target == &tracks[recording_slot]) {
            tracks[recording_slot].event_count = 0;
            tracks[recording_slot].occupied = false;
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
    int frequency = note_frequency(live_note);
    if (held & KEY_L) {
        int vibrato = ((now / 35) & 1) ? 3 : -3;
        frequency += vibrato;
    }
    soundSetFreq(live_voice, (u16)frequency);
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

static void update_loops() {
    for (int i = 0; i < TRACKS; ++i) {
        int event_index = -1;
        if (tracks[i].occupied && !paused) {
            uint32_t phase = loop_logic::phase_ms(transport_ms, tracks[i].duration_ms);
            event_index = active_track_event(tracks[i], phase);
        }
        int note = event_index >= 0 ? tracks[i].events[event_index].midi_note : -1;
        int event_timbre = event_index >= 0 ? tracks[i].events[event_index].timbre : 0;
        set_voice(i, tracks[i].muted ? -1 : note, event_timbre);
        last_loop_note[i] = note;
    }
}

static void draw_top() {
    consoleSelect(&top_console);
    iprintf("\x1b[1;0HSTYLOPHONE / LOOPER       ");
    const char* state0 = recording && recording_slot == 0 ? "REC" : (tracks[0].occupied ? (tracks[0].muted ? "MUTE" : "PLAY") : "EMPTY");
    const char* state1 = recording && recording_slot == 1 ? "REC" : (tracks[1].occupied ? (tracks[1].muted ? "MUTE" : "PLAY") : "EMPTY");
    const char* state2 = recording && recording_slot == 2 ? "REC" : (tracks[2].occupied ? (tracks[2].muted ? "MUTE" : "PLAY") : "EMPTY");
    iprintf("\x1b[3;0HTRACK 1 %s %s", selected_track == 0 ? ">" : " ", state0);
    iprintf("\x1b[5;0HTRACK 2 %s %s", selected_track == 1 ? ">" : " ", state1);
    iprintf("\x1b[7;0HTRACK 3 %s %s", selected_track == 2 ? ">" : " ", state2);
    for (int i = 0; i < TRACKS; ++i) {
        iprintf("\x1b[%d;0H", 4 + i * 2);
        if (!tracks[i].occupied) { iprintf("EMPTY"); continue; }
        uint32_t phase = loop_logic::phase_ms(transport_ms, tracks[i].duration_ms);
        int playhead = (int)((phase * 20u) / tracks[i].duration_ms);
        for (int x = 0; x < 20; ++x) iprintf("%c", x == playhead ? '|' : (bucket_has_activity(tracks[i], x) ? '#' : '.'));
        iprintf(" %lums", (unsigned long)tracks[i].duration_ms);
    }
    iprintf("\x1b[10;0H%s %s  OCT %d  T:%s", recording ? "REC" : (paused ? "PAUSED" : "RUN"), b_hold_clear ? "CLEAR?" : "       ", octave, TIMBRE_NAMES[timbre]);
    iprintf("\x1b[12;0HA REC  B UNDO/CLEAR  START PAUSE  SELECT MUTE");
    iprintf("\x1b[13;0HXY TIMBRE/SUSTAIN  L VIB  R GLIDE");
}

static void draw_bottom(int key) {
    consoleSelect(&bottom_console);
    iprintf("\x1b[1;0HOCT %d   TRACK %d   %s", octave, selected_track + 1, TIMBRE_NAMES[timbre]);
    iprintf("\x1b[4;0H");
    for (int i = 0; i < KEY_COUNT; ++i) iprintf("|%-3s", NOTE_NAMES[i]);
    iprintf("|");
    for (int row = 7; row < 17; ++row) {
        iprintf("\x1b[%d;0H", row);
        for (int i = 0; i < KEY_COUNT; ++i) iprintf("| %c ", i == key ? '#' : ' ');
        iprintf("|");
    }
    iprintf("\x1b[18;0HTOUCH ANY STRIPE TO PLAY");
}

int main() {
    consoleInit(&top_console, 3, BgType_Text4bpp, BgSize_T_256x256, 31, 0, true, true);
    consoleInit(&bottom_console, 3, BgType_Text4bpp, BgSize_T_256x256, 31, 0, false, true);
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
        if (down & KEY_X) timbre = (timbre + 1) & 3;
        if (down & KEY_A) { if (recording) stop_recording(now); else start_recording(now); }
        if (down & KEY_B) { b_hold_started = now; b_hold_clear = true; }
        if (!(held & KEY_B) && b_hold_clear) { if (now - b_hold_started < 850) undo_recording(); b_hold_clear = false; }
        if ((held & KEY_B) && now - b_hold_started >= 850) { clear_selected(); b_hold_clear = false; }

        int key = -1;
        if (touchRead(&touch) && touch.px < 256 && touch.py < 192) key = (touch.px * KEY_COUNT) / 256;
        if (key >= 0) begin_note(key, now, (held & KEY_R) != 0);
        else if (!(held & KEY_TOUCH) && !(held & KEY_Y)) end_live_note(now);
        update_live_effects(now, held);
        update_loops();
        draw_top();
        draw_bottom(key);
    }
    stop_all_voices();
    return 0;
}
