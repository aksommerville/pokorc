/* ChartRenderer.js
 */
 
import { Song } from "/js/song/Song.js";
 
const NOTE_HEIGHT_MIN = 5;
const NOTE_HEIGHT_MAX = 30;
const RIGHT_MARGIN_QNOTES = 0;
const NOTE_WIDTH_MIN = 5;
const BEAT_WIDTH_MIN = 4;
const BEAT_WIDTH_MAX = 200;
const TIME_FUZZ_WIDTH = 4;
 
export class ChartRenderer {
  constructor() {
    this.song = null; // owner sets directly
    this.canvas = null; // ''
    this.context = null; // ''
    this.scroll = [0, 0]; // ''
    this.beatWidth = BEAT_WIDTH_MIN;
    this.noteHeight = NOTE_HEIGHT_MIN;
    this.channelFilter = "all"; // "all" or integer
    this.eventFilter = "all"; // per Toolbar
    this.highlightEvent = null;
  }
  
  /* Desired total dimensions in pixels: [w,h]
   * (maxw,maxh) are the visible space, a hint only.
   */
  measure(maxw, maxh) {
    if (!this.song) return [0, 0];
    if (!maxw || (maxw < 256)) maxw = 256;
    if (!maxh || (maxh < 256)) maxh = 256;
    const h = this.noteHeight * 128;
    const w = this.beatWidth * (this.song.getDurationQnotes() + RIGHT_MARGIN_QNOTES);
    return [w, h];
  }
  
  /* Adjust zoom. Inputs are normalized from whatever range we feel should be available.
   * 0=out, 1=in.
   */
  zoom(x, y) {
    if (x < 0) x = 0; else if (x > 1) x = 1;
    if (y < 0) y = 0; else if (y > 1) y = 1;
    this.beatWidth = BEAT_WIDTH_MIN + Math.round(x * (BEAT_WIDTH_MAX - BEAT_WIDTH_MIN + 1));
    this.noteHeight = NOTE_HEIGHT_MIN + Math.round(y * (NOTE_HEIGHT_MAX - NOTE_HEIGHT_MIN + 1));
  }
  
  /* (channel): "all", "0" .. "15"
   * (events): "all", "notes", "inputs"
   * Refer Toolbar.js
   */
  setVisibility(channel, events) {
    if (channel === "all") this.channelFilter = "all";
    else this.channelFilter = +channel;
    this.eventFilter = events;
  }
  
  /* [x, y] what should our scroll be, to put the given time and note in the center?
   * Caller could ask this, after taking (timeForX(), noteForY()) and then zooming.
   * This does not actually change the scroll position. Or anything else.
   */
  measureScrollForFocus(time, note) {
    if (!this.song || !this.canvas) return [0, 0];
    let [x, y] = this.getPositionForEvent({ time, a: note, type: Song.EVENT_NOTE_ON });
    x += this.scroll[0]; // unapply scroll...
    y += this.scroll[1];
    x = Math.round(x - this.canvas.width / 2);
    y = Math.round(y - this.canvas.height / 2);
    return [x, y];
  }
  
  /* Akin to findEventAtPoint(), just one axis.
   */
  timeForX(x) {
    if (!this.song) return 0;
    x += this.scroll[0];
    const timeFuzz = (TIME_FUZZ_WIDTH * this.song.ticksPerQnote) / this.beatWidth;
    return (x * this.song.ticksPerQnote) / this.beatWidth;
  }
  noteForY(y) {
    y += this.scroll[1];
    let note = 127 - Math.floor(y / this.noteHeight);
    if (note < 0) return 0;
    if (note > 127) return 127;
    return note;
  }
  
