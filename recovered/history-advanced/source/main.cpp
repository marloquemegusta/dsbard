#include <nds.h>
#include <nds/system_counter.h>
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
using loop_logic::PendingNoteInsertion;
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
static constexpr int COMPOSER_RULER_TOP = 27;
static constexpr int COMPOSER_RULER_BOTTOM = 37;
static constexpr int COMPOSER_TRACK_TOP = 39;
static constexpr int COMPOSER_TRACK_ROW = 36;
static constexpr int COMPOSER_TRACK_HEIGHT = 32;
static constexpr uint32_t MIN_LOOP_MS = 120;
static constexpr uint16_t JOURNAL_CAPACITY = 256;
static constexpr uint32_t HOLD_MS = 850;
static constexpr int MAX_LIVE_TAILS = 4;
static constexpr uint32_t DEFAULT_PULSE_MS = 1000;
static constexpr uint32_t MIN_PULSE_MS = 250;
static constexpr uint32_t MAX_PULSE_MS = 2000;
static constexpr uint32_t COMPOSER_CONTENT_LIMIT_MS = 60000;
static constexpr uint32_t PULSE_STEP_MS = 125;
static constexpr uint32_t METRONOME_CLICK_MS = 100;
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
static uint32_t master_clock_ms = 0;
static int previous_buttons = 0;
static bool paused = false;
static bool transport_finished = false;
static bool loop_enabled = true;
static loop_logic::TimelineSelection timeline_selection = {0, 0, 0, false};
static bool focus_timeline = false;
static int phrase_picker_selected = -1;
static uint32_t pulse_ms = DEFAULT_PULSE_MS;
static uint8_t meter_beats = 4;
static bool recording = false;
static bool recording_armed = false;
static uint32_t record_start = 0;
static uint16_t record_count = 0;
static int recording_slot = 0;
static Track* recording_target = nullptr;
static bool composer_keyboard_dragging = false;
static bool composer_ruler_scrubbing = false;
static bool composer_view_panning = false;
static int composer_drag_key = -1;
static int composer_drag_track = -1;
static uint32_t composer_drag_drop_ms = 0;
static int composer_pan_origin_x = 0;
static uint32_t composer_pan_origin_start_ms = 0;
static uint32_t composer_view_start_ms = 0;
static uint32_t composer_view_ms = loop_logic::COMPOSER_VIEW_MS;
static int open_note = -1;
static int open_note_midi = -1;
static uint32_t open_note_start = 0;
static uint8_t open_note_instrument = 0;
static uint8_t open_note_sustain_level = 0;
static uint32_t b_hold_started = 0;
static bool b_hold_clear = false;
static bool b_hold_cleared = false;
static int b_action = 0;
static int16_t journal_values[JOURNAL_CAPACITY] = {};
static uint16_t journal_count = 0;
static uint32_t journal_flash_until = 0;
static uint32_t select_hold_started = 0;
static bool select_pending = false;
static bool select_help_active = false;
static int live_voice = -1;
static int live_note = -1;
static int live_journal_key = -1;
static int live_root = -1;
static bool vibrato_active = false;
static uint8_t vibrato_depth_level = 0;
static uint8_t sustain_release_level = 2;
static uint8_t rendered_vibrato_depth_level = 255;
static uint8_t rendered_sustain_release_level = 255;
static bool rendered_sustain_active = false;
static constexpr uint32_t SUSTAIN_RELEASE_MIN_MS = 180;
static constexpr uint32_t SUSTAIN_RELEASE_STEP_MS = 300;
struct LiveTail {
    int voice = -1;
    uint32_t started = 0;
    uint32_t release_ms = 0;
};
static LiveTail live_tails[MAX_LIVE_TAILS] = {};
static bool sustain_active = false;
static bool loop_button_pending = false;
static bool loop_button_direction_used = false;
static bool trim_active = false;
static uint32_t trim_start_ms = 0;
static uint32_t trim_end_ms = 0;
static uint32_t trim_raw_duration_ms = 0;
static uint16_t trim_repeat_count = 1;
static bool trim_preview_playing = false;
static uint32_t trim_preview_started_ms = 0;
static uint32_t trim_preview_last_phase = 0;
static bool trim_preview_phase_valid = false;
static int trim_drag_handle = 0;
static int trim_selected_event = -1;
static loop_logic::NoteEditMode trim_edit_mode = loop_logic::NOTE_MOVE;
static bool trim_touch_editing = false;
static uint32_t trim_touch_origin_ms = 0;
static uint32_t trim_edit_original_start_ms = 0;
static uint32_t trim_edit_original_end_ms = 0;
static bool trim_touch_free_changed = false;
static Track trim_preview_track = {};
static bool note_insert_active = false;
static bool note_insert_existing = false;
static uint16_t note_insert_existing_index = 0;
static PendingNoteInsertion note_insert_pending = {};
static int preview_note_midi = -1;
static uint8_t preview_note_instrument = 0;
static int note_insert_preview_voice = -1;
static int note_insert_preview_note = -1;
static int note_insert_preview_instrument = -1;
static char note_rejection_text[32] = {};
static uint32_t note_rejection_until = 0;
static bool editing_existing_phrase = false;
static int editing_phrase_index = -1;
static int bottom_bitmap_bg = -1;
static u16* bottom_vram = nullptr;
static u16 bottom_framebuffer[256 * 256] = {};
static u16* bottom_bitmap = bottom_framebuffer;
static int top_bitmap_bg = -1;
static u16* top_vram = nullptr;
static u16 top_framebuffer[256 * 256] = {};
static u16* top_bitmap = top_framebuffer;
static u16 trim_static_framebuffer[256 * 256] = {};
static bool trim_editor_dirty = true;
static int trim_playhead_x = -1;
static bool trim_full_present_pending = false;
static int trim_rect_left = 256;
static int trim_rect_right = -1;
static int trim_rect_top = 192;
static int trim_rect_bottom = -1;
static bool bottom_needs_redraw = true;
static int rendered_bottom_key = -2;
static bool rendered_bottom_playback = false;
static int rendered_octave = -1;
static int rendered_track = -1;
static int rendered_timbre = -1;
static uint32_t rendered_pulse_ms = 0;
static int stable_touch_key = -1;
static uint8_t top_render_divider = 0;
static int playback_key = -1;
static int last_loop_note[TRACKS] = {-1, -1, -1};
static bool last_loop_sustain[TRACKS] = {};
static uint8_t last_loop_sustain_level[TRACKS] = {};
static uint32_t last_phase[TRACKS] = {0, 0, 0};
static uint32_t flash_until = 0;
static bool metronome_enabled = false;
static int metronome_voice = -1;
static uint32_t metronome_click_until = 0;
static uint32_t metronome_last_pulse = 0;
static uint32_t metronome_last_phase = 0;
static bool metronome_phase_valid = false;

static uint32_t now_ms() {
    // All temporal consumers read this same monotonic hardware clock. It is
    // sampled once per main-loop iteration, but it advances independently of
    // rendering and VBlank frequency.
    return master_clock_ms;
}

static void show_note_rejection(const char* reason) {
    snprintf(note_rejection_text, sizeof(note_rejection_text), "REJECT %s", reason);
    note_rejection_until = now_ms() + 1400;
    bottom_needs_redraw = true;
    DS_TRACE("DSDBG|NOTE_REJECT|reason=%s\n", reason);
}

static void advance_master_clock() {
    // BlocksDS owns the timer used by this counter. Reading the monotonic
    // system counter keeps recording and metronome time independent of FPS,
    // and avoids interpreting timer 0 differently on emulator and DS.
    master_clock_ms = systemCounterTicksToMsec(
        (uint32_t)systemCounterGetTicks());
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
    if (note_insert_preview_voice >= 0) soundKill(note_insert_preview_voice);
    note_insert_preview_voice = -1;
    note_insert_preview_note = -1;
    note_insert_preview_instrument = -1;
}

static void reset_loop_playback() {
    for (int i = 0; i < TRACKS; ++i) {
        kill_voice(i);
        last_loop_note[i] = -1;
        last_loop_sustain[i] = false;
        last_loop_sustain_level[i] = 0;
        last_phase[i] = 0;
    }
    for (int i = 0; i < MAX_LIVE_TAILS; ++i) {
        if (live_tails[i].voice >= 0) soundKill(live_tails[i].voice);
        live_tails[i] = {};
    }
}

static void reset_trim_preview_audio() {
    kill_voice(0);
    if (note_insert_preview_voice >= 0) soundKill(note_insert_preview_voice);
    note_insert_preview_voice = -1;
    note_insert_preview_note = -1;
    note_insert_preview_instrument = -1;
    for (int i = 0; i < MAX_LIVE_TAILS; ++i) {
        if (live_tails[i].voice >= 0) soundKill(live_tails[i].voice);
        live_tails[i] = {};
    }
    last_loop_note[0] = -1;
    last_loop_sustain[0] = false;
    last_loop_sustain_level[0] = 0;
    trim_preview_started_ms = now_ms();
    trim_preview_last_phase = 0;
    trim_preview_phase_valid = false;
}

static void close_open_note(uint32_t at, uint8_t sustain_level = 0) {
    if (!recording || recording_target == nullptr || open_note < 0 || record_count >= 96) { open_note = -1; return; }
    NoteEvent& event = recording_target->events[record_count++];
    event.start_ms = open_note_start;
    event.end_ms = at > open_note_start ? at : open_note_start + 1;
    event.midi_note = open_note_midi;
    event.instrument_id = open_note_instrument;
    uint8_t captured_level = sustain_level > open_note_sustain_level ? sustain_level : open_note_sustain_level;
    event.sustain_on_release = captured_level > 0 ? 1 : 0;
    event.sustain_level = event.sustain_on_release ? captured_level : 0;
    DS_TRACE("DSDBG|REC_EVENT|index=%u|start=%lu|end=%lu|midi=%d|timbre=%u\n",
             (unsigned)(record_count - 1), (unsigned long)event.start_ms,
             (unsigned long)event.end_ms, (int)event.midi_note, (unsigned)event.instrument_id);
    open_note = -1;
    open_note_sustain_level = 0;
}

static uint32_t sustain_release_ms_for_level(uint8_t level) {
    return SUSTAIN_RELEASE_MIN_MS + (uint32_t)level * SUSTAIN_RELEASE_STEP_MS;
}

static void add_live_tail(int voice, uint32_t at, uint8_t sustain_level) {
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
    live_tails[slot].release_ms = sustain_release_ms_for_level(sustain_level);
    soundSetVolume(voice, 88);
}

static void update_live_tails(uint32_t now) {
    for (int i = 0; i < MAX_LIVE_TAILS; ++i) {
        if (live_tails[i].voice < 0) continue;
        uint32_t elapsed = now - live_tails[i].started;
        uint32_t release_ms = live_tails[i].release_ms;
        if (elapsed >= release_ms) {
            soundKill(live_tails[i].voice);
            live_tails[i] = {};
            continue;
        }
        int volume = (int)(88u * (release_ms - elapsed) / release_ms);
        soundSetVolume(live_tails[i].voice, volume);
    }
}

static loop_logic::PulseGrid pulse_grid() {
    return loop_logic::PulseGrid{pulse_ms};
}

static void clear_journal() {
    journal_count = 0;
}

