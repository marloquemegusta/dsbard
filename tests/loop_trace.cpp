#include <iostream>
#include "loop_logic.h"

int main() {
    using namespace loop_logic;

    Track track{};
    track.occupied = true;
    track.duration_ms = 1600;
    track.phrase_count = 1;
    track.phrases[0] = {0, 800, 3, 2};
    track.event_count = 6;
    track.events[0] = {100, 200, 60, 0};
    track.events[1] = {300, 500, 64, 0};
    track.events[2] = {600, 700, 67, 0};
    track.events[3] = {900, 1000, 60, 0};
    track.events[4] = {1100, 1300, 64, 0};
    track.events[5] = {1400, 1500, 67, 0};

    std::cout << "TRACE|RECORD_START|absolute=0\n";
    for (uint16_t i = 0; i < track.event_count; ++i) {
        const NoteEvent& event = track.events[i];
        std::cout << "TRACE|EVENT|index=" << i
                  << "|start=" << event.start_ms
                  << "|end=" << event.end_ms
                  << "|midi=" << event.midi_note << "\n";
    }

    const uint32_t probes[] = {150, 350, 650, 950};
    const int expected[] = {60, 64, 67, 60};
    for (unsigned i = 0; i < sizeof(probes) / sizeof(probes[0]); ++i) {
        const uint32_t phase = phase_ms(probes[i], track.duration_ms);
        int active = -1;
        for (uint16_t j = 0; j < track.event_count; ++j) {
            if (event_active(track.events[j], phase)) {
                active = track.events[j].midi_note;
                break;
            }
        }
        std::cout << "TRACE|PLAYBACK|absolute=" << probes[i]
                  << "|phase=" << phase
                  << "|midi=" << active << "\n";
        if (active != expected[i]) {
            std::cerr << "TRACE|RESULT|FAIL|probe=" << probes[i]
                      << "|expected=" << expected[i]
                      << "|actual=" << active << "\n";
            return 1;
        }
    }

    std::cout << "TRACE|RESULT|PASS\n";
    return 0;
}
