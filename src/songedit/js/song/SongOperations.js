/* SongOperations.js
 * High-level editing operations on a Song.
 * Separated to avoid clutter.
 */
 
import { Song } from "./Song.js";

export class SongOperations {
  constructor() {
  }
  
  /* High-level entry points.
   ********************************************************/
   
  /* params, all boolean: {
   *  removeLeadingSilence
   *  removeTrailingSilence
   *  removeChannelPressure
   *  removeAftertouch
   *  removeWheel
   *  removeUnknownControl
   *  removeRedundantProgram
   *  removeText
   *  removeUnknownMeta
   *  removeUnknownSysex
   * }
   */
  optimize(song, params) {
    if (song) this._optimize(song, params || {});
  }
  
  /* params: {
   *  TODO
   * }
   */
  quantize(song, params) {
    //TODO
  }
  
  /* Optimize, internals.
   ****************************************************************/
   
  _optimize(song, params) {
    this._optimizeFilterEvents(song, params);
    if (params.removeLeadingSilence) this._optimizeRemoveLeadingSilence(song);
    if (params.removeTrailingSilence) this._optimizeRemoveTrailingSilence(song);
  }
  
  _optimizeFilterEvents(song, params) {
    const programByChannel = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
    song.events = song.events.filter((event) => {
      switch (event.type) {
        case Song.EVENT_NOTE_ADJUST: return !params.removeAftertouch;
        case Song.EVENT_CONTROL: return !params.removeUnknownControl; // all Control Changes are "unknown" to us
        case Song.EVENT_PROGRAM: {
            if (!params.removeRedundantProgram) return true;
            if (event.a === programByChannel[event.channel]) return false;
            programByChannel[event.channel] = event.a;
            return true;
          } break;
        case Song.EVENT_PRESSURE: return !params.removeChannelPressure;
        case Song.EVENT_WHEEL: return !params.removeWheel;
        case Song.EVENT_META: switch (event.meta) {
            case 0x2f: return true; // Terminate
            case 0x51: return true; // Set Tempo
            default: {
                if (event.meta < 0x20) return !params.removeText;
                return !params.removeUnknownMeta;
              }
          } break;
        case Song.EVENT_SYSEX: return !params.removeUnknownSysex; // all Sysex are "unknown" to us
      }
      return true;
    });
  }
  
  _optimizeRemoveLeadingSilence(song) {
    let firstNoteTime = 0;
    for (const event of song.events) {
      if (event.type !== Song.EVENT_NOTE_ON) continue;
      firstNoteTime = event.time;
      break;
    }
    if (!firstNoteTime) return;
    for (const event of song.events) {
      if (event.time <= firstNoteTime) event.time = 0;
      else event.time -= firstNoteTime;
    }
  }
  
  /* Trailing silence is a kind of weird proposition.
   * (note that we also take care of it at mksong, when converting from MIDI to our private format).
   * We don't want to eliminate *all* the trailing silence, because it might deliberately run silent to the end of a measure.
   * Um. Come to think of it, we actually never repeat songs so i guess whatever.
   * But we'll do what mksong does: Look for Meta termination events with no intervening notes.
   * This means our end time comes from a Meta termination event, but not necessarily the last one.
   */
  _optimizeRemoveTrailingSilence(song) {
    let termIndex = -1, termTime = 0, noteCount = 0, thisIndex = -1;;
    for (const event of song.events) {
      thisIndex++;
      switch (event.type) {
        case Song.EVENT_NOTE_ON: noteCount++; break;
        case Song.EVENT_META: if (event.meta === 0x2f) {
            if (noteCount) {
              termIndex = thisIndex;
              termTime = event.time;
              noteCount = 0;
            }
          } break;
      }
    }
    if (!noteCount && (termIndex >= 0)) {
      // Whatever we got as (termTime), make that the time of all events after (termIndex).
      for (let i=termIndex; i<song.events.length; i++) {
        event.time = termTime;
      }
    }
  }
}

SongOperations.singleton = true;