/* Song.js
 * Live model of a MIDI file, with extra Pocket Orchestra knowledge.
 */
 
import { Decoder } from "/js/app/Decoder.js";
import { Encoder } from "/js/app/Encoder.js";
 
/* Event:
 *   time: integer // ticks
 *   type: integer
 *   track: integer // zero-based index of MTrk
 *   channel?: integer // all but META and SYSEX, 0..15
 *   duration?: integer // NOTE_ON only, ticks
 *   offVelocity?: integer // NOTE_ON only
 *   meta?: integer // META only; the one-byte meta event id
 *   body?: any // META,SYSEX
 *   a,b?: integer // NOTE_ON,NOTE_OFF,NOTE_ADJUST,CONTROL,PROGRAM,PRESSURE, 0..127
 *   wheel?: integer // WHEEL only, -8192..8191
 */
 
export class Song {
  constructor(serial) {
    this.format = 0; // MThd
    this.trackCount = 0; // MThd (may disagree with discovered track count)
    this.ticksPerQnote = 48; // aka MThd Division
    this.trackChunkCount = 0;
    this.longestEvent = 0; // so renderer knows how far left of the edge to read
    this.secondsPerQnote = 0.500; // from Meta 0x51 if present
    this.events = [];
    if (serial) this._decode(serial);
  }
  
  getDurationTicks() {
    // unfortunately, the last event is not necessarily the end of the song, could be a long note before it
    let end = 0;
    for (const event of this.events) {
      const q = event.time + (event.duration || 0);
      if (q > end) end = q;
    }
    return end;
  }
  
  getDurationQnotes() {
    return Math.ceil(this.getDurationTicks() / this.ticksPerQnote);
  }
  
  _decode(serial) {
    this.secondsPerQnote = 0; // so we can detect duplicates
    const src = new Decoder(serial);
    while (src.remaining()) {
      const chunkId = src.string(4);
      const chunk = src.u32belen();
      this._decodeChunk(chunkId, chunk);
    }
    this.longestEvent = this._calculateLongestEvent();
    if (!this.secondsPerQnote) {
      console.log(`No explicit tempo, assuming 500 ms/qnote`);
      this.secondsPerQnote = 0.500;
    }
  }
  
  _decodeChunk(chunkId, chunk) {
    const src = new Decoder(chunk);
    switch (chunkId) {
    
      case "MThd": {
          this.format = src.u16be();
          this.trackCount = src.u16be();
          this.ticksPerQnote = src.u16be();
          if (
            (this.format !== 1) ||
            (this.trackCount < 1) ||
            (this.ticksPerQnote < 1) ||
            (this.ticksPerQnote & 0x8000) // SMPTE timing, we won't support that
          ) throw new Error(`Unsupported content in MThd: ${this.format},${this.trackCount},${this.ticksPerQnote}`);
        } break;
        
      case "MTrk": {
          this._decodeTrack(src);
        } break;
        
      default: {
          console.log(`Ignoring unknown ${chunk.byteLength}-byte '${chunkId}' chunk`);
        }
    }
  }
  
