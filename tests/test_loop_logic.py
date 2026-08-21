from pathlib import Path

source = Path(__file__).parents[1] / "include" / "loop_logic.h"
text = source.read_text(encoding="utf-8")
assert "elapsed_ms % duration_ms" in text
assert "return value < 0 ? value + 3 : value;" in text
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
assert "trim_preview_started_ms" in main_text
assert "trim_preview_frame_started" not in main_text
assert "uint32_t elapsed_ms = now_ms() - trim_preview_started_ms;" in main_text
print("loop_logic structural tests: OK")