static void append_journal_note(int key) {
    if (journal_count >= JOURNAL_CAPACITY) return;
    journal_values[journal_count] = (int16_t)loop_logic::journal_value_for_key(key);
    ++journal_count;
    journal_flash_until = now_ms() + 120;
}

static void pop_journal() {
    if (journal_count == 0) return;
    --journal_count;
}

static void begin_note(int key, uint32_t at) {
    int midi = loop_logic::midi_for_key(octave, key);
    preview_note_midi = midi;
    preview_note_instrument = (uint8_t)timbre;
    if (recording_armed) recording_armed = false;
    if (live_note != midi) {
        if (live_journal_key != key) {
            if (!recording) append_journal_note(key);
            live_journal_key = key;
        }
        if (live_voice >= 0) {
            if (sustain_active) add_live_tail(live_voice, at, sustain_release_level);
            else soundKill(live_voice);
        }
        live_root = root_index_for_note(midi);
        live_voice = play_instrument(timbre, midi, 100);
        live_note = midi;
    }
    if (recording && open_note != key) {
        // Event timestamps are relative to the start of the take.
        close_open_note(at - record_start, sustain_active ? sustain_release_level : 0);
        open_note = key;
        open_note_midi = midi;
        open_note_start = at - record_start;
        open_note_instrument = (uint8_t)timbre;
        open_note_sustain_level = sustain_active ? sustain_release_level : 0;
    }
    if (recording && open_note >= 0 && sustain_active && sustain_release_level > open_note_sustain_level)
        open_note_sustain_level = sustain_release_level;
}

static void end_live_note(uint32_t at) {
    if (live_voice >= 0) {
        if (sustain_active) add_live_tail(live_voice, at, sustain_release_level);
        else soundKill(live_voice);
    }
    live_voice = -1;
    live_note = -1;
    live_journal_key = -1;
    vibrato_active = false;
    // The event ends when the touch ends. Sustain only adds a tail; it must
    // not turn the recorded key press into one long held note.
    if (recording) close_open_note(at - record_start, sustain_active ? sustain_release_level : 0);
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
    focus_timeline = false;
    phrase_picker_selected = -1;
    editing_existing_phrase = false;
    editing_phrase_index = -1;
    recording_slot = selected_track;
    recording_track = {};
    recording_target = &recording_track;
    recording = true;
    recording_armed = true;
    record_start = now;
    record_count = 0;
    open_note = -1;
    open_note_sustain_level = 0;
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
        event.sustain_on_release = source.sustain_on_release;
        event.sustain_level = source.sustain_level;
    }
    trim_editor_dirty = true;
    trim_playhead_x = -1;
}

static void stop_recording(uint32_t now) {
    close_open_note(now - record_start, sustain_active ? sustain_release_level : 0);
    recording_track.event_count = record_count;
    loop_logic::snap_note_events(recording_track, pulse_ms);
    recording_track.duration_ms = loop_logic::snap_loop_duration(now - record_start, pulse_ms);
    if (recording_track.duration_ms == 0) recording_track.duration_ms = 1;
    recording = false;
    recording_armed = false;
    recording_target = nullptr;
    if (record_count == 0) return;
    // Keep the complete capture as the phrase range. Its boundaries are the
    // stable beat grid, including intentional leading/trailing silence.
    trim_start_ms = 0;
    trim_end_ms = recording_track.duration_ms;
    trim_raw_duration_ms = recording_track.duration_ms;
    trim_repeat_count = 1;
    trim_selected_event = -1;
    trim_touch_editing = false;
    trim_active = true;
    trim_preview_playing = false;
    build_trim_preview();
    stop_all_voices();
    DS_TRACE("DSDBG|REC_STOP|now=%lu|start=%lu|end=%lu|count=%u|slot=%d\n",
             (unsigned long)now, (unsigned long)trim_start_ms,
             (unsigned long)trim_end_ms, (unsigned)record_count, recording_slot);
}

static bool rebuild_track_with_phrase(const Track& original, int phrase_index,
                                      const Track& edited_phrase, uint16_t edited_repeat_count,
                                      Track& rebuilt) {
    return loop_logic::rebuild_track_with_phrase_preserving_direct(
        original, phrase_index, edited_phrase, edited_repeat_count, rebuilt);
}

static bool open_phrase_editor(int track_index, int phrase_index) {
    if (track_index < 0 || track_index >= TRACKS) return false;
    const Track& source = tracks[track_index];
    if (phrase_index < 0 || phrase_index >= source.phrase_count) return false;
    const loop_logic::Phrase& phrase = source.phrases[phrase_index];
    recording_slot = track_index;
    recording_track = {};
    recording_track.occupied = true;
    recording_track.duration_ms = phrase.duration_ms;
    for (uint16_t i = 0; i < source.event_count &&
         recording_track.event_count < phrase.event_count; ++i) {
        if (!loop_logic::event_belongs_to_phrase(source.events[i], phrase, 0)) continue;
        recording_track.events[recording_track.event_count] = source.events[i];
        recording_track.events[recording_track.event_count].start_ms -= phrase.start_ms;
        recording_track.events[recording_track.event_count].end_ms -= phrase.start_ms;
        ++recording_track.event_count;
    }
    if (phrase.event_count == 0 || recording_track.event_count != phrase.event_count) return false;
    record_count = phrase.event_count;
    trim_start_ms = 0;
    trim_end_ms = phrase.duration_ms;
    trim_raw_duration_ms = phrase.duration_ms;
    trim_repeat_count = phrase.repeat_count;
    trim_selected_event = -1;
    trim_touch_editing = false;
    trim_preview_playing = false;
    editing_existing_phrase = true;
    editing_phrase_index = phrase_index;
    focus_timeline = false;
    trim_active = true;
    build_trim_preview();
    reset_trim_preview_audio();
    bottom_needs_redraw = true;
    DS_TRACE("DSDBG|PHRASE_EDIT_OPEN|track=%d|phrase=%d|start=%lu|duration=%lu|repeat=%u\n",
             track_index, phrase_index, (unsigned long)phrase.start_ms,
             (unsigned long)phrase.duration_ms, (unsigned)phrase.repeat_count);
    return true;
}

static void cancel_trim() {
    if (note_insert_active) loop_logic::cancel_note_insertion(note_insert_pending);
    trim_active = false;
    trim_preview_playing = false;
    recording_track = {};
    record_count = 0;
    trim_selected_event = -1;
    trim_touch_editing = false;
    editing_existing_phrase = false;
    editing_phrase_index = -1;
    note_insert_active = false;
    note_insert_existing = false;
    note_insert_existing_index = 0;
    bottom_needs_redraw = true;
    rendered_bottom_key = -2;
    stop_all_voices();
}

