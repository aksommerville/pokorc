# Pocket Orchestra

## TODO

- [x] Song select, intro splash
- [x] Game over splash
- [x] Customizable dancer
- - [x] React to combo
- [x] Finalize scoring
- - There's three levels of combo multiplier. The second and third are too close together.
- - xxx Extra fireworks when you reach the next multiplier. meh
- [x] Fireworks
- [ ] Persist high scores
- - [ ] Medal per song: None, Turkey, Ribbon, Cup
- [ ] MacOS drivers
- [ ] Windows drivers
- [x] Tiny menu thumbnail
- xxx Pause menu (down 3 times?). meh
- [ ] Are notes passing the line below the cue? Take screencaps at the moment we register a note.
- - It feels like they're low at the cue moment.
- [ ] Test the boundaries, how far off do you have to be to miss a note?
- [ ] Eliminate redundant notes from tinylamb and goatpotion
- [ ] More songs
- [ ] Sort songs by difficulty.
- [ ] Final waves
- [x] New strategy for encoding cues in MIDI: Use Note On velocity instead of Program Change.

## Defects

- ~2022-04-11T18:18 stuck note in tinylamb~
- - Not reliably reproducible.
- - I had forgotten to lock ALSA from genioc during loop().
- - [x] Play ten times with the lock, if you don't get stuck call it fixed.

- ~2022-04-11T20:37 tinylamb crashes the Tiny.~
- - This happened before and I fixed it by reading the song as uint8_t instead of uint32_t
- - [x] Made the same change reading song header (uint8_t rather than uint16_t). I guess that fixed it?
