#include "song_storage.h"

#include <cassert>

int main() {
    using namespace song_storage;
    SongDocument original{};
    original.pulse_ms = 750;
    ensure_default_label(original);
    draw_label_segment(original.label, 4, 5, 12, 5);
    original.tracks[3].occupied = true;
    original.tracks[3].duration_ms = 1250;
    original.tracks[3].event_count = 1;
    original.tracks[3].events[0] = {250, 500, 64, 2, 1, 7};
    std::vector<uint8_t> encoded = serialize(original);
    SongDocument decoded{};
    assert(deserialize(encoded.data(), encoded.size(), decoded));
    assert(decoded.pulse_ms == 750);
    assert(decoded.label.width == LABEL_WIDTH);
    assert(decoded.tracks[3].event_count == 1);
    assert(decoded.tracks[3].events[0].midi_note == 64);
    assert(decoded.tracks[3].events[0].sustain_level == 7);
    std::size_t bit = (std::size_t)5 * LABEL_WIDTH + 8;
    assert((decoded.label.pixels[bit / 8] & (uint8_t)(1u << (bit % 8))) != 0);
    encoded[encoded.size() - 1] ^= 1;
    assert(!deserialize(encoded.data(), encoded.size(), decoded));
    return 0;
}
