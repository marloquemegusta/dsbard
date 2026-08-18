#include <nds.h>
#ifndef __BLOCKSDS__
#include <calico/system/tick.h>
#endif
#ifdef DS_DEBUG_TRACE
#include <nds/debug.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "loop_logic.h"
#include "rhodes_48_bin.h"
#include "rhodes_60_bin.h"
#include "rhodes_72_bin.h"
#include "rhodes_84_bin.h"
#include "kalimba_48_bin.h"
#include "kalimba_60_bin.h"
#include "kalimba_72_bin.h"
#include "kalimba_84_bin.h"
#include "bass_48_bin.h"
#include "bass_60_bin.h"
#include "bass_72_bin.h"
#include "bass_84_bin.h"
#include "pluck_48_bin.h"
#include "pluck_60_bin.h"
#include "pluck_72_bin.h"
#include "pluck_84_bin.h"
#include "glass_48_bin.h"
#include "glass_60_bin.h"
#include "glass_72_bin.h"
#include "glass_84_bin.h"
#include "pad_48_bin.h"
#include "pad_60_bin.h"
#include "pad_72_bin.h"
#include "pad_84_bin.h"

using loop_logic::NoteEvent;
using loop_logic::Track;

#ifdef DS_DEBUG_TRACE
#define DS_TRACE(...) fprintf(stderr, __VA_ARGS__)
#else
#define DS_TRACE(...) ((void)0)
#endif

static constexpr int TRACKS = 3;
static constexpr int KEY_COUNT = 12;
static constexpr int MAX_VOICES = 4;
static constexpr int INSTRUMENTS = 10;
static constexpr int PCM_INSTRUMENTS = 6;
static constexpr int ROOTS = 4;
static constexpr int SAMPLE_RATE = 22050;
static constexpr int TOUCH_TIMELINE_TOP = 160;
static constexpr uint32_t MIN_LOOP_MS = 120;
static constexpr uint16_t JOURNAL_CAPACITY = 256;
static constexpr uint32_t HOLD_MS = 850;
static constexpr uint32_t LIVE_RELEASE_MS = 400;
static constexpr int MAX_LIVE_TAILS = 4;
static const int SAMPLE_ROOTS[ROOTS] = {48, 60, 72, 84};
static const char* TIMBRE_NAMES[INSTRUMENTS] = {"RHODES", "KALIMBA", "BASS", "PLUCK", "GLASS", "PAD", "SQUARE", "PULSE", "THIN", "DARK"};
static const DutyCycle PSG_DUTIES[4] = {DutyCycle_50, DutyCycle_25, DutyCycle_12, DutyCycle_75};
struct SampleRef { const void* data; uint32_t size; };
#define SAMPLE(kind, root) { kind##_##root##_bin, kind##_##root##_bin_size }
static const SampleRef SAMPLES[PCM_INSTRUMENTS][ROOTS] = {
    {SAMPLE(rhodes,48), SAMPLE(rhodes,60), SAMPLE(rhodes,72), SAMPLE(rhodes,84)},
    {SAMPLE(kalimba,48), SAMPLE(kalimba,60), SAMPLE(kalimba,72), SAMPLE(kalimba,84)},
    {SAMPLE(bass,48), SAMPLE(bass,60), SAMPLE(bass,72), SAMPLE(bass,84)},
    {SAMPLE(pluck,48), SAMPLE(pluck,60), SAMPLE(pluck,72), SAMPLE(pluck,84)},
    {SAMPLE(glass,48), SAMPLE(glass,60), SAMPLE(glass,72), SAMPLE(glass,84)},
    {SAMPLE(pad,48), SAMPLE(pad,60), SAMPLE(pad,72), SAMPLE(pad,84)}
};

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
static int voice_instruments[MAX_VOICES] = {-1, -1, -1, -1};
static uint32_t transport_ms = 0;
static uint32_t previous_ms = 0;
static uint32_t frame_count = 0;
static int previous_buttons = 0;
static bool paused = false;
static bool recording = false;
static bool recording_armed = false;
static uint32_t record_start = 0;
static uint16_t record_count = 0;
static int recording_slot = 0;
static Track* recording_target = nullptr;
static int open_note = -1;
static int open_note_midi = -1;
static uint32_t open_note_start = 0;
static uint8_t open_note_instrument = 0;
static uint32_t b_hold_started = 0;
static bool b_hold_clear = false;
static bool b_hold_cleared = false;
static int b_action = 0;
static int16_t journal_values[JOURNAL_CAPACITY] = {};
static bool journal_separators[JOURNAL_CAPACITY] = {};
static uint16_t journal_count = 0;
static uint32_t journal_flash_until = 0;
static uint32_t select_hold_started = 0;
static bool select_pending = false;
static bool select_help_active = false;
static int live_voice = -1;
static int live_note = -1;
static int live_journal_key = -1;
static int live_target_note = -1;
static int live_root = -1;
static uint32_t live_transition_started = 0;
static bool live_glide = false;
static bool vibrato_active = false;
struct LiveTail {
    int voice = -1;
    uint32_t started = 0;
};
static LiveTail live_tails[MAX_LIVE_TAILS] = {};
static bool sustain_active = false;
static bool trim_active = false;
static uint32_t trim_start_ms = 0;
static uint32_t trim_end_ms = 0;
static uint32_t trim_raw_duration_ms = 0;
static uint16_t trim_repeat_count = 1;
static bool trim_preview_playing = false;
static uint32_t trim_preview_started = 0;
static int trim_drag_handle = 0;
static Track trim_preview_track = {};
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
static int stable_touch_key = -1;
static uint8_t top_render_divider = 0;
static int last_loop_note[TRACKS] = {-1, -1, -1};
static uint32_t last_phase[TRACKS] = {0, 0, 0};
static uint32_t flash_until = 0;

