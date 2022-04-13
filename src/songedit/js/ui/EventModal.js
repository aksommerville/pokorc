/* EventModal.js
 * Right-click or 'd' on a song event.
 */
 
import { Dom } from "/js/app/Dom.js";
import { Song } from "/js/song/Song.js";
import { Midi } from "/js/song/Midi.js";

export class EventModal {
  static getDependencies() {
    return [HTMLElement, Dom, Window];
  }
  constructor(element, dom, window) {
    this.element = element;
    this.dom = dom;
    this.window = window;
    
    this.ondelete = (event) => {};
    this.ondirty = () => {};
    
    this.event = null;
    this.song = null;
    
    this.buildUi();
  }
  
  setEvent(event, song) {
    this.event = event;
    if (song) this.song = song;
    this.setVisibility(this.event?.type);
    this.writeForm(this.event || {});
  }
  
  /* Model <~> Form
   *****************************************************************/
   
  setVisibility(type) {
    // Select by label, not key. Some keys are duplicated with different labels (a,b).
    const _ = (label, visible) => {
      const tr = this.element.querySelector(`tr[data-label='${label}']`);
      if (!tr) return;
      if (visible) tr.classList.remove("hidden");
      else tr.classList.add("hidden");
    };
    _("Time", true);
    _("Track", true);
    _("Channel", true);
    _("Type", true);
    // FYI There are no EVENT_NOTE_OFF.
    _("Note", type === Song.EVENT_NOTE_ON || type === Song.EVENT_NOTE_ADJUST);
    _("Velocity", type === Song.EVENT_NOTE_ON || type === Song.EVENT_NOTE_ADJUST);
    _("Duration", type === Song.EVENT_NOTE_ON);
    _("Off Velocity", type === Song.EVENT_NOTE_ON);
    _("Key", type === Song.EVENT_CONTROL);
    _("Value", type === Song.EVENT_CONTROL);
    _("Program", type === Song.EVENT_PROGRAM);
    _("Pressure", type === Song.EVENT_PRESSURE);
    _("Wheel", type === Song.EVENT_WHEEL);
    _("Meta ID", type === Song.EVENT_META);
    _("Body", type === Song.EVENT_META || type === Song.EVENT_SYSEX);
    _("(Program)", type === Song.EVENT_NOTE_ON);
    _("Wave", type === Song.EVENT_NOTE_ON);
    _("Input", type === Song.EVENT_NOTE_ON);
  }
   
  writeForm(event) {
    const _ = (key, value) => {
      for (const input of this.element.querySelectorAll(`*[name='${key}']`)) {
        input.value = value;
        this.rewriteDetail(key);
      }
    };
    _("time", event.time);
    _("track", event.track);
    _("channel", event.channel);
    _("type", event.type);
    _("a", event.a);
    _("b", event.b);
    _("duration", event.duration);
    _("offVelocity", event.offVelocity);
    _("wheel", event.wheel);
    _("meta", event.meta);
    _("body", event.body);
    _("program", event.program);
    _("wave", event.program);
    _("input", event.b & 0xf0);
  }
  
  readForm() {
    const event = {};
    for (const input of this.element.querySelectorAll("tr[data-key]")) {
      const key = tr.getAttribute("data-key");
      const value = tr.querySelector("td.value input,td.value select,td.value textarea")?.value;
      event[key] = value;
    }
    return event;
  }
  
  rewriteDetail(key) {
    const text = this.describeField(this.event, key);
    for (const td of this.element.querySelectorAll(`.detail[data-key='${key}']`)) {
      td.innerText = text;
    }
  }
  
  describeField(event, key) {
    if (!event) return "";
    switch (key) {
      case "time": return this.mmssFromTicks(event.time);
      case "type": return ""; // The menu already shows pretty names; underlying value is not interesting.
      case "track": return "";
      case "channel": return `MIDI ${event.channel + 1}`; // Reinforce that our indices are zero-based, but MIDI is conventionally one-based.
      case "duration": return this.mmssFromTicks(event.duration);
      case "offVelocity": return "";
      case "meta": return this.metaIdName(event.meta);
      case "body": return "";//TODO opportunity to format based on meta id... does anyone care?
      case "a": switch (event.type) {
          case Song.EVENT_NOTE_ON: return Midi.noteName(event.a);
          case Song.EVENT_NOTE_OFF: return Midi.noteName(event.a);
          case Song.EVENT_NOTE_ADJUST: return Midi.noteName(event.a);
          case Song.EVENT_CONTROL: return Midi.controlKeyName(event.a);
          case Song.EVENT_PROGRAM: return Midi.describePocketOrchestraProgram(event.a);
          case Song.EVENT_PRESSURE: return "";
        } break;
      case "b": return ""; // velocity, control value... never interesting
      case "wheel": return this.describeWheel(event.wheel);
      case "program": return Midi.describePocketOrchestraProgram(event.program);
    }
    return "";
  }
  
  mmssFromTicks(ticks) {
    if (!this.song) return "";
    const sf = (ticks * this.song.secondsPerQnote) / this.song.ticksPerQnote;
    let ms = Math.round(sf * 1000);
    let s = Math.floor(ms / 1000);
    let min = Math.floor(s / 60);
    s %= 60;
    ms %= 1000;
    return `${min}:${s.toString().padStart(2, '0')}.${ms.toString().padStart(3, '0')}`;
  }
  
