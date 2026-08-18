"""Generate the CC0-equivalent, offline-synthesized DS PCM sample set."""

from __future__ import annotations

import math
import random
import struct
from pathlib import Path

RATE = 22050
ROOTS = (48, 60, 72, 84)
OUT = Path(__file__).parents[1] / "assets" / "samples"


def midi_hz(note: int) -> float:
    return 440.0 * (2.0 ** ((note - 69) / 12.0))


def env(t: float, attack: float, decay: float, sustain: float, release: float, total: float) -> float:
    if t < attack:
        return t / attack
    if t < attack + decay:
        return 1.0 - (1.0 - sustain) * ((t - attack) / decay)
    if t < total - release:
        return sustain
    return sustain * max(0.0, (total - t) / release)


def osc(kind: str, phase: float, rng: random.Random) -> float:
    if kind == "rhodes":
        return 0.72 * math.sin(phase) + 0.18 * math.sin(2 * phase) + 0.08 * math.sin(3 * phase)
    if kind == "kalimba":
        return 0.72 * math.sin(phase) + 0.20 * math.sin(2.01 * phase) + 0.08 * math.sin(3.98 * phase)
    if kind == "bass":
        return 0.82 * math.sin(phase) + 0.14 * math.sin(2 * phase) + 0.04 * math.sin(3 * phase)
    if kind == "pluck":
        return 0.62 * math.sin(phase) + 0.25 * math.sin(2 * phase) + 0.10 * math.sin(3 * phase) + 0.03 * math.sin(5 * phase)
    if kind == "glass":
        return 0.55 * math.sin(phase) + 0.28 * math.sin(2.76 * phase) + 0.14 * math.sin(4.07 * phase)
    return 0.68 * math.sin(phase) + 0.22 * math.sin(0.997 * phase) + 0.10 * math.sin(2 * phase)


def make_sample(kind: str, root: int) -> bytes:
    rng = random.Random(root * 97 + len(kind))
    if kind == "rhodes":
        total, attack, decay, sustain, release = 1.25, 0.018, 0.28, 0.36, 0.20
    elif kind == "kalimba":
        total, attack, decay, sustain, release = 0.72, 0.002, 0.34, 0.10, 0.12
    elif kind == "bass":
        total, attack, decay, sustain, release = 1.05, 0.012, 0.18, 0.54, 0.18
    elif kind == "pluck":
        total, attack, decay, sustain, release = 1.05, 0.006, 0.42, 0.20, 0.18
    elif kind == "glass":
        total, attack, decay, sustain, release = 1.45, 0.003, 0.85, 0.12, 0.28
    else:
        total, attack, decay, sustain, release = 1.80, 0.16, 0.30, 0.48, 0.32
    count = int(total * RATE)
    freq = midi_hz(root)
    data = bytearray()
    phase = 0.0
    for i in range(count):
        t = i / RATE
        phase += 2.0 * math.pi * freq / RATE
        value = osc(kind, phase, rng)
        # A gentle second harmonic keeps low notes intelligible on the DS speakers.
        value += (0.06 if kind == "glass" else 0.10) * math.sin(2.0 * phase)
        if kind == "kalimba":
            value += 0.025 * (rng.random() * 2.0 - 1.0) * math.exp(-t * 18.0)
        if kind == "glass":
            value += 0.035 * math.sin(phase * 7.13) * math.exp(-t * 5.0)
        value *= env(t, attack, decay, sustain, release, total)
        # Root-dependent gain compensates for the quieter low-register roots;
        # the upper roots are intentionally a little quieter to avoid harsh highs.
        root_gain = {48: 1.45, 60: 1.25, 72: 1.05, 84: 0.90}[root]
        value = max(-0.94, min(0.94, value * 0.72 * root_gain))
        data.extend(struct.pack("b", int(value * 127.0)))
    return bytes(data)


def main() -> None:
    OUT.mkdir(parents=True, exist_ok=True)
    for kind in ("rhodes", "kalimba", "bass", "pluck", "glass", "pad"):
        for root in ROOTS:
            (OUT / f"{kind}_{root}.bin").write_bytes(make_sample(kind, root))


if __name__ == "__main__":
    main()
