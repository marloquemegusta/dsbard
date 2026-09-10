#pragma once

#include "song_storage.h"

#include <cstdint>
#include <array>
#include <string>

namespace song_files {

static constexpr uint8_t MAX_SONGS = 16;
static constexpr const char* SONG_DIRECTORY = "/songs";

inline std::string filename_for_id(int id) {
    if (id < 0 || id >= MAX_SONGS) return std::string();
    char name[16];
    name[0] = 's'; name[1] = 'o'; name[2] = 'n'; name[3] = 'g';
    name[4] = (char)('0' + id / 100);
    name[5] = (char)('0' + (id / 10) % 10);
    name[6] = (char)('0' + id % 10);
    name[7] = '.'; name[8] = 'd'; name[9] = 's'; name[10] = 's'; name[11] = 'g';
    name[12] = '\0';
    return std::string(name);
}

inline int song_id_from_filename(const std::string& name) {
    if (name.size() != 12 || name[0] != 's' || name[1] != 'o' || name[2] != 'n' ||
        name[3] != 'g' || name[7] != '.' || name.substr(8) != "dssg") return -1;
    for (int i = 4; i <= 6; ++i) if (name[i] < '0' || name[i] > '9') return -1;
    int id = (name[4] - '0') * 100 + (name[5] - '0') * 10 + name[6] - '0';
    return id < MAX_SONGS ? id : -1;
}

inline bool is_song_filename(const std::string& name) {
    return song_id_from_filename(name) >= 0;
}

bool initialize();
bool load(int id, song_storage::SongDocument& song);
bool save_atomic(int id, const song_storage::SongDocument& song);
bool remove(int id);
std::array<bool, MAX_SONGS> list_valid();

} // namespace song_files
