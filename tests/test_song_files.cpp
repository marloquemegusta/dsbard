#include "song_files.h"

#include <cassert>

int main() {
    assert(song_files::is_song_filename("song000.dssg"));
    assert(song_files::is_song_filename("song015.dssg"));
    assert(!song_files::is_song_filename("song016.dssg"));
    assert(!song_files::is_song_filename("song001.tmp"));
    assert(song_files::song_id_from_filename("song007.dssg") == 7);
    assert(song_files::filename_for_id(12) == "song012.dssg");
    return 0;
}