  /* (x,y) are relative to the displayed element -- don't add scroll yourself.
   * Returns null or an event from the song.
   */
  findEventAtPoint(x, y) {
    if (!this.song) return null;
    x += this.scroll[0];
    y += this.scroll[1];
    let note = 127 - Math.floor(y / this.noteHeight);
    if ((note < 0) || (note >= 128)) return null;
    const timeFuzz = (TIME_FUZZ_WIDTH * this.song.ticksPerQnote) / this.beatWidth;
    const time = (x * this.song.ticksPerQnote) / this.beatWidth;
    let p = this.song.searchEvents(time - this.song.longestEvent - timeFuzz);
    if (p < 0) p = -p - 1;
    if (p < 0) p = 0;
    const candidates = [];
    for (; p<this.song.events.length; p++) {
      const event = this.song.events[p];
      if (event.time > time + timeFuzz) break;
      if (!this.eventIsEnabled(event)) continue;
      if (event.type === Song.EVENT_NOTE_ON) {
        if (event.a !== note) continue;
      }
      const eventEndTime = event.time + (event.duration || 0);
      if (eventEndTime < time - timeFuzz) continue;
      candidates.push(event);
    }
    if (!candidates.length) return null;
    
    return candidates.reduce((best, event) => {
      if (!best) return event;
      
      // Notes are preferable to impulses.
      const bestIsNote = best.type === Song.EVENT_NOTE_ON;
      const eventIsNote = event.type === Song.EVENT_NOTE_ON;
      if (bestIsNote && !eventIsNote) return best;
      if (!bestIsNote && eventIsNote) return event;
      
      // Among impulses, closest in time wins and that's all.
      if (!bestIsNote) {
        const bestDiff = Math.abs(time - best.time);
        const eventDiff = Math.abs(time - event.time);
        return (bestDiff < eventDiff) ? best : event;
      }
      
      // One OOB, the other wins.
      if ((time < best.time) || (time >= best.time + best.duration)) {
        if ((time >= event.time) && (time < event.time + event.duration)) {
          return event;
        }
      } else if ((time < event.time) || (time >= event.time + event.duration)) {
        return best;
      }
      
      // Finally take the higher time or index -- this is always (event).
      // This causes us to match rendering order when two notes overlap.
      return event;
    }, null);
  }
  
  /* Estimate [x,y] in element space for any event.
   * We apply scroll, you apply element offset.
   */
  getPositionForEvent(event) {
    if (!this.song || !event) return [0, 0];
    let x = event.time, y;
    if (event.duration) x += event.duration / 2;
    x = Math.round((x * this.beatWidth) / this.song.ticksPerQnote - this.scroll[0]);
    if (event.type === Song.EVENT_NOTE_ON) {
      y = Math.round((127 - event.a + 0.5) * this.noteHeight - this.scroll[1]);
    } else {
      y = Math.round(this.canvas.height / 2);
    }
    return [x, y];
  }
  
  render() {
    if (!this.canvas || !this.context) return;
    this.fillBackground();
    if (!this.song) return;
    this.drawGuides();
    this.drawEvents();
    this.blotBottom();
  }
  
  /* Background.
   *********************************************************************/
  
  fillBackground() {
    this.context.fillStyle = "#222";
    this.context.fillRect(0, 0, this.canvas.width, this.canvas.height);
  }
  
  drawGuides() {
    this.drawOctaveRibbons();
    this.drawNoteLines();
    this.drawBeatLines();
  }
  
  drawOctaveRibbons() {
    this.context.fillStyle = "#282828";
    const fillHeight = this.noteHeight * 12;
    const stepHeight = fillHeight * 2;
    for (let y=-this.scroll[1]; y<this.canvas.height; y+=stepHeight) {
      this.context.fillRect(0, y, this.canvas.width, fillHeight);
    }
  }
  
  drawNoteLines() {
    this.context.beginPath();
    for (let y=-this.scroll[1]; y<this.canvas.height; y+=this.noteHeight) {
      this.context.moveTo(0, y);
      this.context.lineTo(this.canvas.width, y);
    }
    this.context.strokeStyle = "#111";
    this.context.stroke();
  }
  
  drawBeatLines() {
    this.context.beginPath();
    let x = -this.scroll[0] % this.beatWidth;
    for (; x<this.canvas.width; x+=this.beatWidth) {
      this.context.moveTo(x, 0);
      this.context.lineTo(x, this.canvas.height);
    }
    this.context.strokeStyle = "#111";
    this.context.stroke();
  }
  
  blotBottom() {
    this.context.fillStyle = "#000";
    this.context.fillRect(0, 128 * this.noteHeight - this.scroll[1], this.canvas.width, this.canvas.height);
  }
  
  /* Events.
   ********************************************************************/
   
  drawEvents() {
    const [eventp, eventc] = this.getEventIndices();
    // We create a Z order for events right here:
    this.drawDisabledImpulseEvents(eventp, eventc);
    this.drawDisabledNoteEvents(eventp, eventc);
    this.drawEnabledImpulseEvents(eventp, eventc);
    this.drawEnabledNoteEvents(eventp, eventc);
  }
  
