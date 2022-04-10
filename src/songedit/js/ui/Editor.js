/* Editor.js
 * Child of RootController.
 * The whole UI except the narrow top bar.
 *
 * We create a giant dummy div the size of the whole song, for scrolling purposes.
 * Then a canvas on top of that, which never moves, but redraws according to the dummy's scroll position.
 */

import { Dom } from "/js/app/Dom.js";
import { ChartRenderer } from "/js/ui/ChartRenderer.js";
import { Song } from "/js/song/Song.js";
import { Midi } from "/js/song/Midi.js";
import { SongOperations } from "/js/song/SongOperations.js";
import { EventModal } from "/js/ui/EventModal.js";
import { ParametersModal } from "/js/ui/ParametersModal.js";

export class Editor {
  static getDependencies() {
    return [HTMLElement, Dom, Window, ChartRenderer, SongOperations];
  }
  constructor(element, dom, window, chartRenderer, songOperations) {
    this.element = element;
    this.dom = dom;
    this.window = window;
    this.chartRenderer = chartRenderer;
    this.songOperations = songOperations;
    
    this.song = null;
    this.drawPending = false;
    this.mouseMoveHandler = null;
    this.mouseUpHandler = null;
    this.draggingEvent = null;
    this.dragAnchor = [0, 0]; // (x,y) in document coordinates where it started
    this.dragTimeOffset = 0;
    
    this.element.addEventListener("mousedown", (event) => this.onMouseDown(event));
    this.element.addEventListener("contextmenu", (event) => this.onContextMenu(event));
    
    this.buildUi();
  }
  
  onRemoveFromDom() {
    this.dropMouseHandlers();
  }
  
  /* Setup.
   *************************************************/
  
  reset(song) {
    this.song = song;
    this.chartRenderer.song = song;
    this.resizeDummy();
    this.forceInitialScroll();
    this.redrawLater();
  }
  
  forceInitialScroll() {
    const scroller = this.element.querySelector(".scroller");
    scroller.scrollLeft = 0;
    scroller.scrollTop = scroller.scrollTopMax / 2;
  }
  
  resizeDummy() {
    const scrollBarWidth = 16; // TODO can we do better than guessing?
    const maxw = this.element.clientWidth - scrollBarWidth;
    const maxh = this.element.clientHeight - scrollBarWidth;
    const [w, h] = this.chartRenderer.measure(maxw, maxh);
    const dummy = this.element.querySelector(".dummy");
    dummy.style.width = `${w}px`;
    dummy.style.height = `${h}px`;
  }
  
  /* Build UI.
   ************************************************/
   
  buildUi() {
    this.element.innerHTML = "";
    const canvas = this.dom.spawn(this.element, "CANVAS");
    const scroller = this.dom.spawn(this.element, "DIV", ["scroller"]);
    scroller.addEventListener("scroll", (event) => this.redrawLater());
    const dummy = this.dom.spawn(scroller, "DIV", ["dummy"]);
  }
  
  /* Render.
   ***************************************************/
   
  redrawLater() {
    if (this.drawPending) return;
    this.drawPending = true;
    window.requestAnimationFrame(() => this.redrawNow());
  }
  
  redrawNow() {
    this.drawPending = false;
    const scroller = this.element.querySelector(".scroller");
    const canvas = this.element.querySelector("canvas");
    canvas.width = canvas.clientWidth;
    canvas.height = canvas.clientHeight;
    const context = canvas.getContext("2d");
    this.chartRenderer.canvas = canvas;
    this.chartRenderer.context = context;
    this.chartRenderer.scroll[0] = scroller.scrollLeft;
    this.chartRenderer.scroll[1] = scroller.scrollTop;
    this.chartRenderer.render();
  }
  
  /* Events.
   ***********************************************************/
   
  onToolbarInput(model) {
    const focusTime = this.chartRenderer.timeForX(this.element.clientWidth / 2);
    const focusNote = this.chartRenderer.noteForY(this.element.clientHeight / 2);
    
    this.chartRenderer.zoom(model.x, model.y);
    
    const [scrollX, scrollY] = this.chartRenderer.measureScrollForFocus(focusTime, focusNote);
    const scroller = this.element.querySelector(".scroller");
    scroller.scrollLeft = scrollX;
    scroller.scrollTop = scrollY;
    
    this.chartRenderer.setVisibility(model.channel, model.events);
    this.resizeDummy();
    this.redrawLater();
  }
  
  dropMouseHandlers() {
    if (this.mouseMoveHandler) {
      this.window.removeEventListener("mousemove", this.mouseMoveHandler);
      this.mouseMoveHandler = null;
    }
    if (this.mouseUpHandler) {
      this.window.removeEventListener("mouseup", this.mouseUpHandler);
      this.mouseUpHandler = null;
    }
  }
  
  onContextMenu(event) {
    event.preventDefault();
  }
  
  onMouseDown(event) {
    event.preventDefault();
    event.stopPropagation();
    this.dropMouseHandlers();
    if (this.chartRenderer.highlightEvent) {
      this.chartRenderer.highlightEvent = null;
      this.redrawLater();
    }
    
    const bounds = this.element.getBoundingClientRect();
    const x = event.x - bounds.x;
    const y = event.y - bounds.y;
    const songEvent = this.chartRenderer.findEventAtPoint(x, y);

    switch (event.button) {
      case 0: {
          if (!songEvent) return;
          this.chartRenderer.highlightEvent = songEvent;
          this.draggingEvent = songEvent;
          this.dragAnchor = [x, y];
          this.dragTimeOffset = this.chartRenderer.timeForX(x) - songEvent.time;
          this.window.addEventListener("mousemove", this.mouseMoveHandler = (event) => this.onMouseMove(event));
          this.window.addEventListener("mouseup", this.mouseUpHandler = (event) => this.onMouseUp(event));
          this.redrawLater();
          this.dom.startMouseyToast(this.describeEventForToast(songEvent), event.x, event.y);
        } break;
      case 1: break; // middle button, it's here if we want it
      case 2: {
          if (!songEvent) return;
          this.chartRenderer.highlightEvent = songEvent;
          this.redrawLater();
          // Don't create a modal from this event handler; the blotter will try to process the right-click. ugh.
          this.window.setTimeout(() => {
            this.onOpenDetails();
          }, 10);
        } break;
    }
  }
  