  metaIdName(id) {
    switch (id) {
      // Probably we don't care.
    }
    return "";
  }
  
  describeWheel(wheel) {
    if (wheel < -8192) return "! below min !";
    if (wheel > 8191) return "! above max !";
    const norm = wheel / 8192.0;
    return norm.toFixed(3);
  }
  
  /* Build UI.
   ***************************************************************/
  
  buildUi() {
    this.element.innerHTML = "";
    const form = this.dom.spawn(this.element, "FORM");
    const table = this.dom.spawn(form, "TABLE");
    
    this.addRowInteger(table, "time", "Time", true);
    this.addRowInteger(table, "track", "Track", true);
    this.addRowInteger(table, "channel", "Channel", true);
    const typeSelect = this.addRowSelect(table, "type", "Type", EventModal.EVENTS_PICKLIST, true);
    typeSelect.addEventListener("input", () => this.onTypeChange());
    
    this.addRowInteger(table, "a", "Note", false);
    this.addRowInteger(table, "b", "Velocity", false);
    this.addRowInteger(table, "duration", "Duration", false);
    this.addRowInteger(table, "offVelocity", "Off Velocity", false);
    
    this.addRowInteger(table, "a", "Key", false);
    this.addRowInteger(table, "b", "Value", false);
    
    this.addRowInteger(table, "a", "Program", false);
    this.addRowInteger(table, "program", "(Program)", false); // we also record effective program id in Note On
    this.addRowSelect(table, "wave", "Wave", EventModal.WAVES_PICKLIST, false);
    this.addRowSelect(table, "input", "Input", EventModal.INPUTS_PICKLIST, false);
    
    this.addRowInteger(table, "a", "Pressure", false);
    
    this.addRowInteger(table, "wheel", "Wheel", false);
    
    this.addRowInteger(table, "meta", "Meta ID", false);
    this.addRowHexDump(table, "body", "Body", false);
    
    form.addEventListener("input", (event) => this.onInput(event));
    
    const deleteButton = this.dom.spawn(this.element, "INPUT",  null, { type: "button", value: "Delete" });
    deleteButton.addEventListener("click", () => this.onDelete());
  }
  
  addRowInteger(table, key, label, visible) {
    const td = this.addRow(table, key, label, visible);
    const input = this.dom.spawn(td, "INPUT", null, { name: key, type: "number" });
    return input;
  }
  
  addRowSelect(table, key, label, picklist, visible) {
    const td = this.addRow(table, key, label, visible);
    const select = this.dom.spawn(td, "SELECT", null, { name: key });
    for (const { value, label } of picklist) {
      const option = this.dom.spawn(select, "OPTION", null, { value }, label);
    }
    return select;
  }
  
  addRowHexDump(table, key, label, visible) {
    const td = this.addRow(table, key, label, visible);
    const textarea = this.dom.spawn(td, "TEXTAREA", null, { name: key });
    return textarea;
  }
  
  addRow(table, key, label, visible) {
    const tr = this.dom.spawn(table, "TR", visible ? [] : ["hidden"], { "data-key": key, "data-label": label });
    const tdKey = this.dom.spawn(tr, "TD", ["key"], null, label);
    const tdValue = this.dom.spawn(tr, "TD", ["value"]);
    const tdDetail = this.dom.spawn(tr, "TD", ["detail"], { "data-key": key });
    return tdValue;
  }
  
  /* Events.
   **********************************************************/
   
  onTypeChange() {
    this.setVisibility(+this.element.querySelector("select[name='type']").value);
  }
  
  onInput(event) {
    const key = event.target.name;
    const value = event.target.value;
    if (!this.event) return;
    switch (key) {
      case "wave": this.event.program = value & 0x07; break;
      case "input": this.event.b = value ? (value | 0x08) : 0x01; break;
      default: this.event[key] = value; break;
    }
    this.rewriteDetail(event.target.name);
    this.ondirty();
  }
  
  onDelete() {
    if (!this.event) return;
    if (!this.window.confirm("Really delete this event?")) return;
    this.ondelete(this.event);
    this.dom.dismissModal(this);
  }
}

EventModal.EVENTS_PICKLIST = [
  { value: Song.EVENT_NOTE_OFF, label: "Note Off" },
  { value: Song.EVENT_NOTE_ON, label: "Note On" },
  { value: Song.EVENT_NOTE_ADJUST, label: "Note Adjust" },
  { value: Song.EVENT_CONTROL, label: "Control" },
  { value: Song.EVENT_PROGRAM, label: "Program" },
  { value: Song.EVENT_PRESSURE, label: "Pressure" },
  { value: Song.EVENT_WHEEL, label: "Wheel" },
  { value: Song.EVENT_SYSEX, label: "Sysex" },
  { value: Song.EVENT_META, label: "Meta" },
];

EventModal.WAVES_PICKLIST = [
  { value: 0, label: "0" },
  { value: 1, label: "1" },
  { value: 2, label: "2" },
  { value: 3, label: "3" },
  { value: 4, label: "4" },
  { value: 5, label: "5" },
  { value: 6, label: "6" },
  { value: 7, label: "7" },
];

EventModal.INPUTS_PICKLIST = [
  { value: 0x00, label: "None" },
  { value: 0x30, label: "Left" },
  { value: 0x40, label: "Up" },
  { value: 0x50, label: "Right" },
  { value: 0x60, label: "B" },
  { value: 0x70, label: "A" },
];