static uint32_t now_ms() {
#ifdef __BLOCKSDS__
    return (uint32_t)(((uint64_t)frame_count * 1000u) / 60u);
#else
    return (uint32_t)(((uint64_t)tickGetCount() * 1000u) / TICK_FREQ);
#endif
}

static uint16_t note_frequency(int midi) {
    return loop_logic::frequency_for_midi(midi);
}

static int root_index_for_note(int midi) {
    int best = 0;
    for (int i = 1; i < ROOTS; ++i)
        if (abs(midi - SAMPLE_ROOTS[i]) < abs(midi - SAMPLE_ROOTS[best])) best = i;
    return best;
}

static uint16_t sample_frequency(int midi, int root_midi) {
    int delta = midi - root_midi;
    double ratio = 1.0;
    if (delta >= 0) for (int i = 0; i < delta; ++i) ratio *= 1.059463094;
    else for (int i = 0; i > delta; --i) ratio /= 1.059463094;
    uint32_t hz = (uint32_t)(SAMPLE_RATE * ratio + 0.5);
    return (uint16_t)(hz > 65535u ? 65535u : hz);
}

static bool is_psg_instrument(int instrument) { return instrument >= PCM_INSTRUMENTS; }

static uint16_t playback_frequency(int midi, int instrument, int root) {
    return is_psg_instrument(instrument) ? loop_logic::psg_frequency_for_midi(midi)
                                         : sample_frequency(midi, SAMPLE_ROOTS[root]);
}

// The DS speakers lose low fundamentals. Keep PCM audible across the keyboard
// without making the high register disproportionately loud.
static u8 pcm_volume_for_note(int midi, u8 base) {
    int compensation = midi < 72 ? (72 - midi) : 0;
    int volume = (int)base + compensation;
    return (u8)(volume > 127 ? 127 : volume);
}

static int play_instrument(int instrument, int midi, u8 volume) {
    if (is_psg_instrument(instrument))
        return soundPlayPSG(PSG_DUTIES[instrument - PCM_INSTRUMENTS],
                            loop_logic::psg_frequency_for_midi(midi), volume, 64);
    int root = root_index_for_note(midi);
    return soundPlaySample(SAMPLES[instrument][root].data, SoundFormat_8Bit,
                           SAMPLES[instrument][root].size, sample_frequency(midi, SAMPLE_ROOTS[root]),
                           pcm_volume_for_note(midi, volume), 64,
                           instrument == 5, instrument == 5 ? 35280u : 0u);
}

static void kill_voice(int slot) {
    if (voice_ids[slot] >= 0) soundKill(voice_ids[slot]);
    voice_ids[slot] = -1;
    voice_notes[slot] = -1;
    voice_instruments[slot] = -1;
}

static void set_voice(int slot, int note, int instrument) {
    if (note < 0) { kill_voice(slot); return; }
    instrument %= INSTRUMENTS;
    if (voice_notes[slot] == note && voice_instruments[slot] == instrument && voice_ids[slot] >= 0) return;
    kill_voice(slot);
    voice_ids[slot] = play_instrument(instrument, note, 82);
    voice_notes[slot] = note;
    voice_instruments[slot] = instrument;
}

static void stop_all_voices() {
    for (int i = 0; i < MAX_VOICES; ++i) kill_voice(i);
    for (int i = 0; i < MAX_LIVE_TAILS; ++i) {
        if (live_tails[i].voice >= 0) soundKill(live_tails[i].voice);
        live_tails[i] = {};
    }
    if (live_voice >= 0) soundKill(live_voice);
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
    event.instrument_id = open_note_instrument;
    DS_TRACE("DSDBG|REC_EVENT|index=%u|start=%lu|end=%lu|midi=%d|timbre=%u\n",
             (unsigned)(record_count - 1), (unsigned long)event.start_ms,
             (unsigned long)event.end_ms, (int)event.midi_note, (unsigned)event.instrument_id);
    open_note = -1;
}

static void add_live_tail(int voice, uint32_t at) {
    if (voice < 0) return;
    int slot = -1;
    for (int i = 0; i < MAX_LIVE_TAILS; ++i) {
        if (live_tails[i].voice < 0) { slot = i; break; }
    }
    if (slot < 0) {
        slot = 0;
        for (int i = 1; i < MAX_LIVE_TAILS; ++i)
            if (live_tails[i].started < live_tails[slot].started) slot = i;
        soundKill(live_tails[slot].voice);
    }
    live_tails[slot].voice = voice;
    live_tails[slot].started = at;
    soundSetVolume(voice, 88);
}

