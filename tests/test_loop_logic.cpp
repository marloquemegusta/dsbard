#include <cassert>
#include "loop_logic.h"

int main() {
    using namespace loop_logic;
    assert(phase_ms(0, 1000) == 0);
    assert(phase_ms(1250, 1000) == 250);
    assert(phase_ms(50, 0) == 0);
    assert(wrap_track(0, -1) == 2);
    assert(wrap_track(2, 1) == 0);
    NoteEvent event{100, 200, 60, 0};
    assert(!event_active(event, 99));
    assert(event_active(event, 100));
    assert(!event_active(event, 200));
    Track track{};
    track.origin_ms = 5000;
    assert(phase_ms(7250 - track.origin_ms, 1000) == 250);
    assert(sizeof(track.events) / sizeof(track.events[0]) == 96);
    return 0;
}
