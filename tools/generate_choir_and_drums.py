"""Generate high-quality CC0-synthesized Choir and 12-piece Drum kit samples for Nintendo DS."""

from __future__ import annotations

import math
import random
import struct
from pathlib import Path

RATE = 22050
OUT = Path(__file__).parents[1] / "assets" / "samples"


def midi_hz(note: int) -> float:
    return 440.0 * (2.0 ** ((note - 69) / 12.0))


def clamp_byte(val: float) -> int:
    return max(-128, min(127, int(val * 127.0)))


def make_choir_sample(root: int) -> tuple[bytes, int]:
    """Synthesizes epic choir with formants: 'Ohhh' for roots 48, 60 and 'Ahhh' for roots 72, 84."""
    freq = midi_hz(root)
    # Roots 48, 60: 'Ohhh' (F1=500Hz, F2=900Hz)
    # Roots 72, 84: 'Ahhh' (F1=800Hz, F2=1250Hz, F3=2600Hz)
    is_oh = (root <= 60)
    total_sec = 1.6
    total_samples = int(total_sec * RATE)
    loop_start_sec = 0.6
    loop_start = int(loop_start_sec * RATE)
    
    # Detuned voices for rich choral unison
    detunes = [-0.003, 0.0, 0.0035, -0.007, 0.006]
    phases = [0.0] * len(detunes)
    
    data = bytearray()
    attack_sec = 0.25
    attack_samples = int(attack_sec * RATE)
    
    # Single-pole formant resonators state
    y1, y2, y3 = 0.0, 0.0, 0.0
    f1 = 500.0 if is_oh else 800.0
    f2 = 900.0 if is_oh else 1250.0
    f3 = 2400.0 if is_oh else 2600.0
    
    w1 = 2.0 * math.pi * f1 / RATE
    w2 = 2.0 * math.pi * f2 / RATE
    w3 = 2.0 * math.pi * f3 / RATE
    r = 0.95  # resonance factor
    
    y1_prev1, y1_prev2 = 0.0, 0.0
    y2_prev1, y2_prev2 = 0.0, 0.0
    y3_prev1, y3_prev2 = 0.0, 0.0
    
    a1_1 = 2.0 * r * math.cos(w1)
    a1_2 = -r * r
    a2_1 = 2.0 * r * math.cos(w2)
    a2_2 = -r * r
    a3_1 = 2.0 * r * math.cos(w3)
    a3_2 = -r * r
    
    b = (1.0 - r) * 0.5
    
    rng = random.Random(root * 313)
    
    for i in range(total_samples):
        t = i / RATE
        # Attack envelope
        env = min(1.0, i / attack_samples) if i < attack_samples else 1.0
        
        # Vibrato LFO: ~5.2 Hz
        vib = 1.0 + 0.006 * math.sin(2.0 * math.pi * 5.2 * t)
        
        # Glottal pulse excitation: rich buzz
        pulse = 0.0
        for v, d in enumerate(detunes):
            f_v = freq * (1.0 + d) * vib
            phases[v] += 2.0 * math.pi * f_v / RATE
            # Sawtooth-like glottal waveshape
            w = (phases[v] % (2.0 * math.pi)) / (2.0 * math.pi)
            pulse += (1.0 - 2.0 * w) + 0.5 * (1.0 - 2.0 * ((phases[v] * 2.0) % (2.0 * math.pi)) / (2.0 * math.pi))
        
        pulse /= len(detunes)
        # Add slight breath noise
        breath = (rng.random() * 2.0 - 1.0) * 0.08
        src = (pulse * 0.92 + breath) * env
        
        # Resonator filters (biquad 2-pole bandpass)
        out1 = b * src + a1_1 * y1_prev1 + a1_2 * y1_prev2
        y1_prev2 = y1_prev1
        y1_prev1 = out1
        
        out2 = b * src + a2_1 * y2_prev1 + a2_2 * y2_prev2
        y2_prev2 = y2_prev1
        y2_prev1 = out2
        
        out3 = b * src + a3_1 * y3_prev1 + a3_2 * y3_prev2
        y3_prev2 = y3_prev1
        y3_prev1 = out3
        
        if is_oh:
            sample_val = 0.65 * out1 + 0.45 * out2 + 0.15 * out3
        else:
            sample_val = 0.45 * out1 + 0.55 * out2 + 0.25 * out3
        
        # Soft saturation
        sample_val = math.tanh(sample_val * 1.5) * 0.82
        data.extend(struct.pack("b", clamp_byte(sample_val)))
    
    # Adjust loop point to nearest zero crossing with positive slope
    best_loop = loop_start
    min_diff = 999.0
    for cand in range(loop_start - 300, loop_start + 300):
        if 0 < cand < len(data) - 1:
            val = data[cand]
            if val == 0 or (data[cand - 1] <= 0 and data[cand + 1] >= 0):
                best_loop = cand
                break
                
    return bytes(data), best_loop