static void update_live_tails(uint32_t now) {
    for (int i = 0; i < MAX_LIVE_TAILS; ++i) {
        if (live_tails[i].voice < 0) continue;
        uint32_t elapsed = now - live_tails[i].started;
        if (elapsed >= LIVE_RELEASE_MS) {
            soundKill(live_tails[i].voice);
            live_tails[i] = {};
            continue;
        }
        int volume = (int)(88u * (LIVE_RELEASE_MS - elapsed) / LIVE_RELEASE_MS);
        soundSetVolume(live_tails[i].voice, volume);
    }
}

static void clear_journal() {
    journal_count = 0;
}

static void append_journal_note(int key) {
    if (journal_count >= JOURNAL_CAPACITY) return;
    journal_values[journal_count] = (int16_t)loop_logic::journal_value_for_key(key);
    journal_separators[journal_count] = false;
    ++journal_count;
    journal_flash_until = now_ms() + 120;
}

static void append_journal_separator() {
    if (journal_count >= JOURNAL_CAPACITY) return;
    journal_values[journal_count] = 0;
    journal_separators[journal_count] = true;
    ++journal_count;
    journal_flash_until = now_ms() + 120;
}

static void pop_journal() {
    if (journal_count == 0) return;
    --journal_count;
}

static void begin_note(int key, uint32_t at, bool glide) {
    int midi = loop_logic::midi_for_key(octave, key);
    if (recording_armed) recording_armed = false;
    if (live_note != midi) {
        if (live_journal_key != key) {
            append_journal_note(key);
            live_journal_key = key;
        }
        if (glide && live_voice >= 0 && live_note >= 0 && !(live_glide && live_target_note == midi)) {
            live_target_note = midi;
            live_transition_started = at;
            live_glide = true;
        } else {
            if (live_voice >= 0) {
                if (sustain_active) add_live_tail(live_voice, at);
                else soundKill(live_voice);
            }
            live_root = root_index_for_note(midi);
            live_voice = play_instrument(timbre, midi, 100);
            live_note = midi;
            live_target_note = midi;
            live_glide = false;
        }
    }
    if (recording && open_note != key) {
        // Event timestamps are relative to the start of the take.
        close_open_note(at - record_start);
        open_note = key;
        open_note_midi = midi;
        open_note_start = at - record_start;
        open_note_instrument = (uint8_t)timbre;
    }
}

static void end_live_note(uint32_t at) {
    if (live_voice >= 0) {
        if (sustain_active) add_live_tail(live_voice, at);
        else soundKill(live_voice);
    }
    live_voice = -1;
    live_note = -1;
    live_journal_key = -1;
    live_target_note = -1;
    live_glide = false;
    vibrato_active = false;
    // Keep the recorded event open while L sustains a released touch. It is
    // closed when sustain is released, when another note starts, or when the
    // recording stops.
    if (recording && !sustain_active) close_open_note(at - record_start);
}

static void refresh_live_timbre() {
    if (live_voice < 0 || live_note < 0) return;
    soundKill(live_voice);
    live_root = root_index_for_note(live_note);
    live_voice = play_instrument(timbre, live_note, 100);
}

static void clear_selected() {
    tracks[selected_track] = {};
    if ((recording || recording_armed) && recording_slot == selected_track) {
        recording = false;
        recording_armed = false;
        open_note = -1;
        recording_target = nullptr;
    }
    for (int i = 0; i < TRACKS; ++i) last_loop_note[i] = -1;
}

static void prepare_recording(uint32_t now) {
    recording_slot = selected_track;
    recording_track = {};
    recording_target = &recording_track;
    recording = true;
    recording_armed = true;
    record_start = now;
    record_count = 0;
    open_note = -1;
    open_note_midi = -1;
    DS_TRACE("DSDBG|REC_ARM|now=%lu|transport=%lu|slot=%d\n",
             (unsigned long)now, (unsigned long)transport_ms, recording_slot);
}

static void start_recording(uint32_t now) {
    prepare_recording(now);
}

static void build_trim_preview() {
    trim_preview_track = {};
    trim_preview_track.occupied = true;
    trim_preview_track.duration_ms = trim_end_ms - trim_start_ms;
    for (uint16_t i = 0; i < record_count && trim_preview_track.event_count < loop_logic::MAX_TRACK_EVENTS; ++i) {
        const NoteEvent& source = recording_track.events[i];
        if (source.end_ms <= trim_start_ms || source.start_ms >= trim_end_ms) continue;
        NoteEvent& event = trim_preview_track.events[trim_preview_track.event_count++];
        event.start_ms = source.start_ms < trim_start_ms ? 0 : source.start_ms - trim_start_ms;
        event.end_ms = source.end_ms > trim_end_ms ? trim_preview_track.duration_ms : source.end_ms - trim_start_ms;
        event.midi_note = source.midi_note;
        event.instrument_id = source.instrument_id;
    }
}

