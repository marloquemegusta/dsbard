from pathlib import Path

source = Path(__file__).parents[1] / "include" / "loop_logic.h"
text = source.read_text(encoding="utf-8")
assert "elapsed_ms % duration_ms" in text
assert "return value < 0 ? value + 4 : value;" in text
assert "event.start_ms <= phase && phase < event.end_ms" in text

main = source.parents[0].parent / "source" / "main.cpp"
main_text = main.read_text(encoding="utf-8")
assert "#include <nds/system_counter.h>" in main_text
assert "systemCounterSetup();" in main_text
assert "advance_master_clock();" in main_text
assert "systemCounterGetTicks()" in main_text
assert "systemCounterTicksToMsec" in main_text
assert "timerStart(0" not in main_text
assert "timerElapsed(0)" not in main_text
assert "gettimeofday" not in main_text
assert "consoleDebugInit(DebugDevice_NOCASH);" in main_text
assert "static constexpr int TRACKS = 4;" in main_text
assert "TRACK_ROW_STEP = 27" in main_text
assert '#include "song_files.h"' in main_text
assert "STARTUP_LIBRARY" in main_text
assert "persist_active_song();" in main_text
assert "trim_preview_started_ms" in main_text
assert "trim_preview_frame_started" not in main_text
assert "uint32_t elapsed_ms = now_ms() - trim_preview_started_ms;" in main_text
assert "snap_loop_duration(now - record_start, pulse_ms)" in main_text
assert "trim_start_ms = 0;" in main_text
assert "snap_loop_boundary(trim_start_ms, pulse_ms)" in main_text
assert "static void flush_trim_present()" in main_text
assert "swiWaitForVBlank();\n        flush_trim_present();" in main_text
assert "request_trim_rect_present(left, right, 35, 150);" in main_text
assert "PITCH_WINDOW_SEMITONES = 24" in text
assert "pitch_lane_y" in text
assert "composer_pitch_window_low" in main_text
assert "COMPOSER_KEYBOARD_ROWS = 2" in main_text
assert "COMPOSER_KEYBOARD_ROW_STEP = 25" in main_text
assert "composer_keyboard_row" in main_text
assert "octave + composer_keyboard_row" in main_text
assert "PITCH_PAN" in main_text
assert 'draw_text_bitmap(target, 9, 9, "LOW", text, 1);' in main_text
assert 'draw_text_bitmap(target, 215, 9, "HIGH", text, 1);' in main_text

build = source.parents[0].parent / "build.ps1"
build_text = build.read_text(encoding="utf-8")
assert "source\\song_files.cpp" in build_text
assert "include\\song_storage.h" in build_text
assert "include\\song_files.h" in build_text
print("loop_logic structural tests: OK")