def make_drums() -> dict[int, bytes]:
    """Synthesizes the 12 epic + chiptune drum kit samples."""
    drums = {}
    rng = random.Random(42)
    
    # 0: Bombo profundo (Deep sub kick)
    # 55Hz pitch drop, punchy click, 0.38s
    length = int(0.38 * RATE)
    buf = bytearray()
    phase = 0.0
    for i in range(length):
        t = i / RATE
        freq = 45.0 + 110.0 * math.exp(-t * 32.0)
        phase += 2.0 * math.pi * freq / RATE
        amp = math.exp(-t * 9.5)
        # click at start
        click = (rng.random() * 2.0 - 1.0) * math.exp(-t * 80.0) * 0.4
        val = math.sin(phase) * amp + click
        val = math.tanh(val * 1.6) * 0.92
        buf.extend(struct.pack("b", clamp_byte(val)))
    drums[0] = bytes(buf)
    
    # 1: Timbal de cine grave (Low cinematic tom/timpani)
    length = int(0.50 * RATE)
    buf = bytearray()
    phase = 0.0
    for i in range(length):
        t = i / RATE
        freq = 75.0 + 90.0 * math.exp(-t * 18.0)
        phase += 2.0 * math.pi * freq / RATE
        amp = math.exp(-t * 7.0)
        val = (math.sin(phase) + 0.3 * math.sin(2.02 * phase)) * amp
        buf.extend(struct.pack("b", clamp_byte(val * 0.88)))
    drums[1] = bytes(buf)

    # 2: Timbal de cine medio (Mid cinematic tom)
    length = int(0.42 * RATE)
    buf = bytearray()
    phase = 0.0
    for i in range(length):
        t = i / RATE
        freq = 115.0 + 130.0 * math.exp(-t * 22.0)
        phase += 2.0 * math.pi * freq / RATE
        amp = math.exp(-t * 8.5)
        val = (math.sin(phase) + 0.25 * math.sin(2.01 * phase)) * amp
        buf.extend(struct.pack("b", clamp_byte(val * 0.88)))
    drums[2] = bytes(buf)

    # 3: Timbal de cine alto (High cinematic tom)
    length = int(0.35 * RATE)
    buf = bytearray()
    phase = 0.0
    for i in range(length):
        t = i / RATE
        freq = 175.0 + 160.0 * math.exp(-t * 26.0)
        phase += 2.0 * math.pi * freq / RATE
        amp = math.exp(-t * 10.0)
        val = (math.sin(phase) + 0.2 * math.sin(2.01 * phase)) * amp
        buf.extend(struct.pack("b", clamp_byte(val * 0.88)))
    drums[3] = bytes(buf)

    # 4: Caja limpia (Clean snappy snare)
    length = int(0.32 * RATE)
    buf = bytearray()
    phase = 0.0
    for i in range(length):
        t = i / RATE
        body_freq = 185.0 * math.exp(-t * 16.0)
        phase += 2.0 * math.pi * body_freq / RATE
        body = math.sin(phase) * math.exp(-t * 24.0) * 0.6
        noise = (rng.random() * 2.0 - 1.0) * math.exp(-t * 14.0) * 0.65
        val = (body + noise) * 0.88
        buf.extend(struct.pack("b", clamp_byte(val)))
    drums[4] = bytes(buf)

    # 5: Caja épica / redoble (Epic snare with long reverb tail)
    length = int(0.60 * RATE)
    buf = bytearray()
    phase = 0.0
    for i in range(length):
        t = i / RATE
        body_freq = 170.0 * math.exp(-t * 14.0)
        phase += 2.0 * math.pi * body_freq / RATE
        body = math.sin(phase) * math.exp(-t * 18.0) * 0.55
        noise = (rng.random() * 2.0 - 1.0) * math.exp(-t * 6.5) * 0.6
        val = (body + noise) * 0.88
        buf.extend(struct.pack("b", clamp_byte(val)))
    drums[5] = bytes(buf)

    # 6: Rimshot / Palma (Sharp rimshot / handclap)
    length = int(0.24 * RATE)
    buf = bytearray()
    phase = 0.0
    for i in range(length):
        t = i / RATE
        freq = 420.0 * math.exp(-t * 30.0)
        phase += 2.0 * math.pi * freq / RATE
        rim = math.sin(phase) * math.exp(-t * 35.0) * 0.7
        # clap slaps (multi-burst)
        burst = 0.0
        if t < 0.012: burst = rng.random() * 2.0 - 1.0
        elif 0.015 < t < 0.027: burst = rng.random() * 2.0 - 1.0
        elif t >= 0.030: burst = (rng.random() * 2.0 - 1.0) * math.exp(-(t - 0.030) * 25.0)
        val = (rim * 0.6 + burst * 0.5) * 0.88
        buf.extend(struct.pack("b", clamp_byte(val)))
    drums[6] = bytes(buf)

    # 7: Tom (8-bit / punchy retro tom)
    length = int(0.35 * RATE)
    buf = bytearray()
    phase = 0.0
    for i in range(length):
        t = i / RATE
        freq = 130.0 * math.exp(-t * 8.0)
        phase += 2.0 * math.pi * freq / RATE
        amp = math.exp(-t * 9.0)
        # square-ish distortion
        s = math.sin(phase)
        val = (s + 0.3 * math.sin(3 * phase)) * amp * 0.88
        buf.extend(struct.pack("b", clamp_byte(val)))
    drums[7] = bytes(buf)

    # 8: Hi-hat cerrado (Closed hi-hat: tight metallic)
    length = int(0.09 * RATE)
    buf = bytearray()
    p1, p2, p3 = 0.0, 0.0, 0.0
    for i in range(length):
        t = i / RATE
        p1 += 2.0 * math.pi * 3200.0 / RATE
        p2 += 2.0 * math.pi * 5400.0 / RATE
        p3 += 2.0 * math.pi * 8300.0 / RATE
        metal = (math.sin(p1) + math.sin(p2) + math.sin(p3)) * 0.3
        noise = (rng.random() * 2.0 - 1.0) * 0.7
        val = (metal + noise) * math.exp(-t * 55.0) * 0.75
        buf.extend(struct.pack("b", clamp_byte(val)))
    drums[8] = bytes(buf)

    # 9: Hi-hat abierto (Open hi-hat: sizzling)
    length = int(0.38 * RATE)
    buf = bytearray()
    p1, p2, p3 = 0.0, 0.0, 0.0
    for i in range(length):
        t = i / RATE
        p1 += 2.0 * math.pi * 3100.0 / RATE
        p2 += 2.0 * math.pi * 5200.0 / RATE
        p3 += 2.0 * math.pi * 8100.0 / RATE
        metal = (math.sin(p1) + math.sin(p2) + math.sin(p3)) * 0.3
        noise = (rng.random() * 2.0 - 1.0) * 0.7
        val = (metal + noise) * math.exp(-t * 11.0) * 0.75
        buf.extend(struct.pack("b", clamp_byte(val)))
    drums[9] = bytes(buf)

    # 10: Platillo / Crash épico (Epic crash cymbal)
    length = int(0.75 * RATE)
    buf = bytearray()
    p1, p2, p3, p4 = 0.0, 0.0, 0.0, 0.0
    for i in range(length):
        t = i / RATE
        p1 += 2.0 * math.pi * 1800.0 / RATE
        p2 += 2.0 * math.pi * 3600.0 / RATE
        p3 += 2.0 * math.pi * 6200.0 / RATE
        p4 += 2.0 * math.pi * 9400.0 / RATE
        metal = (math.sin(p1) + math.sin(p2) + math.sin(p3) + math.sin(p4)) * 0.25
        noise = (rng.random() * 2.0 - 1.0) * 0.75
        val = (metal + noise) * math.exp(-t * 4.8) * 0.78
        buf.extend(struct.pack("b", clamp_byte(val)))
    drums[10] = bytes(buf)

    # 11: Campana / Chime (Bright metallic chime/triangle)
    length = int(0.65 * RATE)
    buf = bytearray()
    p1, p2, p3 = 0.0, 0.0, 0.0
    for i in range(length):
        t = i / RATE
        p1 += 2.0 * math.pi * 2093.0 / RATE  # C7
        p2 += 2.0 * math.pi * 4186.0 / RATE  # C8
        p3 += 2.0 * math.pi * 5587.0 / RATE  # F8 overtone
        chime = math.sin(p1) * 0.6 + math.sin(p2) * 0.3 + math.sin(p3) * 0.15
        val = chime * math.exp(-t * 5.5) * 0.82
        buf.extend(struct.pack("b", clamp_byte(val)))
    drums[11] = bytes(buf)

    return drums


def main() -> None:
    OUT.mkdir(parents=True, exist_ok=True)
    
    print("Generating Choir samples...")
    choir_loops = {}
    for root in (48, 60, 72, 84):
        sample_bytes, loop_pt = make_choir_sample(root)
        filename = f"choir_{root}.bin"
        (OUT / filename).write_bytes(sample_bytes)
        choir_loops[root] = loop_pt
        print(f"  {filename}: {len(sample_bytes)} bytes, loop_point={loop_pt}")

    print("Generating Drum kit samples (12 keys)...")
    drums = make_drums()
    for k, sample_bytes in drums.items():
        filename = f"drum_{k}.bin"
        (OUT / filename).write_bytes(sample_bytes)
        print(f"  {filename}: {len(sample_bytes)} bytes")

    print("Sample generation complete!")


if __name__ == "__main__":
    main()