static void stop_recording(uint32_t now) {
    close_open_note(now - record_start);
    recording_track.duration_ms = now - record_start;
    if (recording_track.duration_ms == 0) recording_track.duration_ms = 1;
    recording = false;
    recording_armed = false;
    recording_target = nullptr;
    if (record_count == 0) return;
    trim_start_ms = recording_track.events[0].start_ms;
    trim_end_ms = recording_track.events[record_count - 1].end_ms;
    trim_raw_duration_ms = recording_track.duration_ms;
    if (trim_end_ms <= trim_start_ms) trim_end_ms = trim_start_ms + MIN_LOOP_MS;
    trim_repeat_count = 1;
    trim_active = true;
    trim_preview_playing = false;
    build_trim_preview();
    stop_all_voices();
    DS_TRACE("DSDBG|REC_STOP|now=%lu|start=%lu|end=%lu|count=%u|slot=%d\n",
             (unsigned long)now, (unsigned long)trim_start_ms,
             (unsigned long)trim_end_ms, (unsigned)record_count, recording_slot);
}

static void cancel_trim() {
    trim_active = false;
    trim_preview_playing = false;
    recording_track = {};
    record_count = 0;
    bottom_needs_redraw = true;
    rendered_bottom_key = -2;
    stop_all_voices();
}

static void accept_trim() {
    if (!trim_active || trim_preview_track.event_count == 0) { cancel_trim(); return; }
    Track& target = tracks[recording_slot];
    if (target.phrase_count >= loop_logic::MAX_TRACK_PHRASES) { cancel_trim(); return; }
    uint32_t phrase_duration = trim_preview_track.duration_ms;
    if (phrase_duration < MIN_LOOP_MS) phrase_duration = MIN_LOOP_MS;
    uint32_t base = target.duration_ms;
    uint16_t copies = trim_repeat_count;
    uint32_t added_duration = phrase_duration * (uint32_t)copies;
    if (target.event_count + (uint32_t)trim_preview_track.event_count * copies > loop_logic::MAX_TRACK_EVENTS) {
        copies = (uint16_t)((loop_logic::MAX_TRACK_EVENTS - target.event_count) / trim_preview_track.event_count);
        if (copies == 0) { cancel_trim(); return; }
        added_duration = phrase_duration * (uint32_t)copies;
    }
    undo_track = target;
    undo_slot = recording_slot;
    undo_available = true;
    loop_logic::Phrase& phrase = target.phrases[target.phrase_count++];
    phrase.start_ms = base;
    phrase.duration_ms = phrase_duration;
    phrase.event_count = trim_preview_track.event_count;
    phrase.repeat_count = copies;
    for (uint16_t copy = 0; copy < copies; ++copy) {
        uint32_t offset = base + phrase_duration * (uint32_t)copy;
        for (uint16_t i = 0; i < trim_preview_track.event_count; ++i) {
            NoteEvent& event = target.events[target.event_count++];
            event = trim_preview_track.events[i];
            event.start_ms += offset;
            event.end_ms += offset;
        }
    }
    target.duration_ms += added_duration;
    target.occupied = true;
    trim_active = false;
    trim_preview_playing = false;
    recording_track = {};
    record_count = 0;
    bottom_needs_redraw = true;
    rendered_bottom_key = -2;
    DS_TRACE("DSDBG|PHRASE_ACCEPT|slot=%d|start=%lu|duration=%lu|repeat=%u|track_duration=%lu\n",
             recording_slot, (unsigned long)base, (unsigned long)phrase_duration,
             (unsigned)copies, (unsigned long)target.duration_ms);
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
            int from = playback_frequency(live_note, timbre, live_root);
            int to = playback_frequency(live_target_note, timbre, live_root);
            int interpolated = from + ((to - from) * (int)elapsed) / 100;
            soundSetFreq(live_voice, (u16)interpolated);
            return;
        }
    }
    if ((held & KEY_L) && (held & KEY_LEFT) && (held & KEY_TOUCH)) {
        int frequency = playback_frequency(live_note, timbre, live_root);
        int vibrato = ((now / 30) & 1) ? 14 : -14;
        soundSetFreq(live_voice, (u16)(frequency + vibrato));
        vibrato_active = true;
    } else if (vibrato_active) {
        soundSetFreq(live_voice, playback_frequency(live_note, timbre, live_root));
        vibrato_active = false;
    }
}

static int active_track_event(const Track& track, uint32_t phase) {
    for (uint16_t i = 0; i < track.event_count; ++i)
        if (loop_logic::event_active(track.events[i], phase)) return i;
    return -1;
}

static uint32_t song_duration_ms() {
    uint32_t duration = 0;
    for (int i = 0; i < TRACKS; ++i)
        if (tracks[i].duration_ms > duration) duration = tracks[i].duration_ms;
    return duration;
}

static void draw_text_bitmap(u16* target, int x, int y, const char* text, u16 color, int scale = 2);
static void fill_rect(u16* target, int x0, int y0, int x1, int y1, u16 color);

