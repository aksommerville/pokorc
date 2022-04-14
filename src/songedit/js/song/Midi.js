/* Midi.js
 * Constants and static helpers.
 */
 
export class Midi {

  /* I think the convention among musicians is that octaves begin on C?
   * There are things I can tolerate and things I can't... Our octaves begin on A.
   */
  static noteName(note) {
    if (!note || (note < 9)) return "low";
    if (note > 127) return "high";
    const octave = Math.floor((note - 9) / 12);
    let tone;
    switch ((note - 9) % 12) {
      case 0: tone = "a"; break;
      case 1: tone = "a#"; break;
      case 2: tone = "b"; break;
      case 3: tone = "c"; break;
      case 4: tone = "c#"; break;
      case 5: tone = "d"; break;
      case 6: tone = "d#"; break;
      case 7: tone = "e"; break;
      case 8: tone = "f"; break;
      case 9: tone = "f#"; break;
      case 10: tone = "g"; break;
      case 11: tone = "g#"; break;
    }
    return `${tone}${octave}`;
  }
  
  static controlKeyName(key) {
    return ""; // TODO I think not interesting for us
  }

}

/* The 128 GM programs, indexed by zero-based pid.
 * (NB Program ID is conventionally one-based but I don't do that).
 */
Midi.GM_PROGRAM_NAMES = [
  "Acoustic Grand Piano",
  "Bright Acoustic Piano",
  "Electric Grand Piano",
  "Honky-tonk Piano",
  "Electric Piano 1 (Rhodes Piano)",
  "Electric Piano 2 (Chorused Piano)",
  "Harpsichord",
  "Clavinet",
  "Celesta",
  "Glockenspiel",
  "Music Box",
  "Vibraphone",
  "Marimba",
  "Xylophone",
  "Tubular Bells",
  "Dulcimer (Santur)",
  "Drawbar Organ (Hammond)",
  "Percussive Organ",
  "Rock Organ",
  "Church Organ",
  "Reed Organ",
  "Accordion (French)",
  "Harmonica",
  "Tango Accordion (Band neon)",
  "Acoustic Guitar (nylon)",
  "Acoustic Guitar (steel)",
  "Electric Guitar (jazz)",
  "Electric Guitar (clean)",
  "Electric Guitar (muted)",
  "Overdriven Guitar",
  "Distortion Guitar",
  "Guitar harmonics",
  "Acoustic Bass",
  "Electric Bass (fingered)",
  "Electric Bass (picked)",
  "Fretless Bass",
  "Slap Bass 1",
  "Slap Bass 2",
  "Synth Bass 1",
  "Synth Bass 2",
  "Violin",
  "Viola",
  "Cello",
  "Contrabass",
  "Tremolo Strings",
  "Pizzicato Strings",
  "Orchestral Harp",
  "Timpani",
  "String Ensemble 1 (strings)",
  "String Ensemble 2 (slow strings)",
  "SynthStrings 1",
  "SynthStrings 2",
  "Choir Aahs",
  "Voice Oohs",
  "Synth Voice",
  "Orchestra Hit",
  "Trumpet",
  "Trombone",
  "Tuba",
  "Muted Trumpet",
  "French Horn",
  "Brass Section",
  "SynthBrass 1",
  "SynthBrass 2",
  "Soprano Sax",
  "Alto Sax",
  "Tenor Sax",
  "Baritone Sax",
  "Oboe",
  "English Horn",
  "Bassoon",
  "Clarinet",
  "Piccolo",
  "Flute",
  "Recorder",
  "Pan Flute",
  "Blown Bottle",
  "Shakuhachi",
  "Whistle",
  "Ocarina",
  "Lead 1 (square wave)",
  "Lead 2 (sawtooth wave)",
  "Lead 3 (calliope)",
  "Lead 4 (chiffer)",
  "Lead 5 (charang)",
  "Lead 6 (voice solo)",
  "Lead 7 (fifths)",
  "Lead 8 (bass + lead)",
  "Pad 1 (new age Fantasia)",
  "Pad 2 (warm)",
  "Pad 3 (polysynth)",
  "Pad 4 (choir space voice)",
  "Pad 5 (bowed glass)",
  "Pad 6 (metallic pro)",
  "Pad 7 (halo)",
  "Pad 8 (sweep)",
  "FX 1 (rain)",
  "FX 2 (soundtrack)",
  "FX 3 (crystal)",
  "FX 4 (atmosphere)",
  "FX 5 (brightness)",
  "FX 6 (goblins)",
  "FX 7 (echoes, drops)",
  "FX 8 (sci-fi, star theme)",
  "Sitar",
  "Banjo",
  "Shamisen",
  "Koto",
  "Kalimba",
  "Bag pipe",
  "Fiddle",
  "Shanai",
  "Tinkle Bell",
  "Agogo",
  "Steel Drums",
  "Woodblock",
  "Taiko Drum",
  "Melodic Tom",
  "Synth Drum",
  "Reverse Cymbal",
  "Guitar Fret Noise",
  "Breath Noise",
  "Seashore",
  "Bird Tweet",
  "Telephone Ring",
  "Helicopter",
  "Applause",
  "Gunshot",
];

/* MIDI GM drum names, indexed by zero-based Note ID.
 * Not every valid note has a drum assignment.
 * The ones that do exist are contiguous, 35..81.
 */
Midi.GM_DRUM_NAMES = [
  "", "", "", "", "", "", "",
  "", "", "", "", "", "", "",
  "", "", "", "", "", "", "",
  "", "", "", "", "", "", "",
  "", "", "", "", "", "", "",
  "Acoustic Bass Drum",
  "Bass Drum 1",
  "Side Stick",
  "Acoustic Snare",
  "Hand Clap",
  "Electric Snare",
  "Low Floor Tom",
  "Closed Hi Hat",
  "High Floor Tom",
  "Pedal Hi-Hat",
  "Low Tom",
  "Open Hi-Hat",
  "Low-Mid Tom",
  "Hi Mid Tom",
  "Crash Cymbal 1",
  "High Tom",
  "Ride Cymbal 1",
  "Chinese Cymbal",
  "Ride Bell",
  "Tambourine",
  "Splash Cymbal",
  "Cowbell",
  "Crash Cymbal 2",
  "Vibraslap",
  "Ride Cymbal 2",
  "Hi Bongo",
  "Low Bongo",
  "Mute Hi Conga",
  "Open Hi Conga",
  "Low Conga",
  "High Timbale",
  "Low Timbale",
  "High Agogo",
  "Low Agogo",
  "Cabasa",
  "Maracas",
  "Short Whistle",
  "Long Whistle",
  "Short Guiro",
  "Long Guiro",
  "Claves",
  "Hi Wood Block",
  "Low Wood Block",
  "Mute Cuica",
  "Open Cuica",
  "Mute Triangle",
  "Open Triangle",
];
