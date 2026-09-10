#include "song_files.h"

#include <cstdio>

#ifdef ARM9
#include <fat.h>
#include <sys/stat.h>
#endif

namespace song_files {

static std::string path_for(int id, const char* suffix = "") {
    return std::string(SONG_DIRECTORY) + "/" + filename_for_id(id) + suffix;
}

bool initialize() {
#ifdef ARM9
    if (!fatInitDefault()) return false;
    mkdir(SONG_DIRECTORY, 0777);
#endif
    return true;
}

bool load(int id, song_storage::SongDocument& song) {
    if (!is_song_filename(filename_for_id(id))) return false;
    FILE* file = std::fopen(path_for(id).c_str(), "rb");
    if (!file) return false;
    std::fseek(file, 0, SEEK_END);
    long length = std::ftell(file);
    std::rewind(file);
    if (length <= 0 || length > 20000) { std::fclose(file); return false; }
    std::vector<uint8_t> bytes((std::size_t)length);
    bool read_ok = std::fread(bytes.data(), 1, bytes.size(), file) == bytes.size();
    std::fclose(file);
    return read_ok && song_storage::deserialize(bytes.data(), bytes.size(), song);
}

bool save_atomic(int id, const song_storage::SongDocument& song) {
    if (!is_song_filename(filename_for_id(id))) return false;
    std::vector<uint8_t> bytes = song_storage::serialize(song);
    std::string temp = path_for(id, ".tmp");
    FILE* file = std::fopen(temp.c_str(), "wb");
    if (!file) return false;
    bool ok = std::fwrite(bytes.data(), 1, bytes.size(), file) == bytes.size();
    if (std::fclose(file) != 0) ok = false;
    if (!ok) { std::remove(temp.c_str()); return false; }
    std::string target = path_for(id);
    std::string backup = path_for(id, ".bak");
    std::remove(backup.c_str());
    bool had_previous = std::rename(target.c_str(), backup.c_str()) == 0;
    if (std::rename(temp.c_str(), target.c_str()) == 0) {
        if (had_previous) std::remove(backup.c_str());
        return true;
    }
    std::remove(temp.c_str());
    if (had_previous) std::rename(backup.c_str(), target.c_str());
    return false;
}

bool remove(int id) {
    if (!is_song_filename(filename_for_id(id))) return false;
    bool removed = std::remove(path_for(id).c_str()) == 0;
    std::remove(path_for(id, ".tmp").c_str());
    std::remove(path_for(id, ".bak").c_str());
    return removed;
}

std::array<bool, MAX_SONGS> list_valid() {
    std::array<bool, MAX_SONGS> result{};
    song_storage::SongDocument song{};
    for (int id = 0; id < MAX_SONGS; ++id) result[id] = load(id, song);
    return result;
}

} // namespace song_files
