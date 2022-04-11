# Pocket Orchestra

## TODO

- [ ] Song select, intro splash
- [ ] Game over splash
- [ ] Customizable dancer
- - [ ] React to combo
- [ ] Finalize scoring
- [x] Fireworks
- [ ] Persist high scores
- [ ] MacOS drivers
- [ ] Windows drivers
- [ ] Tiny menu thumbnail
- [ ] Pause menu (down 3 times?)
- [ ] Are notes passing the line below the cue? Take screencaps at the moment we register a note.
- - It feels like they're low at the cue moment.
- [ ] Test the boundaries, how far off do you have to be to miss a note?
- [ ] Eliminate redundant notes from tinylamb and goatpotion
- [ ] More songs
- [ ] Final waves

## Defects

- 2022-04-11T18:18 stuck note in tinylamb
- - Not reliably reproducible.
- - I had forgotten to lock ALSA from genioc during loop().
- - [x] Play ten times with the lock, if you don't get stuck call it fixed.