static void draw_phrase_boxes(const Track& track, int y, uint32_t song_duration,
                              u16 fill, u16 border) {
    if (!track.occupied || track.phrase_count == 0 || song_duration == 0) return;
    for (uint8_t phrase_index = 0; phrase_index < track.phrase_count; ++phrase_index) {
        const loop_logic::Phrase& phrase = track.phrases[phrase_index];
        for (uint16_t repetition = 0; repetition < phrase.repeat_count; ++repetition) {
            uint32_t start = phrase.start_ms + phrase.duration_ms * (uint32_t)repetition;
            uint32_t end = start + phrase.duration_ms;
            int left = 91 + (int)(((uint64_t)start * 160u) / song_duration);
            int right = 91 + (int)(((uint64_t)end * 160u) / song_duration);
            if (left < 91) left = 91;
            if (right > 251) right = 251;
            if (right <= left) right = left + 1;
            fill_rect(top_bitmap, left, y + 19, right, y + 27, fill);
            for (int x = left; x <= right; ++x) {
                top_bitmap[(y + 18) * 256 + x] = border;
                top_bitmap[(y + 28) * 256 + x] = border;
            }
            top_bitmap[(y + 19) * 256 + left] = border;
            top_bitmap[(y + 27) * 256 + left] = border;
            top_bitmap[(y + 19) * 256 + right] = border;
            top_bitmap[(y + 27) * 256 + right] = border;
        }
    }
}

static void update_loops() {
    if (trim_active) {
        for (int i = 0; i < TRACKS; ++i) set_voice(i, -1, 0);
        int event_index = -1;
        if (trim_preview_playing && trim_preview_track.duration_ms > 0) {
            uint32_t phase = (now_ms() - trim_preview_started) % trim_preview_track.duration_ms;
            event_index = active_track_event(trim_preview_track, phase);
        }
        int note = event_index >= 0 ? trim_preview_track.events[event_index].midi_note : -1;
        int instrument = event_index >= 0 ? trim_preview_track.events[event_index].instrument_id : 0;
        set_voice(0, note, instrument);
        return;
    }
    uint32_t song_duration = song_duration_ms();
    uint32_t phase = song_duration == 0 ? 0 : transport_ms % song_duration;
    for (int i = 0; i < TRACKS; ++i) {
        int event_index = -1;
        if (tracks[i].occupied && !paused) {
            event_index = phase < tracks[i].duration_ms ? active_track_event(tracks[i], phase) : -1;
        }
        int note = event_index >= 0 ? tracks[i].events[event_index].midi_note : -1;
        int event_timbre = event_index >= 0 ? tracks[i].events[event_index].instrument_id : 0;
        if (tracks[i].occupied && tracks[i].duration_ms > 0) {
            if (!paused && phase < last_phase[i]) flash_until = now_ms() + 80;
            last_phase[i] = phase;
        }
        set_voice(i, tracks[i].muted ? -1 : note, event_timbre);
        if (note != last_loop_note[i]) {
            uint32_t current_phase = song_duration > 0 ? phase : 0;
            DS_TRACE("DSDBG|LOOP_NOTE|slot=%d|phase=%lu|event=%d|midi=%d|count=%u\n",
                     i, (unsigned long)current_phase, event_index, note, (unsigned)tracks[i].event_count);
        }
        last_loop_note[i] = note;
    }
}