static void accept_trim() {
    if (!trim_active || trim_preview_track.event_count == 0) { cancel_trim(); return; }
    Track& target = tracks[recording_slot];
    if (note_insert_active) {
        NoteEvent edited_event = trim_preview_track.events[0];
        edited_event.start_ms += trim_start_ms;
        edited_event.end_ms += trim_start_ms;
        NoteEvent inserted_event = edited_event;
        Track previous_track = target;
        int previous_slot = recording_slot;
        bool accepted = false;
        if (note_insert_existing) {
            accepted = loop_logic::replace_note_event(target, note_insert_existing_index, edited_event);
        } else {
            note_insert_pending.active = true;
            note_insert_pending.event = edited_event;
            accepted = loop_logic::commit_note_insertion(target, note_insert_pending);
        }
        if (!accepted) {
            show_note_rejection("OVERLAP/FULL");
            cancel_trim();
            return;
        }
        undo_track = previous_track;
        undo_slot = previous_slot;
        undo_available = true;
        trim_active = false;
        trim_preview_playing = false;
        recording_track = {};
        record_count = 0;
        trim_selected_event = -1;
        trim_touch_editing = false;
        note_insert_active = false;
        note_insert_existing = false;
        bottom_needs_redraw = true;
        rendered_bottom_key = -2;
        reset_trim_preview_audio();
        DS_TRACE("DSDBG|NOTE_INSERT_ACCEPT|track=%d|start=%lu|end=%lu|count=%u\n",
                 recording_slot, (unsigned long)inserted_event.start_ms,
                 (unsigned long)inserted_event.end_ms, (unsigned)target.event_count);
        return;
    }
    if (editing_existing_phrase) {
        int edited_index = editing_phrase_index;
        Track rebuilt = {};
        if (!rebuild_track_with_phrase(target, edited_index, trim_preview_track,
                                       trim_repeat_count, rebuilt)) {
            cancel_trim();
            return;
        }
        undo_track = target;
        undo_slot = recording_slot;
        undo_available = true;
        target = rebuilt;
        transport_ms = 0;
        for (int i = 0; i < TRACKS; ++i) last_phase[i] = 0;
        reset_trim_preview_audio();
        trim_active = false;
        trim_preview_playing = false;
        recording_track = {};
        record_count = 0;
        trim_selected_event = -1;
        trim_touch_editing = false;
        editing_existing_phrase = false;
        editing_phrase_index = -1;
        bottom_needs_redraw = true;
        rendered_bottom_key = -2;
        DS_TRACE("DSDBG|PHRASE_EDIT_ACCEPT|track=%d|phrase=%d|duration=%lu|repeat=%u|track_duration=%lu\n",
                 recording_slot, edited_index,
                 (unsigned long)target.phrases[edited_index].duration_ms,
                 (unsigned)target.phrases[edited_index].repeat_count,
                 (unsigned long)target.duration_ms);
        return;
    }
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
    // Start the newly accepted phrase at its own timeline origin instead of
    // leaving the global transport halfway through the freshly appended range.
    transport_ms = base;
    for (int i = 0; i < TRACKS; ++i) last_phase[i] = base;
    reset_trim_preview_audio();
    trim_active = false;
    trim_preview_playing = false;
    recording_track = {};
    record_count = 0;
    trim_selected_event = -1;
    trim_touch_editing = false;
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
    if (vibrato_depth_level > 0 && (held & KEY_TOUCH)) {
        int frequency = playback_frequency(live_note, timbre, live_root);
        int depth = (frequency * vibrato_depth_level) / 160;
        if (depth < 1) depth = 1;
        int vibrato = ((now / 30) & 1) ? depth : -depth;
        int adjusted = frequency + vibrato;
        if (adjusted < 1) adjusted = 1;
        if (adjusted > 65535) adjusted = 65535;
        soundSetFreq(live_voice, (u16)adjusted);
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

static void update_note_insert_preview() {
    if (!note_insert_active || !trim_preview_playing || paused || trim_preview_track.duration_ms == 0) {
        if (note_insert_preview_voice >= 0) soundKill(note_insert_preview_voice);
        note_insert_preview_voice = -1;
        note_insert_preview_note = -1;
        note_insert_preview_instrument = -1;
        return;
    }
    uint32_t phase = transport_ms % trim_preview_track.duration_ms;
    int event_index = active_track_event(trim_preview_track, phase);
    int note = event_index >= 0 ? trim_preview_track.events[event_index].midi_note : -1;
    int instrument = event_index >= 0 ? trim_preview_track.events[event_index].instrument_id : -1;
    if (note == note_insert_preview_note && instrument == note_insert_preview_instrument) return;
    if (note_insert_preview_voice >= 0) soundKill(note_insert_preview_voice);
    note_insert_preview_voice = -1;
    note_insert_preview_note = note;
    note_insert_preview_instrument = instrument;
    if (note >= 0) note_insert_preview_voice = play_instrument(instrument, note, 100);
}

static uint32_t song_duration_ms() {
    uint32_t duration = 0;
    for (int i = 0; i < TRACKS; ++i)
        if (tracks[i].duration_ms > duration) duration = tracks[i].duration_ms;
    return duration;
}

static uint32_t composer_view_duration_ms() {
    return composer_view_ms;
}

static uint32_t composer_content_duration_ms() {
    uint32_t duration = composer_view_duration_ms();
    for (int i = 0; i < TRACKS; ++i)
        if (tracks[i].duration_ms > duration) duration = tracks[i].duration_ms;
    return duration > COMPOSER_CONTENT_LIMIT_MS ? COMPOSER_CONTENT_LIMIT_MS : duration;
}

static uint32_t composer_scroll_content_duration_ms() {
    return COMPOSER_CONTENT_LIMIT_MS;
}

static void start_note_insertion(uint32_t requested_ms) {
    if (preview_note_midi < 0) {
        show_note_rejection("NO PREVIEW");
        return;
    }
    uint32_t cycle_end = COMPOSER_CONTENT_LIMIT_MS;
    loop_logic::begin_note_insertion(note_insert_pending, (int16_t)preview_note_midi,
                                      preview_note_instrument, requested_ms, pulse_ms,
                                      0, cycle_end);
    recording_slot = selected_track;
    recording_track = {};
    recording_track.occupied = true;
    recording_track.duration_ms = cycle_end;
    recording_track.event_count = 1;
    recording_track.events[0] = note_insert_pending.event;
    record_count = 1;
    trim_start_ms = 0;
    trim_end_ms = cycle_end;
    trim_raw_duration_ms = cycle_end;
    trim_repeat_count = 1;
    trim_selected_event = 0;
    trim_touch_editing = false;
    editing_existing_phrase = false;
    editing_phrase_index = -1;
    note_insert_active = true;
    note_insert_existing = false;
    note_insert_existing_index = 0;
    trim_active = true;
    trim_preview_playing = false;
    build_trim_preview();
    reset_trim_preview_audio();
    bottom_needs_redraw = true;
    DS_TRACE("DSDBG|NOTE_INSERT_OPEN|track=%d|midi=%d|start=%lu|end=%lu\n",
             recording_slot, preview_note_midi,
             (unsigned long)note_insert_pending.event.start_ms,
             (unsigned long)note_insert_pending.event.end_ms);
}

static void open_direct_note_editor(uint16_t event_index) {
    Track& source = tracks[selected_track];
    if (event_index >= source.event_count) {
        show_note_rejection("INVALID NOTE");
        return;
    }
    uint32_t cycle_end = song_duration_ms();
    if (cycle_end == 0) cycle_end = source.events[event_index].end_ms;
    recording_slot = selected_track;
    recording_track = {};
    recording_track.occupied = true;
    recording_track.duration_ms = cycle_end;
    recording_track.event_count = 1;
    recording_track.events[0] = source.events[event_index];
    record_count = 1;
    trim_start_ms = 0;
    trim_end_ms = cycle_end;
    trim_raw_duration_ms = cycle_end;
    trim_repeat_count = 1;
    trim_selected_event = 0;
    trim_touch_editing = false;
    editing_existing_phrase = false;
    editing_phrase_index = -1;
    note_insert_active = true;
    note_insert_existing = true;
    note_insert_existing_index = event_index;
    note_insert_pending = {true, source.events[event_index]};
    trim_active = true;
    trim_preview_playing = false;
    build_trim_preview();
    reset_trim_preview_audio();
    bottom_needs_redraw = true;
    DS_TRACE("DSDBG|NOTE_EDIT_OPEN|track=%d|event=%u|start=%lu|end=%lu\n",
             recording_slot, (unsigned)event_index,
             (unsigned long)source.events[event_index].start_ms,
             (unsigned long)source.events[event_index].end_ms);
}

static void playback_window(uint32_t& start, uint32_t& end) {
    uint32_t duration = focus_timeline ? composer_content_duration_ms() : song_duration_ms();
    if (!timeline_selection.explicit_range) {
        start = 0;
        end = duration;
        return;
    }
    loop_logic::normalize_selection(timeline_selection, duration);
    start = timeline_selection.range_start_ms;
    end = timeline_selection.range_end_ms;
}

static void draw_text_bitmap(u16* target, int x, int y, const char* text, u16 color, int scale = 2);
static void fill_rect(u16* target, int x0, int y0, int x1, int y1, u16 color);
static void present_bottom_full();
static void draw_monitor_screen(u16* target);

static void draw_phrase_boxes(u16* target, const Track& track, int y, uint32_t song_duration,
                              int timeline_left, int timeline_width, int box_top, int box_bottom,
                              u16 fill, u16 border, uint32_t timeline_start_ms = 0) {
    if (!track.occupied || track.phrase_count == 0 || song_duration == 0) return;
    for (uint8_t phrase_index = 0; phrase_index < track.phrase_count; ++phrase_index) {
        const loop_logic::Phrase& phrase = track.phrases[phrase_index];
        for (uint16_t repetition = 0; repetition < phrase.repeat_count; ++repetition) {
            uint32_t start = loop_logic::expanded_phrase_start(phrase, repetition);
            uint32_t end = loop_logic::expanded_phrase_end(phrase, repetition);
            if (end <= timeline_start_ms || start >= timeline_start_ms + song_duration) continue;
            uint32_t visible_start = start > timeline_start_ms ? start - timeline_start_ms : 0;
            uint32_t visible_end = end > timeline_start_ms + song_duration
                ? song_duration : end - timeline_start_ms;
            int left = timeline_left + (int)(((uint64_t)visible_start * (uint32_t)timeline_width) / song_duration);
            int right = timeline_left + (int)(((uint64_t)visible_end * (uint32_t)timeline_width) / song_duration);
            if (left < timeline_left) left = timeline_left;
            if (right > timeline_left + timeline_width) right = timeline_left + timeline_width;
            if (right <= left) right = left + 1;
            fill_rect(target, left, y + box_top, right, y + box_bottom, fill);
            for (int x = left; x <= right; ++x) {
                target[(y + box_top - 1) * 256 + x] = border;
                target[(y + box_bottom + 1) * 256 + x] = border;
            }
            target[(y + box_top) * 256 + left] = border;
            target[(y + box_bottom) * 256 + left] = border;
            target[(y + box_top) * 256 + right] = border;
            target[(y + box_bottom) * 256 + right] = border;
        }
    }
}

static void draw_direct_note_events(u16* target, const Track& track, int y,
                                    uint32_t song_duration, u16 fill, u16 border,
                                    uint32_t timeline_start_ms = 0) {
    if (!track.occupied || track.event_count == 0 || song_duration == 0) return;
    for (uint16_t i = 0; i < track.event_count; ++i) {
        const NoteEvent& event = track.events[i];
        bool belongs_to_phrase = false;
        for (uint8_t p = 0; p < track.phrase_count && !belongs_to_phrase; ++p) {
            const loop_logic::Phrase& phrase = track.phrases[p];
            for (uint16_t repetition = 0; repetition < phrase.repeat_count; ++repetition) {
                uint32_t start = loop_logic::expanded_phrase_start(phrase, repetition);
                uint32_t end = loop_logic::expanded_phrase_end(phrase, repetition);
                if (event.start_ms >= start && event.start_ms < end) {
                    belongs_to_phrase = true;
                    break;
                }
            }
        }
        if (belongs_to_phrase) continue;
        if (event.end_ms <= timeline_start_ms || event.start_ms >= timeline_start_ms + song_duration) continue;
        uint32_t visible_start = event.start_ms > timeline_start_ms
            ? event.start_ms - timeline_start_ms : 0;
        uint32_t visible_end = event.end_ms > timeline_start_ms + song_duration
            ? song_duration : event.end_ms - timeline_start_ms;
        int timeline_width = loop_logic::TIMELINE_RIGHT_X - loop_logic::TIMELINE_LEFT_X;
        int left = loop_logic::TIMELINE_LEFT_X + (int)(((uint64_t)visible_start * timeline_width) / song_duration);
        int right = loop_logic::TIMELINE_LEFT_X + (int)(((uint64_t)visible_end * timeline_width) / song_duration);
        loop_logic::ensure_note_visual_width(left, right,
                                             loop_logic::TIMELINE_LEFT_X,
                                             loop_logic::TIMELINE_RIGHT_X);
        fill_rect(target, left, y + 19, right, y + 27, fill);
        target[(y + 18) * 256 + left] = border;
        target[(y + 28) * 256 + right] = border;
    }
}

static int phrase_picker_hit(int x, int y) {
    if (y < 30 || y >= 142) return -1;
    int track_index = (y - 30) / 37;
    if (track_index < 0 || track_index >= TRACKS) return -1;
    const Track& track = tracks[track_index];
    uint32_t total_duration = focus_timeline ? composer_content_duration_ms() : song_duration_ms();
    if (total_duration == 0) return -1;
    for (uint8_t phrase_index = 0; phrase_index < track.phrase_count; ++phrase_index) {
        const loop_logic::Phrase& phrase = track.phrases[phrase_index];
        for (uint16_t repetition = 0; repetition < phrase.repeat_count; ++repetition) {
            uint32_t start_ms = loop_logic::expanded_phrase_start(phrase, repetition);
            if (right - left < loop_logic::NARROW_NOTE_PIXEL_WIDTH) {
                const u16 narrow_note_accent = BIT(15) | RGB15(31, 31, 20);
                int center_x = (left + right) / 2;
                for (int note_y = y + 20; note_y <= y + 26; ++note_y)
                    target[note_y * 256 + center_x] = narrow_note_accent;
            }
            int pitch = event.midi_note % 12;
            char pitch_text[4];
            snprintf(pitch_text, sizeof(pitch_text), "%d", pitch);
            int label_x = loop_logic::centered_note_label_x(left, right, pitch);
            if (right - left >= 8)
                draw_text_bitmap(target, label_x, y + 21, pitch_text, border, 1);
            uint32_t repetition_end_ms = loop_logic::expanded_phrase_end(phrase, repetition);
            int left = 91 + (int)(((uint64_t)start_ms * 160u) / total_duration);
            int right = 91 + (int)(((uint64_t)repetition_end_ms * 160u) / total_duration);
            if (right <= left) right = left + 3;
            if (x >= left && x <= right) {
                selected_track = track_index;
                return phrase_index;
            }
        }
    }
    return -1;
}

static int direct_note_hit(int x, int y) {
    if (!loop_logic::timeline_x_is_valid(x) || y < COMPOSER_TRACK_TOP ||
        y >= COMPOSER_TRACK_TOP + TRACKS * COMPOSER_TRACK_ROW) return -1;
    int track_index = (y - COMPOSER_TRACK_TOP) / COMPOSER_TRACK_ROW;
    if (y >= COMPOSER_TRACK_TOP + track_index * COMPOSER_TRACK_ROW + COMPOSER_TRACK_HEIGHT)
        return -1;
    if (track_index < 0 || track_index >= TRACKS) return -1;
    if (!loop_logic::note_lane_hit_y(
            y, COMPOSER_TRACK_TOP + track_index * COMPOSER_TRACK_ROW)) return -1;
    uint32_t duration = composer_view_duration_ms();
    uint32_t time_ms = loop_logic::viewport_time_for_x(x, composer_view_start_ms, duration);
    return loop_logic::find_direct_event_at(tracks[track_index], time_ms);
}

static void draw_phrase_picker() {
    draw_monitor_screen(bottom_bitmap);
    present_bottom_full();
}

static int timeline_x_for_ms(uint32_t value_ms, uint32_t duration_ms) {
    if (duration_ms == 0) return 8;
    int x = 8 + (int)(((uint64_t)value_ms * 240u) / duration_ms);
    return x < 8 ? 8 : (x > 248 ? 248 : x);
}

static uint32_t timeline_ms_for_x(int x, uint32_t duration_ms) {
    if (x < 8) x = 8;
    if (x > 248) x = 248;
    return duration_ms == 0 ? 0 : (uint32_t)(((uint64_t)(x - 8) * duration_ms) / 240u);
}

static int composer_timeline_x_for_ms(uint32_t value_ms) {
    if (value_ms <= composer_view_start_ms) return loop_logic::TIMELINE_LEFT_X;
    if (value_ms >= composer_view_start_ms + composer_view_duration_ms())
        return loop_logic::TIMELINE_RIGHT_X;
    return loop_logic::TIMELINE_LEFT_X +
        (int)(((uint64_t)(value_ms - composer_view_start_ms) *
               (uint32_t)(loop_logic::TIMELINE_RIGHT_X - loop_logic::TIMELINE_LEFT_X)) /
              composer_view_duration_ms());
}

static uint32_t dragged_note_free_request(uint32_t original_ms, uint32_t origin_ms,
                                          uint32_t current_ms) {
    int64_t requested = (int64_t)original_ms + (int64_t)current_ms - (int64_t)origin_ms;
    if (requested < 0) return 0;
    return (uint32_t)requested;
}

static int composer_view_x_for_time(uint32_t value_ms) {
    if (value_ms <= composer_view_start_ms) return loop_logic::TIMELINE_LEFT_X;
    uint32_t view_end = composer_view_start_ms + composer_view_duration_ms();
    if (value_ms >= view_end) return loop_logic::TIMELINE_RIGHT_X;
    return composer_timeline_x_for_ms(value_ms);
}

static uint32_t trim_preview_phase_ms() {
    if (trim_preview_track.duration_ms == 0) return 0;
    uint32_t elapsed_ms = now_ms() - trim_preview_started_ms;
    return elapsed_ms % trim_preview_track.duration_ms;
}

static void stop_metronome_voice() {
    if (metronome_voice >= 0) {
        // PSG tones are continuous by nature. Kill the channel explicitly;
        // volume zero alone is not enough on every DS audio implementation.
        soundSetVolume(metronome_voice, 0);
        soundKill(metronome_voice);
        DS_TRACE("DSDBG|METRO_STOP|now=%lu\n", (unsigned long)now_ms());
        metronome_voice = -1;
    }
}

static void update_metronome(uint32_t now) {
    if (metronome_voice >= 0 && (int32_t)(now - metronome_click_until) >= 0)
        stop_metronome_voice();
    // PULSE is a global musical clock. Recording, preview, transport and the
    // R mute toggle must never reset its phase.
    if (pulse_ms == 0) return;
    uint32_t phase = now;
    uint32_t pulse = phase / pulse_ms;
    bool wrapped = metronome_phase_valid && phase < metronome_last_phase;
    bool tick = !metronome_phase_valid || wrapped || pulse != metronome_last_pulse;
    if (tick && metronome_enabled) {
        bool bar_accent = (pulse % meter_beats) == 0;
        if (metronome_voice < 0) {
            metronome_voice = soundPlayPSG(DutyCycle_25,
                                           loop_logic::psg_frequency_for_midi(48),
                                           bar_accent ? 16 : 10, 64);
        } else {
            soundSetVolume(metronome_voice, bar_accent ? 16 : 10);
        }
        metronome_click_until = now + METRONOME_CLICK_MS;
        metronome_last_pulse = pulse;
        metronome_phase_valid = true;
        DS_TRACE("DSDBG|METRO_TICK|now=%lu|phase=%lu|pulse=%lu|period=%lu|wrapped=%d\n",
                 (unsigned long)now, (unsigned long)phase, (unsigned long)pulse,
                 (unsigned long)pulse_ms, wrapped ? 1 : 0);
    }
    if (tick) metronome_last_pulse = pulse;
    metronome_phase_valid = true;
    metronome_last_phase = phase;
}

static void update_loops() {
    playback_key = -1;
    if (trim_active && !note_insert_active) {
        // The preview owns voice 0. Do not kill it every frame: set_voice()
        // must be able to keep the sample/PSG voice alive between events.
        for (int i = 1; i < TRACKS; ++i) set_voice(i, -1, 0);
        if (!trim_preview_playing) {
            reset_trim_preview_audio();
            return;
        }
        int event_index = -1;
        if (trim_preview_track.duration_ms > 0) {
            uint32_t phase = trim_preview_phase_ms();
            bool wrapped = trim_preview_phase_valid && phase < trim_preview_last_phase;
            if (wrapped) {
                if (last_loop_sustain[0] && voice_ids[0] >= 0) {
                    int tail = voice_ids[0];
                    voice_ids[0] = -1;
                    voice_notes[0] = -1;
                    voice_instruments[0] = -1;
                    add_live_tail(tail, now_ms(), last_loop_sustain_level[0]);
                } else {
                    kill_voice(0);
                }
                last_loop_note[0] = -1;
            }
            trim_preview_last_phase = phase;
            trim_preview_phase_valid = true;
            event_index = active_track_event(trim_preview_track, phase);
        }
        int note = event_index >= 0 ? trim_preview_track.events[event_index].midi_note : -1;
        int instrument = event_index >= 0 ? trim_preview_track.events[event_index].instrument_id : 0;
        if (note != last_loop_note[0] && last_loop_sustain[0] && voice_ids[0] >= 0) {
            int tail = voice_ids[0];
            voice_ids[0] = -1;
            voice_notes[0] = -1;
            voice_instruments[0] = -1;
            add_live_tail(tail, now_ms(), last_loop_sustain_level[0]);
        }
        set_voice(0, note, instrument);
        last_loop_note[0] = note;
        last_loop_sustain[0] = event_index >= 0 && trim_preview_track.events[event_index].sustain_on_release != 0;
        last_loop_sustain_level[0] = event_index >= 0 ? trim_preview_track.events[event_index].sustain_level : 0;
        return;
    }
    uint32_t song_duration = focus_timeline ? composer_content_duration_ms() : song_duration_ms();
    uint32_t phase = song_duration == 0 ? 0 : transport_ms % song_duration;
    for (int i = 0; i < TRACKS; ++i) {
        int event_index = -1;
        if (tracks[i].occupied && !paused) {
            event_index = phase < tracks[i].duration_ms ? active_track_event(tracks[i], phase) : -1;
        }
        int note = event_index >= 0 ? tracks[i].events[event_index].midi_note : -1;
        int event_timbre = event_index >= 0 ? tracks[i].events[event_index].instrument_id : 0;
        if (recording && recording_slot == i) note = -1;
        if (i == selected_track && note >= 0 && !tracks[i].muted) {
            int first_midi = loop_logic::midi_for_key(octave, 0);
            if (note >= first_midi && note < first_midi + KEY_COUNT)
                playback_key = note - first_midi;
        }
        if (tracks[i].occupied && tracks[i].duration_ms > 0) {
            if (!paused && phase < last_phase[i]) flash_until = now_ms() + 80;
            last_phase[i] = phase;
        }
        if (note != last_loop_note[i] && last_loop_sustain[i] && voice_ids[i] >= 0) {
            int tail = voice_ids[i];
            voice_ids[i] = -1;
            voice_notes[i] = -1;
            voice_instruments[i] = -1;
            add_live_tail(tail, now_ms(), last_loop_sustain_level[i]);
        }
        set_voice(i, tracks[i].muted ? -1 : note, event_timbre);
        if (note != last_loop_note[i]) {
            uint32_t current_phase = song_duration > 0 ? phase : 0;
            DS_TRACE("DSDBG|LOOP_NOTE|slot=%d|phase=%lu|event=%d|midi=%d|count=%u\n",
                     i, (unsigned long)current_phase, event_index, note, (unsigned)tracks[i].event_count);
        }
        last_loop_note[i] = note;
        last_loop_sustain[i] = event_index >= 0 && tracks[i].events[event_index].sustain_on_release != 0;
        last_loop_sustain_level[i] = event_index >= 0 ? tracks[i].events[event_index].sustain_level : 0;
    }
    if (note_insert_active) update_note_insert_preview();
}

static void draw_monitor_screen(u16* target) {
    const u16 opaque = BIT(15);
    const u16 background = opaque | RGB15(1, 1, 2);
    const u16 panel = opaque | RGB15(8, 12, 18);
    const u16 panel_alt = opaque | RGB15(11, 16, 23);
    const u16 line = opaque | RGB15(10, 16, 24);
    const u16 text = opaque | RGB15(31, 31, 31);
    const u16 accent = opaque | RGB15(20, 25, 31);
    const u16 green = opaque | RGB15(20, 31, 22);
    for (int y = 0; y < 192; ++y)
        for (int x = 0; x < 256; ++x) target[y * 256 + x] = background;
    fill_rect(target, 4, 4, 251, 25, panel);
    for (int x = 4; x <= 251; ++x) {
        target[4 * 256 + x] = line;
        target[25 * 256 + x] = line;
    }
    draw_text_bitmap(target, 9, 9, "STYLOPHONE", text, 1);
    char header_hz[12];
    if (live_note >= 0) snprintf(header_hz, sizeof(header_hz), "HZ %d", note_frequency(live_note));
    else snprintf(header_hz, sizeof(header_hz), "HZ --");
    draw_text_bitmap(target, 146, 9, header_hz, live_note >= 0 ? green : text, 1);
    bool waiting_for_note = recording_armed;
    draw_text_bitmap(target, 178, 9, transport_finished ? "STOP" : (paused ? "PAUSED" : (recording ? "REC" : (recording_armed ? (waiting_for_note ? "WAIT" : "ARM") : "RUN"))),
                     recording || recording_armed ? green : text, 1);
    if (focus_timeline) {
        composer_view_start_ms = loop_logic::clamp_viewport_start(
            composer_view_start_ms, composer_scroll_content_duration_ms(),
            composer_view_duration_ms());
        fill_rect(target, 4, COMPOSER_RULER_TOP, 251, COMPOSER_RULER_BOTTOM,
                  panel);
        draw_text_bitmap(target, 8, 29, "TIME", text, 1);
        for (int x = loop_logic::TIMELINE_LEFT_X; x <= loop_logic::TIMELINE_RIGHT_X; ++x)
            target[COMPOSER_RULER_BOTTOM * 256 + x] = line;
        uint32_t marker_step_ms = composer_view_duration_ms() <= 5000 ? 1000 : 2000;
        uint32_t first_marker = (composer_view_start_ms / marker_step_ms) * marker_step_ms;
        if (first_marker < composer_view_start_ms) first_marker += marker_step_ms;
        uint32_t marker_end = composer_view_start_ms + composer_view_duration_ms();
        for (uint32_t marker = first_marker; marker <= marker_end; marker += marker_step_ms) {
            int marker_x = composer_timeline_x_for_ms(marker);
            for (int y = COMPOSER_RULER_BOTTOM - 3; y < COMPOSER_RULER_BOTTOM; ++y)
                target[y * 256 + marker_x] = accent;
            char marker_text[12];
            snprintf(marker_text, sizeof(marker_text), "%lus", (unsigned long)(marker / 1000u));
            draw_text_bitmap(target, marker_x - 4, 28, marker_text, text, 1);
        }
    }
    uint32_t total_duration = focus_timeline ? composer_view_duration_ms() : song_duration_ms();
    for (int i = 0; i < TRACKS; ++i) {
        int y = focus_timeline ? COMPOSER_TRACK_TOP + i * COMPOSER_TRACK_ROW
                              : 30 + i * 37;
        bool selected = selected_track == i;
        bool is_recording = (recording || recording_armed) && recording_slot == i;
        const Track& track = tracks[i];
        const Track& visual_track = is_recording && recording_target != nullptr ? *recording_target : track;
        const char* state = recording_armed && recording_slot == i ? (waiting_for_note ? "WAIT" : "ARM")
            : (recording && recording_slot == i ? "REC"
            : (track.occupied ? (track.muted ? "MUTE" : "PLAY") : "EMPTY"));
        u16 card = selected ? panel_alt : panel;
        fill_rect(target, 4, y, 251, y + 31, card);
        for (int x = 4; x <= 251; ++x) target[y * 256 + x] = selected ? accent : line;
        if (focus_timeline) {
            const u16 subdivision = opaque | RGB15(5, 8, 14);
            const u16 beat = opaque | RGB15(8, 13, 21);
            const u16 bar = opaque | RGB15(12, 19, 29);
            int timeline_width = loop_logic::TIMELINE_RIGHT_X - loop_logic::TIMELINE_LEFT_X;
            loop_logic::PulseGrid grid{pulse_ms};
            uint32_t view_end = composer_view_start_ms + total_duration;
            uint32_t first_step = (composer_view_start_ms * 4u) / pulse_ms;
            if (grid.time(first_step) < composer_view_start_ms) ++first_step;
            uint32_t last_step = (view_end * 4u) / pulse_ms + 1u;
            for (uint32_t step = first_step; step <= last_step; ++step) {
                uint32_t value = loop_logic::PulseGrid{pulse_ms}.time(step);
                if (value > view_end) break;
                int x = loop_logic::TIMELINE_LEFT_X +
                    (int)(((uint64_t)(value - composer_view_start_ms) * timeline_width) / total_duration);
                u16 color = loop_logic::pulse_grid_line_kind(step, meter_beats) == loop_logic::GRID_BAR
                    ? bar : (loop_logic::pulse_grid_line_kind(step, meter_beats) == loop_logic::GRID_BEAT
                        ? beat : subdivision);
                bool dotted = loop_logic::pulse_grid_line_kind(step, meter_beats) == loop_logic::GRID_SUBDIVISION;
                for (int grid_y = y + 1; grid_y < y + 31; ++grid_y)
                    if (!dotted || ((grid_y - y) & 1)) target[grid_y * 256 + x] = color;
            }
        }
        draw_text_bitmap(target, 9, y + 3, "TRK", text, 1);
        char track_number[2] = {(char)('1' + i), '\0'};
        draw_text_bitmap(target, 9, y + 11, track_number, text, 1);
        draw_text_bitmap(target, 9, y + 20, state, is_recording ? green : text, 1);
        int composer_left = loop_logic::TIMELINE_LEFT_X;
        int composer_width = loop_logic::TIMELINE_RIGHT_X - composer_left;
        draw_phrase_boxes(target, track, y, total_duration,
                          focus_timeline ? composer_left : 91,
                          focus_timeline ? composer_width : 160, 19, 27,
                          track.muted ? green : accent, line,
                          focus_timeline ? composer_view_start_ms : 0);
        draw_direct_note_events(target, track, y, total_duration, green, line,
                                focus_timeline ? composer_view_start_ms : 0);
        if (is_recording && visual_track.duration_ms > 0) {
            int right = 91 + (int)(((uint64_t)visual_track.duration_ms * 160u) /
                                   (total_duration == 0 ? visual_track.duration_ms : total_duration));
            if (right < 92) right = 92;
            if (right > 251) right = 251;
            fill_rect(target, 91, y + 19, right, y + 27, green);
            for (int x = 91; x <= right; ++x) {
                target[(y + 18) * 256 + x] = line;
                target[(y + 28) * 256 + x] = line;
            }
        }
        if (!focus_timeline && (track.occupied || is_recording) && visual_track.duration_ms > 0) {
            uint32_t phase = is_recording ? now_ms() - record_start
                : (total_duration == 0 ? 0 : transport_ms % total_duration);
            int playhead = 91 + (int)((phase * 160u) / visual_track.duration_ms);
            if (playhead > 251) playhead = 251;
            fill_rect(target, playhead, y + 17, playhead + 1, y + 29, green);
        }
    }
    fill_rect(target, 4, 142, 251, 172, panel);
        int playhead = composer_view_x_for_time(transport_ms);
        fill_rect(target, playhead - 1, COMPOSER_RULER_TOP, playhead + 1,
                  COMPOSER_TRACK_TOP + TRACKS * COMPOSER_TRACK_ROW - 2, green);
        fill_rect(target, playhead - 5, COMPOSER_RULER_TOP - 2,
                  playhead + 5, COMPOSER_RULER_BOTTOM, green);
    }
    if (focus_timeline) {
    draw_text_bitmap(target, 9, 146, "DIARY", text, 1);
    char diary_text[96] = {};
    int start = journal_count > 10 ? journal_count - 10 : 0;
    int cursor = 0;
    if (start > 0) cursor += snprintf(diary_text + cursor, sizeof(diary_text) - cursor, "... ");
    for (int i = start; i < journal_count && cursor < (int)sizeof(diary_text) - 8; ++i) {
        if (i > start) cursor += snprintf(diary_text + cursor, sizeof(diary_text) - cursor, " ");
        cursor += snprintf(diary_text + cursor, sizeof(diary_text) - cursor, "%d", (int)journal_values[i]);
    }
    if (journal_count == 0) snprintf(diary_text, sizeof(diary_text), "--");
    draw_text_bitmap(target, 37, 146, diary_text,
                     now_ms() < journal_flash_until ? green : text, 1);
    if (now_ms() < note_rejection_until)
        draw_text_bitmap(target, 9, 176, note_rejection_text, green, 1);
    if (b_hold_clear && b_action == 0) {
        int progress = (int)(((now_ms() - b_hold_started) * 80u) / HOLD_MS);
        if (progress > 80) progress = 80;
        draw_text_bitmap(target, 9, 162, "CLEAR", text, 1);
        fill_rect(target, 34, 163, 34 + progress, 167, green);
        fill_rect(target, 34 + progress, 163, 114, 167, line);
    }
    fill_rect(target, 4, 177, 251, 188, panel);
    draw_text_bitmap(target, 9, 180, "OCT", text, 1);
    char octave_text[2] = {(char)('0' + octave), '\0'};
    draw_text_bitmap(target, 25, 180, octave_text, text, 1);
    draw_text_bitmap(target, 39, 180, TIMBRE_NAMES[timbre], text, 1);
    draw_text_bitmap(target, 82, 180, vibrato_depth_level == 0 ? "VIB 0" : "VIB ON", vibrato_depth_level > 0 ? green : text, 1);
    char live_text[32];
    if (live_note >= 0) snprintf(live_text, sizeof(live_text), "LIVE %d %dHZ%s", live_note,
                                 note_frequency(live_note), vibrato_active ? " V" : "");
    else snprintf(live_text, sizeof(live_text), "LIVE --");
    char loop_text[16];
    snprintf(loop_text, sizeof(loop_text), loop_enabled ? "LOOP ON" : "LOOP OFF");
    draw_text_bitmap(target, 103, 180, loop_text, text, 1);
    draw_text_bitmap(target, 167, 180, live_text, text, 1);
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
    draw_text_bitmap(top_bitmap, 178, 9, transport_finished ? "STOP" : (paused ? "PAUSED" : (recording ? "REC" : (recording_armed ? (waiting_for_note ? "WAIT" : "ARM") : "RUN"))),
                     recording || recording_armed ? green : text, 1);

    if (recording && recording_target != nullptr) {
        recording_target->occupied = true;
        recording_target->duration_ms = (uint32_t)(now_ms() - record_start);
        if (recording_target->duration_ms == 0) recording_target->duration_ms = 1;
    }

    if (focus_timeline) {
        fill_rect(top_bitmap, 4, 30, 251, 188, panel);
        draw_text_bitmap(top_bitmap, 10, 35, "KEYBOARD / PHRASE", text, 1);
        for (int i = 0; i < KEY_COUNT; ++i) {
            int left = (i * 240) / KEY_COUNT + 8;
            int right = ((i + 1) * 240) / KEY_COUNT + 8;
            fill_rect(top_bitmap, left, 55, right - 1, 178,
                      i == playback_key ? green : ((i == 1 || i == 3 || i == 6 || i == 8 || i == 10) ? line : panel_alt));
            draw_text_bitmap(top_bitmap, left + 7, 84, i < 10 ? (i == 0 ? "0" : i == 1 ? "1" : i == 2 ? "2" : i == 3 ? "3" : i == 4 ? "4" : i == 5 ? "5" : i == 6 ? "6" : i == 7 ? "7" : i == 8 ? "8" : "9") : (i == 10 ? "10" : "11"), text, 1);
        }
        draw_text_bitmap(top_bitmap, 10, 180, "SELECT CLOSE", green, 1);
        dmaCopy(top_framebuffer, top_vram, 256 * 192 * sizeof(u16));
        return;
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
        draw_text_bitmap(top_bitmap, 10, 89, "Y LOOP", text, 1);
        draw_text_bitmap(top_bitmap, 10, 98, "Y LOOP ON OFF", text, 1);
        draw_text_bitmap(top_bitmap, 10, 107, "FX PULSE VIB SUS", text, 1);
        draw_text_bitmap(top_bitmap, 10, 116, "R CLICK ON OFF", text, 1);
        draw_text_bitmap(top_bitmap, 10, 125, "VIB 0 OFF L SUS", text, 1);
        draw_text_bitmap(top_bitmap, 10, 134, "START PLAY/PAUSE", text, 1);
        draw_text_bitmap(top_bitmap, 10, 143, "SEL MUTE HOLD PHRASE", text, 1);
        draw_text_bitmap(top_bitmap, 10, 152, "L+B UNDO R+B CLEAR", text, 1);
        draw_text_bitmap(top_bitmap, 10, 163, "NUMBERS ONLY", green, 1);
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
        draw_phrase_boxes(top_bitmap, track, y, total_duration, 91, 160, 19, 27,
                          track.muted ? green : accent, line);
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
        cursor += snprintf(diary_text + cursor, sizeof(diary_text) - cursor, "%d", (int)journal_values[i]);
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
    draw_text_bitmap(top_bitmap, 82, 180, vibrato_depth_level == 0 ? "VIB 0" : "VIB ON", vibrato_depth_level > 0 ? green : text, 1);
    char live_text[32];
    if (live_note >= 0) {
        snprintf(live_text, sizeof(live_text), "LIVE %d %dHZ%s", live_note,
                 note_frequency(live_note), vibrato_active ? " V" : "");
    } else {
        snprintf(live_text, sizeof(live_text), "LIVE --");
    }
    char loop_text[16];
    snprintf(loop_text, sizeof(loop_text), loop_enabled ? "LOOP ON" : "LOOP OFF");
    draw_text_bitmap(top_bitmap, 103, 180, loop_text, text, 1);
    draw_text_bitmap(top_bitmap, 167, 180, live_text, text, 1);
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

static void copy_bottom_rect_now(int left, int right, int top, int bottom) {
    if (left < 0) left = 0;
    if (right > 255) right = 255;
    if (top < 0) top = 0;
    if (bottom > 191) bottom = 191;
    if (left > right || top > bottom) return;
    for (int y = top; y <= bottom; ++y) {
        dmaCopy(&bottom_framebuffer[y * 256 + left],
                &bottom_vram[y * 256 + left],
                (right - left + 1) * sizeof(u16));
    }
}

static void request_trim_full_present() {
    trim_full_present_pending = true;
    trim_rect_left = 256;
    trim_rect_right = -1;
    trim_rect_top = 192;
    trim_rect_bottom = -1;
}

static void request_trim_rect_present(int left, int right, int top, int bottom) {
    if (trim_full_present_pending) return;
    if (left < trim_rect_left) trim_rect_left = left;
    if (right > trim_rect_right) trim_rect_right = right;
    if (top < trim_rect_top) trim_rect_top = top;
    if (bottom > trim_rect_bottom) trim_rect_bottom = bottom;
}

static void flush_trim_present() {
    if (trim_full_present_pending) {
        present_bottom_full();
        trim_full_present_pending = false;
    } else if (trim_rect_left <= trim_rect_right && trim_rect_top <= trim_rect_bottom) {
        copy_bottom_rect_now(trim_rect_left, trim_rect_right, trim_rect_top, trim_rect_bottom);
    }
    trim_rect_left = 256;
    trim_rect_right = -1;
    trim_rect_top = 192;
    trim_rect_bottom = -1;
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

static void paint_bottom_band(int index, bool active, bool playback = false) {
    if (index < 0 || index >= KEY_COUNT) return;
    const u16 opaque = BIT(15);
    const u16 natural = opaque | RGB15(5, 8, 14);
    const u16 sharp = opaque | RGB15(3, 4, 8);
    const u16 separator = opaque | RGB15(14, 18, 24);
    const u16 active_color = playback ? opaque | RGB15(20, 31, 22) : opaque | RGB15(20, 25, 31);
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

static void draw_bottom_full(int key, bool playback) {
    // 16-bit bitmap backgrounds use bit 15 as the opaque flag.
    const u16 opaque = BIT(15);
    const u16 background = opaque | RGB15(1, 1, 2);
    const u16 separator = opaque | RGB15(14, 18, 24);
    const u16 label = opaque | RGB15(31, 31, 31);
    const u16 label_panel = opaque | RGB15(11, 16, 23);
    const u16 green = opaque | RGB15(20, 31, 22);
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

    for (int i = 0; i < KEY_COUNT; ++i) paint_bottom_band(i, i == key, playback && i == key);
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
    fill_rect(bottom_bitmap, 1, 161, 82, 190, panel);
    fill_rect(bottom_bitmap, 85, 161, 166, 190, panel);
    fill_rect(bottom_bitmap, 169, 161, 254, 190, panel);
    for (int x = 0; x < 256; ++x) bottom_bitmap[160 * 256 + x] = line;
    for (int y = 161; y <= 190; ++y) {
        bottom_bitmap[y * 256 + 83] = line;
        bottom_bitmap[y * 256 + 167] = line;
    }
    char pulse_text[24];
    snprintf(pulse_text, sizeof(pulse_text), "P%lums %u/4 %s", (unsigned long)pulse_ms,
             (unsigned)meter_beats,
             metronome_enabled ? "CLK" : "--");
    draw_text_bitmap(bottom_bitmap, 5, 163, pulse_text, green, 1);
    char vibrato_text[20];
    snprintf(vibrato_text, sizeof(vibrato_text), "VIB %u", (unsigned)vibrato_depth_level);
    draw_text_bitmap(bottom_bitmap, 88, 163, vibrato_text, vibrato_depth_level > 0 ? green : label, 1);
    char sustain_text[20];
    snprintf(sustain_text, sizeof(sustain_text), "SUS %u%s", (unsigned)(sustain_release_level + 1),
             sustain_active ? " L" : "");
    draw_text_bitmap(bottom_bitmap, 172, 163, sustain_text, sustain_active ? green : label, 1);
    for (int x = 8; x <= 76; ++x) bottom_bitmap[178 * 256 + x] = line;
    for (int x = 92; x <= 160; ++x) bottom_bitmap[178 * 256 + x] = line;
    for (int x = 176; x <= 248; ++x) bottom_bitmap[178 * 256 + x] = line;
    for (int i = 0; i <= 14; ++i) {
        int x = 8 + (68 * i) / 14;
        for (int y = 176; y <= 180; ++y) bottom_bitmap[y * 256 + x] = (i * PULSE_STEP_MS + MIN_PULSE_MS <= pulse_ms) ? green : label;
    }
    for (int i = 0; i <= 10; ++i) {
        int x = 92 + (68 * i) / 10;
        for (int y = 176; y <= 180; ++y) bottom_bitmap[y * 256 + x] = i <= vibrato_depth_level ? green : label;
    }
    for (int i = 0; i <= 9; ++i) {
        int x = 176 + (72 * i) / 9;
        for (int y = 176; y <= 180; ++y) bottom_bitmap[y * 256 + x] = i <= sustain_release_level ? green : label;
    }
    int pulse_knob = 8 + (68 * (int)((pulse_ms - MIN_PULSE_MS) / PULSE_STEP_MS)) / 14;
    int vibrato_knob = 92 + (68 * vibrato_depth_level) / 10;
    int sustain_knob = 176 + (72 * sustain_release_level) / 9;
    for (int x = pulse_knob - 2; x <= pulse_knob + 2; ++x)
        for (int y = 175; y <= 181; ++y) bottom_bitmap[y * 256 + x] = green;
    for (int x = vibrato_knob - 2; x <= vibrato_knob + 2; ++x)
        for (int y = 175; y <= 181; ++y) bottom_bitmap[y * 256 + x] = green;
    for (int x = sustain_knob - 2; x <= sustain_knob + 2; ++x)
        for (int y = 175; y <= 181; ++y) bottom_bitmap[y * 256 + x] = green;
    rendered_bottom_key = key;
    rendered_bottom_playback = playback;
    rendered_octave = octave;
    rendered_track = selected_track;
    rendered_timbre = timbre;
    rendered_pulse_ms = pulse_ms;
    rendered_vibrato_depth_level = vibrato_depth_level;
    rendered_sustain_release_level = sustain_release_level;
    rendered_sustain_active = sustain_active;
    bottom_needs_redraw = false;
    request_trim_full_present();
}

static void draw_trim_editor() {
    const u16 opaque = BIT(15);
    const u16 background = opaque | RGB15(1, 2, 4);
    const u16 panel = opaque | RGB15(7, 12, 19);
    const u16 line = opaque | RGB15(10, 17, 26);
    const u16 text = opaque | RGB15(31, 31, 31);
    const u16 block = opaque | RGB15(20, 25, 31);
    const u16 green = opaque | RGB15(20, 31, 22);
    for (int y = 0; y < 192; ++y)
        for (int x = 0; x < 256; ++x) bottom_bitmap[y * 256 + x] = background;
    fill_rect(bottom_bitmap, 3, 2, 252, 24, panel);
    draw_text_bitmap(bottom_bitmap, 8, 7, note_insert_active ? "INSERT NOTE" : "EDIT TAKE", green, 1);
    draw_text_bitmap(bottom_bitmap, 82, 7, note_insert_active ? "PROVISIONAL" : "TOUCH NOTE", text, 1);
    draw_text_bitmap(bottom_bitmap, 167, 7, note_insert_active ? "MOVE LENGTH" : "START END MOVE", text, 1);
    if (trim_raw_duration_ms == 0) trim_raw_duration_ms = 1;
    for (uint32_t step = 0; step <= (trim_raw_duration_ms * 4u) / pulse_ms + 1u; ++step) {
        uint32_t value = pulse_grid().time(step);
        if (value > trim_raw_duration_ms) break;
        int x = timeline_x_for_ms(value, trim_raw_duration_ms);
        u16 color = (step % 4u == 0) ? line : panel;
        for (int y = 30; y < 166; ++y) bottom_bitmap[y * 256 + x] = color;
    }
    for (uint16_t i = 0; i < record_count; ++i) {
        const NoteEvent& event = recording_track.events[i];
        int left = timeline_x_for_ms(event.start_ms, trim_raw_duration_ms);
        int right = timeline_x_for_ms(event.end_ms, trim_raw_duration_ms);
        if (right <= left) right = left + 1;
        u16 color = (int)i == trim_selected_event ? green : block;
        int fill_right = right > left + 1 ? right - 1 : right;
        fill_rect(bottom_bitmap, left + (right > left + 1 ? 1 : 0), 72, fill_right, 108, color);
        for (int x = left; x <= right; ++x) {
            bottom_bitmap[71 * 256 + x] = line;
            bottom_bitmap[109 * 256 + x] = line;
        }
        bottom_bitmap[84 * 256 + left] = green;
        bottom_bitmap[96 * 256 + right] = green;
        if (event.sustain_on_release)
            draw_text_bitmap(bottom_bitmap, left < 4 ? 4 : left - 1, 57, "S", green, 1);
    }
    if (!note_insert_active) {
        int start_x = timeline_x_for_ms(trim_start_ms, trim_raw_duration_ms);
        int end_x = timeline_x_for_ms(trim_end_ms, trim_raw_duration_ms);
        fill_rect(bottom_bitmap, start_x, 40, start_x, 145, green);
        fill_rect(bottom_bitmap, end_x, 40, end_x, 145, green);
        draw_text_bitmap(bottom_bitmap, start_x < 4 ? 4 : start_x - 2, 43, "S", green, 1);
        draw_text_bitmap(bottom_bitmap, end_x < 4 ? 4 : end_x - 2, 43, "E", green, 1);
    }
    memcpy(trim_static_framebuffer, bottom_framebuffer, 256 * 192 * sizeof(u16));
    trim_playhead_x = -1;
    if (trim_preview_playing && trim_preview_track.duration_ms > 0) {
        uint32_t phase = trim_preview_phase_ms();
        int playhead_x = timeline_x_for_ms(trim_start_ms + phase, trim_raw_duration_ms);
        fill_rect(bottom_bitmap, playhead_x - 1, 35, playhead_x + 1, 150, green);
        trim_playhead_x = playhead_x;
    }
    draw_text_bitmap(bottom_bitmap, 8, 151, "START PREVIEW", text, 1);
    draw_text_bitmap(bottom_bitmap, 94, 151, "A ACCEPT", green, 1);
    draw_text_bitmap(bottom_bitmap, 154, 151, "B CANCEL", text, 1);
    char repeat_text[16];
    snprintf(repeat_text, sizeof(repeat_text), "REPEAT %u", (unsigned)trim_repeat_count);
    draw_text_bitmap(bottom_bitmap, 8, 181, repeat_text, text, 1);
    draw_text_bitmap(bottom_bitmap, 182, 181, "L SUS", green, 1);
    trim_editor_dirty = false;
    present_bottom_full();
}

static void update_trim_playhead() {
    if (!trim_preview_playing || trim_preview_track.duration_ms == 0) return;
    const u16 green = BIT(15) | RGB15(20, 31, 22);
    uint32_t phase = trim_preview_phase_ms();
    int next_x = timeline_x_for_ms(trim_start_ms + phase, trim_raw_duration_ms);
    if (next_x == trim_playhead_x) return;
    int left = (trim_playhead_x < 0 ? next_x : (trim_playhead_x < next_x ? trim_playhead_x : next_x)) - 1;
    int right = (trim_playhead_x < next_x ? next_x : trim_playhead_x) + 1;
    if (trim_playhead_x >= 0) {
        for (int y = 35; y <= 150; ++y)
            for (int x = trim_playhead_x - 1; x <= trim_playhead_x + 1; ++x)
                if (x >= 0 && x < 256) bottom_framebuffer[y * 256 + x] = trim_static_framebuffer[y * 256 + x];
    }
    for (int y = 35; y <= 150; ++y)
        for (int x = next_x - 1; x <= next_x + 1; ++x)
            if (x >= 0 && x < 256) bottom_framebuffer[y * 256 + x] = green;
    trim_playhead_x = next_x;
    request_trim_rect_present(left, right, 35, 150);
}

static void draw_bottom(int key) {
    if (trim_active) {
        if (trim_editor_dirty) draw_trim_editor();
        else update_trim_playhead();
        return;
    }
    if (focus_timeline) {
        draw_phrase_picker();
        return;
    }
    int desired_key = key >= 0 ? key : playback_key;
    bool desired_playback = key < 0 && playback_key >= 0;
    if (bottom_needs_redraw || rendered_octave != octave || rendered_track != selected_track ||
        rendered_timbre != timbre || rendered_pulse_ms != pulse_ms || rendered_bottom_playback != desired_playback ||
        rendered_vibrato_depth_level != vibrato_depth_level ||
        rendered_sustain_release_level != sustain_release_level || rendered_sustain_active != sustain_active) {
        draw_bottom_full(desired_key, desired_playback);
        return;
    }
    if (desired_key == rendered_bottom_key) return;
    int previous_key = rendered_bottom_key;
    paint_bottom_band(previous_key, false, rendered_bottom_playback);
    paint_bottom_band(desired_key, true, desired_playback);
    rendered_bottom_key = desired_key;
    rendered_bottom_playback = desired_playback;
    present_bottom_band(previous_key);
    present_bottom_band(desired_key);
}

int main() {
#ifdef DS_DEBUG_TRACE
    // Route stderr/DS_TRACE to NO$GBA's NOCASH/TTY debug window.
    consoleDebugInit(DebugDevice_NOCASH);
    DS_TRACE("DSDBG|BOOT|trace_ready\n");
#endif
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
    systemCounterSetup();
    advance_master_clock();
    touchPosition touch = {};
    previous_ms = now_ms();
#ifdef __BLOCKSDS__
    while (1) {
#else
    while (pmMainLoop()) {
#endif
        swiWaitForVBlank();
        flush_trim_present();
        scanKeys();
        advance_master_clock();
        uint32_t now = now_ms();
        uint32_t delta = now - previous_ms;
        previous_ms = now;
        if (!paused) {
            transport_ms += delta;
            uint32_t window_start = 0;
            uint32_t window_end = 0;
            playback_window(window_start, window_end);
            if (window_end > window_start && transport_ms >= window_end) {
                bool stopped = false;
                uint32_t previous_transport = transport_ms - delta;
                transport_ms = loop_logic::advance_transport(previous_transport, delta,
                                                              window_start, window_end,
                                                              loop_enabled, stopped);
                timeline_selection.cursor_ms = transport_ms;
                if (stopped) {
                    paused = true;
                    transport_finished = true;
                    stop_all_voices();
                    DS_TRACE("DSDBG|STOP|transport=%lu|start=%lu|end=%lu\n",
                             (unsigned long)transport_ms, (unsigned long)window_start,
                             (unsigned long)window_end);
                } else {
                    reset_loop_playback();
                    DS_TRACE("DSDBG|WRAP|transport=%lu|start=%lu|end=%lu\n",
                             (unsigned long)transport_ms, (unsigned long)window_start,
                             (unsigned long)window_end);
                }
            }
        }

        int held = keysHeld();
        int pressed = held & ~previous_buttons;
        int released = previous_buttons & ~held;
        if (!trim_active && (pressed & KEY_Y)) {
            loop_button_pending = true;
            loop_button_direction_used = false;
        }
        if (!trim_active && (pressed & KEY_UP) && focus_timeline && (held & KEY_L)) {
            composer_view_ms = loop_logic::adjust_composer_view_ms(composer_view_ms, 1);
            composer_view_start_ms = loop_logic::clamp_viewport_start(
                composer_view_start_ms, composer_content_duration_ms(), composer_view_duration_ms());
            bottom_needs_redraw = true;
            DS_TRACE("DSDBG|VIEW_ZOOM|duration=%lu\n", (unsigned long)composer_view_ms);
        } else if (!trim_active && (pressed & KEY_UP)) {
            if (focus_timeline && composer_selected_note >= 0 &&
                composer_selected_note < tracks[selected_track].event_count) {
                if (loop_logic::adjust_note_pitch(
                        tracks[selected_track].events[composer_selected_note], 1)) {
                    bottom_needs_redraw = true;
                    DS_TRACE("DSDBG|NOTE_PITCH|track=%d|event=%d|midi=%d\n",
                             selected_track, composer_selected_note,
                             tracks[selected_track].events[composer_selected_note].midi_note);
                }
            } else if (octave < 6) ++octave;
        }
        if (!trim_active && (pressed & KEY_DOWN) && focus_timeline && (held & KEY_L)) {
            composer_view_ms = loop_logic::adjust_composer_view_ms(composer_view_ms, -1);
            composer_view_start_ms = loop_logic::clamp_viewport_start(
                composer_view_start_ms, composer_scroll_content_duration_ms(), composer_view_duration_ms());
            bottom_needs_redraw = true;
            DS_TRACE("DSDBG|VIEW_ZOOM|duration=%lu\n", (unsigned long)composer_view_ms);
        } else if (!trim_active && (pressed & KEY_DOWN)) {
            if (focus_timeline && composer_selected_note >= 0 &&
                composer_selected_note < tracks[selected_track].event_count) {
                if (loop_logic::adjust_note_pitch(
                        tracks[selected_track].events[composer_selected_note], -1)) {
                    bottom_needs_redraw = true;
                    DS_TRACE("DSDBG|NOTE_PITCH|track=%d|event=%d|midi=%d\n",
                             selected_track, composer_selected_note,
                             tracks[selected_track].events[composer_selected_note].midi_note);
                }
            } else if (octave > 2) --octave;
        }
        if (trim_active && (pressed & KEY_UP) && trim_repeat_count < 99) ++trim_repeat_count;
        if (trim_active && (pressed & KEY_DOWN) && trim_repeat_count > 1) --trim_repeat_count;
        if (!trim_active && !(held & KEY_TOUCH) && !(held & KEY_L)) {
            if (pressed & KEY_LEFT) {
                selected_track = loop_logic::wrap_track(selected_track, -1);
                if (focus_timeline) phrase_picker_selected = -1;
                if (held & KEY_Y) loop_button_direction_used = true;
            }
            if (pressed & KEY_RIGHT) {
                selected_track = loop_logic::wrap_track(selected_track, 1);
                if (focus_timeline) phrase_picker_selected = -1;
                if (held & KEY_Y) loop_button_direction_used = true;
            }
        }
        if (pressed & KEY_START) {
            if (focus_timeline) {
                // Phrase picker is a structural screen; transport stays unchanged.
            } else if (trim_active) {
                reset_trim_preview_audio();
                trim_preview_playing = !trim_preview_playing;
                trim_preview_started_ms = now_ms();
                trim_editor_dirty = true;
            } else {
                if (paused && transport_finished) {
                    uint32_t window_start = 0;
                    uint32_t window_end = 0;
                    playback_window(window_start, window_end);
                    transport_ms = window_start;
                }
                paused = !paused;
                if (paused) update_loops();
                else transport_finished = false;
            }
        }
        if (!trim_active && (pressed & KEY_SELECT) && !select_pending) {
            select_hold_started = now;
            select_pending = true;
            select_help_active = false;
        }
        if (select_pending && now - select_hold_started >= HOLD_MS && !select_help_active) {
            select_help_active = true;
            focus_timeline = !focus_timeline;
            phrase_picker_selected = -1;
            bottom_needs_redraw = true;
            DS_TRACE("DSDBG|PHRASE_PICKER|enabled=%d\n", focus_timeline ? 1 : 0);
        }
        if ((released & KEY_SELECT) && select_pending) {
            if (!select_help_active) tracks[selected_track].muted = !tracks[selected_track].muted;
            select_pending = false;
            select_help_active = false;
        }
        if (!trim_active && (pressed & KEY_X)) { timbre = (timbre + 1) % INSTRUMENTS; refresh_live_timbre(); }
        if ((pressed & KEY_R) && !(held & KEY_B)) {
            metronome_enabled = !metronome_enabled;
            if (!metronome_enabled) stop_metronome_voice();
            bottom_needs_redraw = true;
        }
        sustain_active = (held & KEY_L) != 0;
        if (trim_active && (pressed & KEY_L) && trim_selected_event >= 0) {
            NoteEvent& event = recording_track.events[trim_selected_event];
            if (event.sustain_on_release) {
                event.sustain_on_release = 0;
                event.sustain_level = 0;
            } else {
                event.sustain_on_release = 1;
                event.sustain_level = sustain_release_level;
            }
            build_trim_preview();
            reset_trim_preview_audio();
            bottom_needs_redraw = true;
            DS_TRACE("DSDBG|NOTE_SUSTAIN|event=%d|enabled=%d|level=%u\n",
                     trim_selected_event, event.sustain_on_release ? 1 : 0,
                     (unsigned)event.sustain_level);
        }
        if (loop_logic::should_toggle_loop_on_release(
                loop_button_pending, loop_button_direction_used, (released & KEY_Y) != 0)) {
            loop_enabled = !loop_enabled;
            loop_button_pending = false;
            loop_button_direction_used = false;
        } else if ((released & KEY_Y) != 0) {
            loop_button_pending = false;
            loop_button_direction_used = false;
        }
        if (pressed & KEY_A) {
            if (trim_active) accept_trim();
            else if (recording) stop_recording(now);
            else if (!focus_timeline) start_recording(now);
        }
        if (trim_active && (pressed & KEY_B)) cancel_trim();
        if (focus_timeline && (pressed & KEY_B)) {
            focus_timeline = false;
            phrase_picker_selected = -1;
            bottom_needs_redraw = true;
        } else if (!trim_active && !focus_timeline && (pressed & KEY_B) && !b_hold_clear) {
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
            if ((pressed & KEY_TOUCH) && touch.py < 70 && !note_insert_active) {
                int start_x = timeline_x_for_ms(trim_start_ms, trim_raw_duration_ms);
                int end_x = timeline_x_for_ms(trim_end_ms, trim_raw_duration_ms);
                if (abs((int)touch.px - start_x) <= 12)
                    trim_drag_handle = 1;
                else if (abs((int)touch.px - end_x) <= 12)
                    trim_drag_handle = 2;
            }
            if ((pressed & KEY_TOUCH) && touch.py >= 60 && touch.py < 145) {
                uint32_t raw = timeline_ms_for_x(touch.px, trim_raw_duration_ms);
                trim_selected_event = -1;
                for (uint16_t i = 0; i < record_count; ++i) {
                    const NoteEvent& event = recording_track.events[i];
                    int start_x = timeline_x_for_ms(event.start_ms, trim_raw_duration_ms);
                    int end_x = timeline_x_for_ms(event.end_ms, trim_raw_duration_ms);
                    if ((int)touch.px < start_x || (int)touch.px > end_x) continue;
                    trim_selected_event = i;
                    int width = end_x - start_x;
                    int edge_hit = width >= 18 ? 8 : (width >= 6 ? 2 : 0);
                    bool at_start_edge = edge_hit > 0
                        ? (int)touch.px <= start_x + edge_hit
                        : (int)touch.px == start_x;
                    bool at_end_edge = edge_hit > 0
                        ? (int)touch.px >= end_x - edge_hit
                        : (int)touch.px == end_x;
                    if (at_start_edge && !at_end_edge)
                        trim_edit_mode = loop_logic::NOTE_START;
                    else if (at_end_edge && !at_start_edge)
                        trim_edit_mode = loop_logic::NOTE_END;
                    else
                        trim_edit_mode = loop_logic::NOTE_MOVE;
                    trim_touch_editing = true;
                    trim_touch_origin_ms = raw;
                    trim_edit_original_start_ms = event.start_ms;
                    trim_edit_original_end_ms = event.end_ms;
                    trim_touch_free_changed = false;
                    DS_TRACE("DSDBG|NOTE_EDIT|event=%d|mode=%d|select=%lu\n",
                             trim_selected_event, (int)trim_edit_mode, (unsigned long)raw);
                    break;
                }
            }
            if ((held & KEY_TOUCH) && trim_touch_editing && trim_selected_event >= 0 && touch.py < 160) {
                uint32_t raw = timeline_ms_for_x(touch.px, trim_raw_duration_ms);
                uint32_t value = trim_edit_mode == loop_logic::NOTE_END
                    ? dragged_note_free_request(trim_edit_original_end_ms, trim_touch_origin_ms, raw)
                    : dragged_note_free_request(trim_edit_original_start_ms, trim_touch_origin_ms, raw);
                bool changed = loop_logic::edit_note_event(
                    recording_track, (uint16_t)trim_selected_event, trim_edit_mode,
                    value, trim_start_ms, trim_end_ms, pulse_ms, false);
                if (changed) {
                    trim_touch_free_changed = true;
                    build_trim_preview();
                    DS_TRACE("DSDBG|NOTE_EDIT|event=%d|mode=%d|value=%lu|start=%lu|end=%lu\n",
                             trim_selected_event, (int)trim_edit_mode, (unsigned long)value,
                             (unsigned long)recording_track.events[trim_selected_event].start_ms,
                             (unsigned long)recording_track.events[trim_selected_event].end_ms);
                }
            }
            if ((held & KEY_TOUCH) && trim_drag_handle != 0 && touch.py < 160) {
                uint32_t value = timeline_ms_for_x(touch.px, trim_raw_duration_ms);
                if (trim_drag_handle == 1) {
                    if (value >= trim_end_ms) value = trim_end_ms > 0 ? trim_end_ms - 1 : 0;
                    trim_start_ms = value;
                } else {
                    if (value <= trim_start_ms) value = trim_start_ms + 1;
                    trim_end_ms = value > trim_raw_duration_ms ? trim_raw_duration_ms : value;
                }
                build_trim_preview();
                if (trim_preview_playing) {
                    trim_preview_started_ms = now_ms();
                    trim_preview_phase_valid = false;
                    reset_trim_preview_audio();
                }
            }
            if (!(held & KEY_TOUCH)) {
                if (trim_drag_handle != 0) {
                    trim_start_ms = loop_logic::snap_loop_boundary(trim_start_ms, pulse_ms);
                    trim_end_ms = loop_logic::snap_loop_boundary(trim_end_ms, pulse_ms);
                    if (trim_start_ms >= trim_raw_duration_ms)
                        trim_start_ms = trim_raw_duration_ms > pulse_ms ? trim_raw_duration_ms - pulse_ms : 0;
                    if (trim_end_ms <= trim_start_ms)
                        trim_end_ms = trim_start_ms + pulse_ms;
                    if (trim_end_ms > trim_raw_duration_ms) trim_end_ms = trim_raw_duration_ms;
                    build_trim_preview();
                }
                if (trim_touch_editing && trim_touch_free_changed && trim_selected_event >= 0) {
                    NoteEvent& event = recording_track.events[trim_selected_event];
                    uint32_t value = trim_edit_mode == loop_logic::NOTE_END
                        ? event.end_ms : event.start_ms;
                    bool snapped = loop_logic::edit_note_event(
                        recording_track, (uint16_t)trim_selected_event, trim_edit_mode,
                        value, trim_start_ms, trim_end_ms, pulse_ms, true);
                    if (snapped) build_trim_preview();
                }
                trim_drag_handle = 0;
                trim_touch_editing = false;
                trim_touch_free_changed = false;
            }
        } else if (focus_timeline && (pressed & KEY_TOUCH)) {
            if (!loop_logic::timeline_x_is_valid(touch.px)) {
                show_note_rejection("TIMELINE ONLY");
            } else {
                int phrase_index = phrase_picker_hit(touch.px, touch.py);
                bool selected_row = touch.py >= 30 + selected_track * 37 &&
                    touch.py < 30 + (selected_track + 1) * 37;
                if (phrase_index >= 0) {
                    phrase_picker_selected = phrase_index;
                    open_phrase_editor(selected_track, phrase_index);
                } else if (selected_row) {
                    int direct_index = direct_note_hit(touch.px, touch.py);
                    if (direct_index >= 0) {
                        open_direct_note_editor((uint16_t)direct_index);
                    } else {
                        uint32_t duration = song_duration_ms();
                        if (duration == 0) duration = pulse_ms;
                        start_note_insertion(loop_logic::timeline_ms_for_x(touch.px, duration));
                    }
                } else {
                    show_note_rejection("SELECTED ROW");
                }
            }
        } else if ((held & KEY_TOUCH) && touch.py >= TOUCH_TIMELINE_TOP && touch.py < 192) {
            if (touch.px < 84 && touch.py < 175 && (pressed & KEY_TOUCH)) {
                meter_beats = meter_beats == 4 ? 3 : 4;
                bottom_needs_redraw = true;
                DS_TRACE("DSDBG|METER|beats=%u\n", (unsigned)meter_beats);
            } else if (touch.py >= 175 && touch.px < 84) {
                int x = touch.px < 8 ? 8 : (touch.px > 76 ? 76 : touch.px);
                uint32_t level = (uint32_t)(((x - 8) * 14 + 34) / 68);
                pulse_ms = MIN_PULSE_MS + level * PULSE_STEP_MS;
                if (pulse_ms > MAX_PULSE_MS) pulse_ms = MAX_PULSE_MS;
                bottom_needs_redraw = true;
                DS_TRACE("DSDBG|GRID|pulse=%lu\n", (unsigned long)pulse_ms);
            } else if (touch.px < 168) {
                int x = touch.px < 92 ? 92 : (touch.px > 160 ? 160 : touch.px);
                vibrato_depth_level = (uint8_t)(((x - 92) * 10 + 34) / 68);
                if (vibrato_depth_level > 10) vibrato_depth_level = 10;
        } else if (focus_timeline && composer_keyboard_dragging &&
                   (held & KEY_TOUCH)) {
            if (touch.py >= 151 && touch.py < 192) {
                stable_touch_key = composer_drag_key = loop_logic::key_for_x(touch.px);
                key = stable_touch_key;
            } else if (touch.py >= COMPOSER_TRACK_TOP &&
                       touch.py < COMPOSER_TRACK_TOP + TRACKS * COMPOSER_TRACK_ROW &&
                       loop_logic::timeline_x_is_valid(touch.px)) {
                composer_drag_track = (touch.py - COMPOSER_TRACK_TOP) / COMPOSER_TRACK_ROW;
                if (touch.py >= COMPOSER_TRACK_TOP + composer_drag_track * COMPOSER_TRACK_ROW + COMPOSER_TRACK_HEIGHT)
                    composer_drag_track = -1;
                if (composer_drag_track >= 0) {
                    selected_track = composer_drag_track;
                    composer_drag_drop_ms = loop_logic::viewport_time_for_x(
                        touch.px, composer_view_start_ms, composer_view_duration_ms());
                }
                bottom_needs_redraw = true;
            }
        } else if (focus_timeline && (pressed & KEY_TOUCH) && touch.py >= 151 && touch.py < 192) {
            composer_keyboard_dragging = true;
            composer_drag_key = loop_logic::key_for_x(touch.px);
            composer_drag_track = -1;
            composer_drag_drop_ms = 0;
            stable_touch_key = composer_drag_key;
            key = stable_touch_key;
                if (vibrato_depth_level == 0) vibrato_active = false;
            } else {
                int x = touch.px < 176 ? 176 : (touch.px > 248 ? 248 : touch.px);
                sustain_release_level = (uint8_t)(((x - 176) * 9 + 36) / 72);
                if (sustain_release_level > 9) sustain_release_level = 9;
            }
        } else if ((held & KEY_TOUCH) && touch.py < TOUCH_TIMELINE_TOP) {
            // Accept zone changes on the first hardware sample. Requiring two
            // consecutive 60 Hz samples loses fast notes on real DS.
            stable_touch_key = loop_logic::key_for_x(touch.px);
            key = stable_touch_key;
        } else {
            stable_touch_key = -1;
        }
        if (key >= 0) begin_note(key, now);
        else if (!(held & KEY_TOUCH)) end_live_note(now);
        update_live_effects(now, held);
        update_live_tails(now);
        update_loops();
        update_metronome(now);
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
            Track& target = tracks[selected_track];
            if (composer_selected_note >= 0 &&
                composer_selected_note < target.event_count &&
                loop_logic::snap_released_note_event(
                    target, (uint16_t)composer_selected_note, composer_edit_mode,
                    COMPOSER_CONTENT_LIMIT_MS, pulse_ms)) {
                bottom_needs_redraw = true;
                DS_TRACE("DSDBG|NOTE_SNAP_RELEASE|track=%d|event=%d|start=%lu|end=%lu\n",
                         selected_track, composer_selected_note,
                         (unsigned long)target.events[composer_selected_note].start_ms,
                         (unsigned long)target.events[composer_selected_note].end_ms);
            }