  // [p,c] in (this.song.events), which events should we draw.
  getEventIndices() {
    const leftTicks = (this.scroll[0] * this.song.ticksPerQnote) / this.beatWidth - this.song.longestEvent;
    const rightTicks = ((this.scroll[0] + this.canvas.width) * this.song.ticksPerQnote) / this.beatWidth;
    let leftp = this.song.searchEvents(leftTicks);
    if (leftp < 0) leftp = -leftp - 1;
    let rightp = this.song.searchEvents(rightTicks);
    if (rightp < 0) rightp = - rightp - 1;
    return [leftp, rightp - leftp];
  }
  
  eventIsEnabled(event) {
    if (this.channelFilter !== "all") {
      if (this.channelFilter !== event.channel) return false;
    }
    switch (this.eventFilter) {
      case "notes": {
          if (event.type !== Song.EVENT_NOTE_ON) return false;
        } break;
      case "inputs": {
          if (event.type !== Song.EVENT_NOTE_ON) return false;
          if (!(event.program & 0x38)) return false;
        } break;
    }
    return true;
  }
  
  drawDisabledImpulseEvents(p, c) {
    this.context.globalAlpha = 0.5;
    for (; c-->0; p++) {
      const event = this.song.events[p];
      if (event.type === Song.EVENT_NOTE_ON) continue;
      if (this.eventIsEnabled(event)) continue;
      this.drawImpulse(event, false);
    }
    this.context.globalAlpha = 1.0;
  }
  
  drawDisabledNoteEvents(p, c) {
    this.context.globalAlpha = 0.5;
    for (; c-->0; p++) {
      const event = this.song.events[p];
      if (event.type !== Song.EVENT_NOTE_ON) continue;
      if (this.eventIsEnabled(event)) continue;
      this.drawNote(event, false);
    }
    this.context.globalAlpha = 1.0;
  }
  
  drawEnabledImpulseEvents(p, c) {
    for (; c-->0; p++) {
      const event = this.song.events[p];
      if (event.type === Song.EVENT_NOTE_ON) continue;
      if (!this.eventIsEnabled(event)) continue;
      this.drawImpulse(event, true);
    }
  }
  
  drawEnabledNoteEvents(p, c) {
    for (; c-->0; p++) {
      const event = this.song.events[p];
      if (event.type !== Song.EVENT_NOTE_ON) continue;
      if (!this.eventIsEnabled(event)) continue;
      this.drawNote(event, true);
    }
  }
  
  drawImpulse(event, enabled) {
    const x = (event.time * this.beatWidth) / this.song.ticksPerQnote - this.scroll[0];
    this.context.beginPath();
    this.context.moveTo(x, 0);
    this.context.lineTo(x, this.canvas.height);
    if (event === this.highlightEvent) this.context.strokeStyle = "#0ff";
    else if (enabled) this.context.strokeStyle = "#555";
    else this.context.strokeStyle = "#444";
    this.context.stroke();
  }
  
  drawNote(event, enabled) {
    const x = (event.time * this.beatWidth) / this.song.ticksPerQnote - this.scroll[0];
    let w = (event.duration * this.beatWidth) / this.song.ticksPerQnote;
    if (w < NOTE_WIDTH_MIN) w = NOTE_WIDTH_MIN;
    const y = (127 - event.a) * this.noteHeight - this.scroll[1];
    if (enabled) {
      switch ((event.program >> 3) & 7) { // color by input
        case 0: this.context.fillStyle = "#ccc"; break;
        case 1: this.context.fillStyle = "#8a37c6"; break;
        case 2: this.context.fillStyle = "#bc9519"; break;
        case 3: this.context.fillStyle = "#1b73af"; break;
        case 4: this.context.fillStyle = "#aa1940"; break;
        case 5: this.context.fillStyle = "#2ba116"; break;
        case 6: this.context.fillStyle = "#ffc"; break;
        case 7: this.context.fillStyle = "#ffc"; break;
      }
      this.context.strokeStyle = "#fff";
    } else {
      this.context.fillStyle = "#444";
      this.context.strokeStyle = "#555";
    }
    if (event === this.highlightEvent) this.context.strokeStyle = "#0ff";
    this.context.fillRect(x, y, w, this.noteHeight);
    this.context.strokeRect(x, y, w, this.noteHeight);
  }
}

// NB Not singleton. Each injection gets a fresh instance.