static void draw_top(bool show_help) {
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
    bool waiting_for_note = recording_armed;
    draw_text_bitmap(top_bitmap, 178, 9, paused ? "PAUSED" : (recording ? "REC" : (recording_armed ? (waiting_for_note ? "WAIT" : "ARM") : "RUN")),
                     recording || recording_armed ? green : text, 1);

    if (recording && recording_target != nullptr) {
        recording_target->occupied = true;
        recording_target->duration_ms = (uint32_t)(now_ms() - record_start);
        if (recording_target->duration_ms == 0) recording_target->duration_ms = 1;
    }

    if (trim_active) {
        char trim_text[32];
        snprintf(trim_text, sizeof(trim_text), "TRIM %lu-%lu x%02u",
                 (unsigned long)trim_start_ms, (unsigned long)trim_end_ms,
                 (unsigned)trim_repeat_count);
        draw_text_bitmap(top_bitmap, 9, 30, trim_text, green, 1);
        draw_text_bitmap(top_bitmap, 9, 42, "START END  UP DOWN REPEAT", text, 1);
        dmaCopy(top_framebuffer, top_vram, 256 * 192 * sizeof(u16));
        return;
    }

    if (show_help) {
        fill_rect(top_bitmap, 4, 30, 251, 188, panel);
        draw_text_bitmap(top_bitmap, 10, 35, "CONTROLS", text, 1);
        draw_text_bitmap(top_bitmap, 10, 44, "ARROWS OCT TRACK", text, 1);
        draw_text_bitmap(top_bitmap, 10, 53, "A REC", text, 1);
        draw_text_bitmap(top_bitmap, 10, 62, "B DELETE", text, 1);
        draw_text_bitmap(top_bitmap, 10, 71, "B HOLD CLEAR", text, 1);
        draw_text_bitmap(top_bitmap, 10, 80, "X TIMBRE", text, 1);
        draw_text_bitmap(top_bitmap, 10, 89, "Y SEP", text, 1);
        draw_text_bitmap(top_bitmap, 10, 98, "L SUS L+LEFT VIB", text, 1);
        draw_text_bitmap(top_bitmap, 10, 107, "L+RIGHT GLIDE", text, 1);
        draw_text_bitmap(top_bitmap, 10, 116, "START PAUSE", text, 1);
        draw_text_bitmap(top_bitmap, 10, 125, "SEL MUTE HOLD HELP", text, 1);
        draw_text_bitmap(top_bitmap, 10, 134, "L+B UNDO R+B CLEAR", text, 1);
        draw_text_bitmap(top_bitmap, 10, 145, "NUMBERS ONLY", green, 1);
        dmaCopy(top_framebuffer, top_vram, 256 * 192 * sizeof(u16));
        return;
    }

    uint32_t total_duration = song_duration_ms();
    for (int i = 0; i < TRACKS; ++i) {
        int y = 30 + i * 37;
        bool selected = selected_track == i;
        bool is_recording = (recording || recording_armed) && recording_slot == i;
        const Track& track = tracks[i];
        const Track& visual_track = is_recording && recording_target != nullptr ? *recording_target : track;
        const char* state = recording_armed && recording_slot == i ? (waiting_for_note ? "WAIT" : "ARM")
            : (recording && recording_slot == i ? "REC"
            : (track.occupied ? (track.muted ? "MUTE" : "PLAY") : "EMPTY"));
        u16 card = selected ? panel_alt : panel;
        fill_rect(top_bitmap, 4, y, 251, y + 31, card);
        for (int x = 4; x <= 251; ++x) top_bitmap[y * 256 + x] = selected ? accent : line;
        draw_text_bitmap(top_bitmap, 9, y + 6, "TRACK", text, 1);
        char track_number[2] = {(char)('1' + i), '\0'};
        draw_text_bitmap(top_bitmap, 38, y + 6, track_number, text, 1);
        draw_text_bitmap(top_bitmap, 52, y + 6, state, is_recording ? green : text, 1);
        draw_phrase_boxes(track, y, total_duration, track.muted ? green : accent, line);
        if (is_recording && visual_track.duration_ms > 0) {
            int right = 91 + (int)(((uint64_t)visual_track.duration_ms * 160u) /
                                   (total_duration == 0 ? visual_track.duration_ms : total_duration));
            if (right < 92) right = 92;
            if (right > 251) right = 251;
            fill_rect(top_bitmap, 91, y + 19, right, y + 27, green);
            for (int x = 91; x <= right; ++x) {
                top_bitmap[(y + 18) * 256 + x] = line;
                top_bitmap[(y + 28) * 256 + x] = line;
            }
        }
        if ((track.occupied || is_recording) && visual_track.duration_ms > 0) {
            uint32_t total_duration = song_duration_ms();
            uint32_t phase = is_recording ? now_ms() - record_start
                : (total_duration == 0 ? 0 : transport_ms % total_duration);
            int playhead = 91 + (int)((phase * 160u) / visual_track.duration_ms);
            if (playhead > 251) playhead = 251;
            fill_rect(top_bitmap, playhead, y + 17, playhead + 1, y + 29, green);
        }
    }

    fill_rect(top_bitmap, 4, 142, 251, 172, panel);
    draw_text_bitmap(top_bitmap, 9, 146, "DIARY", text, 1);
    char diary_text[96] = {};
    int start = journal_count > 10 ? journal_count - 10 : 0;
    int cursor = 0;
    if (start > 0) cursor += snprintf(diary_text + cursor, sizeof(diary_text) - cursor, "... ");
    for (int i = start; i < journal_count && cursor < (int)sizeof(diary_text) - 8; ++i) {
        if (i > start) cursor += snprintf(diary_text + cursor, sizeof(diary_text) - cursor, " ");
        if (journal_separators[i]) cursor += snprintf(diary_text + cursor, sizeof(diary_text) - cursor, "|");
        else cursor += snprintf(diary_text + cursor, sizeof(diary_text) - cursor, "%d", (int)journal_values[i]);
    }
    if (journal_count == 0) snprintf(diary_text, sizeof(diary_text), "--");
    draw_text_bitmap(top_bitmap, 37, 146, diary_text,
                     now_ms() < journal_flash_until ? green : text, 1);
    if (b_hold_clear && b_action == 0) {
        int progress = (int)(((now_ms() - b_hold_started) * 80u) / HOLD_MS);
        if (progress > 80) progress = 80;
        draw_text_bitmap(top_bitmap, 9, 162, "CLEAR", text, 1);
        fill_rect(top_bitmap, 34, 163, 34 + progress, 167, green);
        fill_rect(top_bitmap, 34 + progress, 163, 114, 167, line);
    }

    fill_rect(top_bitmap, 4, 177, 251, 188, panel);
    draw_text_bitmap(top_bitmap, 9, 180, "OCT", text, 1);
    char octave_text[2] = {(char)('0' + octave), '\0'};
    draw_text_bitmap(top_bitmap, 25, 180, octave_text, text, 1);
    draw_text_bitmap(top_bitmap, 39, 180, TIMBRE_NAMES[timbre], text, 1);
    draw_text_bitmap(top_bitmap, 82, 180, "L SUS", text, 1);
    char live_text[32];
    if (live_note >= 0) {
        snprintf(live_text, sizeof(live_text), "LIVE %d %dHZ%s", live_note,
                 note_frequency(live_note), vibrato_active ? " V" : "");
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
        {'U', 5, 5, 5, 5, 7}, {'V', 5, 5, 5, 5, 2}, {'W', 5, 7, 7, 7, 5}, {'0', 7, 5, 5, 5, 7},
        {'1', 2, 6, 2, 2, 7}, {'2', 6, 1, 2, 4, 7}, {'3', 6, 1, 6, 1, 6},
        {'4', 5, 5, 7, 1, 1}, {'5', 7, 4, 6, 1, 6}, {'6', 3, 4, 7, 5, 7},
        {'7', 7, 1, 2, 2, 2}, {'8', 7, 5, 7, 5, 7}, {'9', 7, 5, 7, 1, 6},
        {'Y', 5, 5, 2, 2, 2}, {'|', 2, 2, 2, 2, 2},
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
    char label[3];
    snprintf(label, sizeof(label), "%d", index);
    draw_text_bitmap(bottom_bitmap, x, y, label, color, 2);
}

static void present_bottom_full() {
    dmaCopy(bottom_framebuffer, bottom_vram, 256 * 192 * sizeof(u16));
}

static void present_bottom_band(int index) {
    if (index < 0 || index >= KEY_COUNT) return;
    int left = (index * 256) / KEY_COUNT;
    int right = ((index + 1) * 256) / KEY_COUNT;
    for (int y = 36; y < TOUCH_TIMELINE_TOP; ++y)
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
    for (int y = 38; y < TOUCH_TIMELINE_TOP; ++y)
        for (int x = left + 1; x < right; ++x)
            bottom_bitmap[y * 256 + x] = fill;
    for (int y = 36; y < TOUCH_TIMELINE_TOP; ++y) bottom_bitmap[y * 256 + left] = separator;
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
        int label_width = (i < 10 ? 6 : 14);
        int label_x = left + ((right - left) - label_width) / 2;
        fill_rect(bottom_bitmap, left + 2, 21, right - 3, 35, label_panel);
        draw_note_label(i, label_x, 24, label);
        int marker_x = left + (right - left) / 2;
        for (int y = 20; y <= 22; ++y)
            for (int x = marker_x - 1; x <= marker_x + 1; ++x)
                bottom_bitmap[y * 256 + x] = separator;
    }
    const u16 timeline_background = opaque | RGB15(2, 3, 6);
    const u16 timeline_line = opaque | RGB15(9, 12, 18);
    for (int y = TOUCH_TIMELINE_TOP; y < 192; ++y)
        for (int x = 0; x < 256; ++x) bottom_bitmap[y * 256 + x] = timeline_background;
    for (int x = 0; x < 256; ++x) bottom_bitmap[TOUCH_TIMELINE_TOP * 256 + x] = timeline_line;
    rendered_bottom_key = key;
    rendered_octave = octave;
    rendered_track = selected_track;
    rendered_timbre = timbre;
    bottom_needs_redraw = false;
    present_bottom_full();
}