  onMouseMove(event) {
    if (!this.draggingEvent) return;
    
    const bounds = this.element.getBoundingClientRect();
    const x = event.x - bounds.x;
    const y = event.y - bounds.y;
    const dx = x - this.dragAnchor[0];
    const dy = y - this.dragAnchor[1];
    
    const time = Math.round(this.chartRenderer.timeForX(x) - this.dragTimeOffset);
    if (time !== this.draggingEvent.time) {
      this.draggingEvent.time = time;
      this.song.shuffleForChangedTime(this.draggingEvent);
      this.redrawLater();
      this.onDirty();
    }
    
    if (this.draggingEvent.type === Song.EVENT_NOTE_ON) {
      const note = this.chartRenderer.noteForY(y);
      if (note !== this.draggingEvent.a) {
        this.draggingEvent.a = note;
        this.redrawLater();
        this.onDirty();
      }
    }
  }
  
  onMouseUp(event) {
    this.dropMouseHandlers();
    if (this.draggingEvent) {
      this.draggingEvent = null;
    }
  }
  
  onDirty() {
  }
  
  onDelete() {
    if (!this.song) return;
    if (!this.chartRenderer.highlightEvent) return;
    if (this.draggingEvent) return; // One thing at a time, please.
    this.song.removeEvent(this.chartRenderer.highlightEvent);
    this.chartRenderer.highlightEvent = null;
    this.redrawLater();
    this.onDirty();
  }
  
  onDeleteEvent(event) {
    if (!event || !this.song) return;
    if (this.draggingEvent) return; // One thing at a time, please.
    if (event === this.chartRenderer.highlightEvent) {
      this.chartRenderer.highlightEvent = null;
    }
    this.song.removeEvent(event);
    this.redrawLater();
    this.onDirty();
  }
  
  onOpenDetails() {
    if (!this.song) return;
    if (!this.chartRenderer.highlightEvent) return;
    const controller = this.dom.spawnModal(EventModal);
    controller.setEvent(this.chartRenderer.highlightEvent, this.song);
    controller.ondelete = (event) => this.onDeleteEvent(event);
    controller.ondirty = () => {
      this.redrawLater();
      this.onDirty();
    };
  }
  
  tattleSelected() {
    const message = this.describeEventForToast(this.chartRenderer.highlightEvent);
    if (!message) return;
    let [x, y] = this.chartRenderer.getPositionForEvent(this.chartRenderer.highlightEvent);
    const bounds = this.element.getBoundingClientRect();
    this.dom.startMouseyToast(message, bounds.x + x, bounds.y + y);
  }
  
  describeEventForToast(event) {
    if (!event) return "";
    switch (event.type) {
      case Song.EVENT_NOTE_ON: return `note ${Midi.noteName(event.a) || event.a}, chan ${event.channel}`;
      case Song.EVENT_NOTE_ADJUST: return `adjust note ${Midi.noteName(event.a) || event.a}, chan ${event.channel}`;
      case Song.EVENT_CONTROL: return `control ${event.a}=${event.b}, chan ${event.channel}`;
      case Song.EVENT_PROGRAM: return `program ${Midi.describePocketOrchestraProgram(event.a)}, chan ${event.channel}`;
      case Song.EVENT_PRESSURE: return `pressure ${event.a}, chan ${event.channel}`;
      case Song.EVENT_WHEEL: return `wheel ${event.wheel}, chan ${event.channel}`;
      case Song.EVENT_META: return `meta ${event.meta}, ${event.body?.byteLength} bytes`;
      case Song.EVENT_SYSEX: return `sysex ${event.body?.byteLength} bytes`;
    }
    return `type ${event.type}`;
  }
  
  onOptimize() {
    if (!this.song) return;
    if (this.draggingEvent) return;
    const controller = this.dom.spawnModal(ParametersModal);
    controller.setup("Optimize...", {
      removeLeadingSilence: "checkbox",
      removeTrailingSilence: "checkbox",
      removeChannelPressure: "checkbox",
      removeAftertouch: "checkbox",
      removeWheel: "checkbox",
      removeUnknownControl: "checkbox",
      removeRedundantProgram: "checkbox",
      removeText: "checkbox",
      removeUnknownMeta: "checkbox",
      removeUnknownSysex: "checkbox",
    }, {
      removeLeadingSilence: true,
      removeTrailingSilence: true,
      removeChannelPressure: true,
      removeAftertouch: true,
      removeWheel: true,
      removeUnknownControl: true,
      removeRedundantProgram: true,
      removeText: true,
      removeUnknownMeta: true,
      removeUnknownSysex: true,
    });
    controller.onsubmit = (params) => {
      this.chartRenderer.highlightEvent = null; // just to be safe
      this.songOperations.optimize(this.song, params);
      this.redrawLater();
      this.onDirty();
    };
  }
  
  onQuantize() {
    if (!this.song) return;
    console.log(`ok lets quantize`);//TODO
  }
}