  _decodeTrack(src) {
    let runningStatus = 0;
    let time = 0;
    const track = this.trackChunkCount++;
    while (src.remaining()) {
    
      const delay = src.vlq();
      time += delay;
    
      let lead = src.u8();
      if (!(lead & 0x80)) {
        src.unread(1);
        if (!runningStatus) throw new Error(`Unexpected leading byte ${lead}, Running Status unset`);
        lead = runningStatus;
      } else {
        runningStatus = lead;
      }
      
      // NOTE_OFF is special: We attach it to the associated NOTE_ON
      if ((lead & 0xf0) === 0x80) {
        const note = src.u8();
        const velocity = src.u8();
        this._addNoteOff(track, time, note, velocity, lead & 0x0f);
        continue;
      }
      
      // NOTE_ON is half-special: With zero velocity, it's actually NOTE_OFF
      if ((lead & 0xf0) === 0x90) {
        const note = src.u8();
        const velocity = src.u8();
        if (velocity) {
          const event = this.addEvent(time);
          event.type = Song.EVENT_NOTE_ON;
          event.track = track;
          event.channel = lead & 0x0f;
          event.a = note;
          event.b = velocity;
          event.duration = 0;
          event.offVelocity = 0;
        } else {
          this._addNoteOff(track, time, note, 0x40, lead & 0x0f);
        }
        continue;
      }
      
      const event = this.addEvent(time);
      event.type = lead & 0xf0; // a reasonable first guess
      event.track = track;
      event.channel = lead & 0x0f;
      
      switch (lead & 0xf0) {
        case 0xa0:
        case 0xb0: {
            event.a = src.u8();
            event.b = src.u8();
          } break;
        case 0xc0:
        case 0xd0: {
            event.a = src.u8();
          } break;
        case 0xe0: {
            const lo = src.u8();
            const hi = src.u8();
            if ((lo & 0x80) || (hi & 0x80)) throw new Error(`Unexpected wheel data ${lo},${hi}`);
            event.wheel = ((hi << 7) | lo) - 8192;
          } break;
        case 0xf0: {
            runningStatus = 0;
            delete event.channel;
            if ((lead === 0xf0) || (lead === 0xf7)) {
              event.type = Song.EVENT_SYSEX;
              event.body = src.vlqlen();
              this._examineSysex(event);
            } else if (lead === 0xff) {
              event.type = Song.EVENT_META;
              event.meta = src.u8();
              event.body = src.vlqlen();
              this._examineMeta(event);
            } else {
              throw new Error(`Unexpected leading byte ${lead}`);
            }
          } break;
      }
    }
  }
  
  _examineSysex(event) {
  }
  
  _examineMeta(event) {
    switch (event.meta) {
    
      case 0x51: {
          if (event.body.byteLength !== 3) {
            console.log(`Ignoring Meta 0x51 Set Tempo due to unexpected length ${event.body.byteLength}, should be 3`);
          } else {
            const src = new Uint8Array(event.body);
            const v = ((src[0] << 16) | (src[1] << 8) | src[0]) / 1000000.0;
            if (this.secondsPerQnote && (this.secondsPerQnote !== v)) {
              console.log(`Tempo change detected! We are going to use the first one, ${this.secondsPerQnote} s/qn, and ignore ${v}`);
            } else {
              this.secondsPerQnote = v;
            }
          }
        } break;
        
    }
  }
  
  _addNoteOff(track, time, note, velocity, channel) {
    for (let i = this.events.length; i-- > 0 ; ) {
      const event = this.events[i];
      if (event.channel !== channel) continue;
      if (event.time > time) continue;
      if (event.type !== Song.EVENT_NOTE_ON) continue;
      if (event.a !== note) continue;
      if (event.offVelocity) return; // already terminated
      event.duration = time - event.time;
      event.offVelocity = velocity || 1;
      return;
    }
  }
  
  addEvent(time) {
    const event = { time };
    if (!this.events.length || (time >= this.events[this.events.length - 1].time)) {
      this.events.push(event);
    } else {
      let lo = 0, hi = this.events.length;
      while (lo < hi) {
        const ck = (lo + hi) >> 1;
             if (time < this.events[ck].time) hi = ck;
        else if (time > this.events[ck].time) lo = ck + 1;
        else {
          lo = ck;
          while ((lo < hi) && (this.events[lo].time === time)) lo++;
          break;
        }
      }
      this.events.splice(lo, 0, event);
    }
    return event;
  }
  
  searchEvents(time) {
    let lo = 0, hi = this.events.length;
    while (lo < hi) {
      let ck = (lo + hi) >> 1;
           if (time < this.events[ck].time) hi = ck;
      else if (time > this.events[ck].time) lo = ck + 1;
      else {
        while ((ck > lo) && (this.events[ck - 1].time === time)) ck--;
        return ck;
      }
    }
    return lo;
  }
  
  _calculateLongestEvent() {
    return this.events.reduce((a, e) => Math.max(e.duration || 0, a), 0);
  }
  
  encode() {
    const dst = new Encoder();
    this._encodeMThd(dst);
    this._encodeTracks(dst);
    return dst.finish();
  }
  
  _encodeMThd(dst) {
    dst.raw("MThd");
    const p = dst.dstc;
    dst.u16be(this.format);
    dst.u16be(this.trackCount);
    dst.u16be(this.ticksPerQnote);
    dst.u32belen(p);
  }
  
