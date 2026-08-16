from pathlib import Path

source = Path(__file__).parents[1] / "include" / "loop_logic.h"
text = source.read_text(encoding="utf-8")
assert "elapsed_ms % duration_ms" in text
assert "return value < 0 ? value + 3 : value;" in text
assert "event.start_ms <= phase && phase < event.end_ms" in text
print("loop_logic structural tests: OK")

