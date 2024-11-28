Sprconv is the 16-bit DOS version, texconv is the Win32 version (a bit more straightforward in how it functions as we're not limited by 16-bit DOS memory management).

Both versions support RLE compression up to two-byte depth (i.e. maximum theoretical compression is 65535:1). On files that have a lot of constant colour variance (i.e. files with lots of dithering) the compressed file can easily take *more* space than the raw file.

List conversion usage: write filename with extension, then 1 or 0 depending if you want transparency or not, then number of frames (if the game uses a format that has multiple frames in the same file, e.g. Amazeing, otherwise use 1), then if 1 or 0 depending on if you want RLE compression or not. Hit Enter and add the next file with the same information and so on until you're done.

See Rocket.txt for an example file.