  _encodeTracks(dst) {
    for (let i=0; i<this.trackChunkCount; i++) {
      dst.raw("MTrk");
      const p = dst.dstc;
      this._encodeTrack(dst, i);
      dst.u32belen(p);
    }
  }
  
  _encodeTrack(dst, track) {
    let time = 0;
    for (const event of this._flattenTrackEvents(track)) {
      if (event.track !== track) continue;
      if (event.time > time) {
        dst.vlq(event.time - time);
        time = event.time;
      } else {
        dst.u8(0); // ie vlq zero
      }
      this._encodeEvent(dst, event);
    }
  }
  
  /* Returns an array of all events on the given track,
   * with a fake Note Off event for each Note On.
   */
  _flattenTrackEvents(track) {
    const dst = [];
    const offs = [];
    for (const event of this.events) {
      if (event.track !== track) continue;
      
      while (offs.length && (offs[0].time <= event.time)) {
        dst.push(offs[0]);
        offs.splice(0, 1);
      }
      
      dst.push(event);
      
      if (event.type === Song.EVENT_NOTE_ON) {
        const off = {
          track,
          channel: event.channel,
          time: event.time + event.duration,
          type: Song.EVENT_NOTE_OFF,
          a: event.a,
          b: event.offVelocity,
        };
        const before = offs.findIndex(e => e.time > off.time);
        if (before < 0) offs.push(off);
        else offs.splice(before, 0, off);
      }
    }
    for (const off of offs) {
      dst.push(off);
    }
    return dst;
  }
  
  /* Not the delay; caller does that.
   * We don't track Running Status and don't turn Off into On Zero.
   * So our MIDI files are suboptimal sizewise.
   * This would matter if we are going to distribute them, but for us, MIDI files are going to be converted anyway.
   */
  _encodeEvent(dst, event) {
    switch (event.type) {
      case Song.EVENT_NOTE_OFF:
      case Song.EVENT_NOTE_ON:
      case Song.EVENT_NOTE_ADJUST:
      case Song.EVENT_NOTE_CONTROL: {
          dst.u8(event.type | event.channel);
          dst.u8(event.a);
          dst.u8(event.b);
        } break;
      case Song.EVENT_PROGRAM:
      case Song.EVENT_PRESSURE: {
          dst.u8(event.type | event.channel);
          dst.u8(event.a);
        } break;
      case Song.EVENT_WHEEL: {
          dst.u8(event.type | event.channel);
          const n = event.wheel + 8192;
          dst.u8(n & 0x7f);
          dst.u8(n >> 7);
        } break;
      case Song.EVENT_SYSEX: {
          dst.u8(0xf0);//TODO should distinguish 0xf0 and 0xf7, if we care, which i don't
          const p = dst.dstc;
          dst.raw(event.body);
          dst.vlqlen(p);
        } break;
      case Song.EVENT_META: {
          dst.u8(0xff);
          dst.u8(event.meta);
          const p = dst.dstc;
          dst.raw(event.body);
          dst.vlqlen(p);
        } break;
    }
  }
  
  /* Call if you change (event.time). We'll figure out its new position.
   */
  shuffleForChangedTime(event) {
    let p = this.events.indexOf(event);
    if (p < 0) return -1;
    while ((p > 0) && (event.time < this.events[p - 1].time)) {
      this.events[p] = this.events[p - 1];
      this.events[p - 1] = event;
      p--;
    }
    while ((p + 1 < this.events.length) && (event.time > this.events[p + 1].time)) {
      this.events[p] = this.events[p + 1];
      this.events[p + 1] = event;
      p++;
    }
    return p;
  }
  
  removeEvent(event) {
    const p = this.events.indexOf(event);
    if (p < 0) return false;
    this.events.splice(p, 1);
    return true;
  }
}

// Event IDs follow MIDI where possible, for no particular reason.
Song.EVENT_NOTE_OFF = 0x80;
Song.EVENT_NOTE_ON = 0x90;
Song.EVENT_NOTE_ADJUST = 0xa0;
Song.EVENT_CONTROL = 0xb0;
Song.EVENT_PROGRAM = 0xc0;
Song.EVENT_PRESSURE = 0xd0;
Song.EVENT_WHEEL = 0xe0;
Song.EVENT_SYSEX = 0xf0;
Song.EVENT_META = 0xff;