static void draw_bottom(int key) {
    if (trim_active) {
        draw_bottom_full(-1);
        const u16 opaque = BIT(15);
        const u16 background = opaque | RGB15(2, 3, 6);
        const u16 line = opaque | RGB15(12, 16, 24);
        const u16 block = opaque | RGB15(20, 25, 31);
        const u16 handle = opaque | RGB15(24, 31, 22);
        for (int y = 160; y < 192; ++y)
            for (int x = 0; x < 256; ++x) bottom_bitmap[y * 256 + x] = background;
        for (int x = 7; x <= 248; ++x) bottom_bitmap[174 * 256 + x] = line;
        int start_x = 8 + (int)(((uint64_t)trim_start_ms * 240u) / trim_raw_duration_ms);
        int end_x = 8 + (int)(((uint64_t)trim_end_ms * 240u) / trim_raw_duration_ms);
        for (uint16_t i = 0; i < record_count; ++i) {
            const NoteEvent& event = recording_track.events[i];
            int left = 8 + (int)(((uint64_t)event.start_ms * 240u) / trim_raw_duration_ms);
            int right = 8 + (int)(((uint64_t)event.end_ms * 240u) / trim_raw_duration_ms);
            fill_rect(bottom_bitmap, left, 168, right > left ? right : left + 1, 181, block);
        }
        fill_rect(bottom_bitmap, start_x - 2, 164, start_x + 2, 187, handle);
        fill_rect(bottom_bitmap, end_x - 2, 164, end_x + 2, 187, handle);
        draw_text_bitmap(bottom_bitmap, start_x - 3, 184, "S", handle, 1);
        draw_text_bitmap(bottom_bitmap, end_x - 3, 184, "E", handle, 1);
        present_bottom_full();
        return;
    }
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
#ifdef __BLOCKSDS__
    while (1) {
#else
    while (pmMainLoop()) {
#endif
        swiWaitForVBlank();
        ++frame_count;
        scanKeys();
        uint32_t now = now_ms();
        uint32_t delta = now - previous_ms;
        previous_ms = now;
        if (!paused) transport_ms += delta;

        int held = keysHeld();
        int pressed = held & ~previous_buttons;
        int released = previous_buttons & ~held;
        if (!trim_active && (pressed & KEY_UP)) if (octave < 6) ++octave;
        if (!trim_active && (pressed & KEY_DOWN)) if (octave > 2) --octave;
        if (trim_active && (pressed & KEY_UP) && trim_repeat_count < 99) ++trim_repeat_count;
        if (trim_active && (pressed & KEY_DOWN) && trim_repeat_count > 1) --trim_repeat_count;
        if (!trim_active && !(held & KEY_TOUCH)) {
            if (pressed & KEY_LEFT) selected_track = loop_logic::wrap_track(selected_track, -1);
            if (pressed & KEY_RIGHT) selected_track = loop_logic::wrap_track(selected_track, 1);
        }
        if (pressed & KEY_START) {
            if (trim_active) {
                trim_preview_playing = !trim_preview_playing;
                trim_preview_started = now;
            } else {
                paused = !paused;
                if (paused) update_loops();
            }
        }
        if (!trim_active && (pressed & KEY_SELECT) && !select_pending) {
            select_hold_started = now;
            select_pending = true;
            select_help_active = false;
        }
        if (select_pending && now - select_hold_started >= HOLD_MS) select_help_active = true;
        if ((released & KEY_SELECT) && select_pending) {
            if (!select_help_active) tracks[selected_track].muted = !tracks[selected_track].muted;
            select_pending = false;
            select_help_active = false;
        }
        if (!trim_active && (pressed & KEY_X)) { timbre = (timbre + 1) % INSTRUMENTS; refresh_live_timbre(); }
        sustain_active = (held & KEY_L) != 0;
        if (!trim_active && (pressed & KEY_Y) && !(held & KEY_TOUCH)) append_journal_separator();
        if (pressed & KEY_A) {
            if (trim_active) accept_trim();
            else if (recording) stop_recording(now);
            else start_recording(now);
        }
        if (trim_active && (pressed & KEY_B)) cancel_trim();
        if (!trim_active && (pressed & KEY_B) && !b_hold_clear) {
            b_hold_started = now;
            b_hold_clear = true;
            b_hold_cleared = false;
            b_action = (held & KEY_L) ? 1 : ((held & KEY_R) ? 2 : 0);
        }
        if (!trim_active && (released & KEY_B) && b_hold_clear) {
            if (b_action == 1) undo_recording();
            else if (b_action == 2) clear_selected();
            else if (!b_hold_cleared) pop_journal();
            b_hold_clear = false;
            b_hold_cleared = false;
            b_action = 0;
        }
        if (!trim_active && b_hold_clear && b_action == 0 && !b_hold_cleared && now - b_hold_started >= HOLD_MS) {
            clear_journal();
            b_hold_cleared = true;
        }

        int key = -1;
        touchRead(&touch);
        if (trim_active) {
            if ((pressed & KEY_TOUCH) && touch.py >= 160) {
                int start_x = 8 + (int)(((uint64_t)trim_start_ms * 240u) / trim_raw_duration_ms);
                int end_x = 8 + (int)(((uint64_t)trim_end_ms * 240u) / trim_raw_duration_ms);
                trim_drag_handle = abs((int)touch.px - start_x) <= abs((int)touch.px - end_x) ? 1 : 2;
            }
            if ((held & KEY_TOUCH) && trim_drag_handle != 0 && touch.py >= 160) {
                int x = touch.px < 8 ? 8 : (touch.px > 248 ? 248 : touch.px);
                uint32_t value = (uint32_t)(((uint64_t)(x - 8) * trim_raw_duration_ms) / 240u);
                if (trim_drag_handle == 1) {
                    if (value >= trim_end_ms) value = trim_end_ms - 1;
                    trim_start_ms = value;
                } else {
                    if (value <= trim_start_ms) value = trim_start_ms + 1;
                    trim_end_ms = value > trim_raw_duration_ms ? trim_raw_duration_ms : value;
                }
                build_trim_preview();
                if (trim_preview_playing) trim_preview_started = now;
            }
            if (!(held & KEY_TOUCH)) trim_drag_handle = 0;
        } else if ((held & KEY_TOUCH) && touch.py < 192) {
            // Accept zone changes on the first hardware sample. Requiring two
            // consecutive 60 Hz samples loses fast notes and glides on real DS.
            stable_touch_key = loop_logic::key_for_x(touch.px);
            key = stable_touch_key;
        } else {
            stable_touch_key = -1;
        }
        if (key >= 0) begin_note(key, now, (held & KEY_L) && (held & KEY_RIGHT));
        else if (!(held & KEY_TOUCH)) end_live_note(now);
        update_live_effects(now, held);
        update_live_tails(now);
        update_loops();
        // The top screen is expensive to rasterize and copy. Keep input/audio
        // at 60 Hz while updating the visual dashboard at 30 Hz.
        if (++top_render_divider >= 2) {
            top_render_divider = 0;
            draw_top(select_help_active);
        }
        draw_bottom(key);
        previous_buttons = held;
    }
    stop_all_voices();
    return 0;
}
