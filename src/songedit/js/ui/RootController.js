import { Dom } from "/js/app/Dom.js";
import { FileWidget } from "/js/ui/FileWidget.js";
import { Toolbar } from "/js/ui/Toolbar.js";
import { Editor } from "/js/ui/Editor.js";
import { Song } from "/js/song/Song.js";

export class RootController {
  static getDependencies() {
    return [HTMLElement, Dom, Window];
  }
  constructor(element, dom, window) {
    this.element = element;
    this.dom = dom;
    this.window = window;
    
    this.fileWidget = null;
    this.song = null;
    this.toolbar = null;
    this.editor = null;
    
    this.window.addEventListener("keydown", this.keyDownHandler = (event) => this.onKeyDown(event));
    
    this.buildUi();
    
    this.toolbar.onInput(); // force an initial input, makes the editor adjust
  }
  
  onRemoveFromDom() {
    if (this.keyDownHandler) {
      this.window.removeEventListener("keydown", this.keyDownHandler);
      this.keyPressHandler = null;
    }
  }
  
  /* Build UI.
   ********************************************************/
  
  buildUi() {
    this.element.innerHTML = "";
    
    const topBar = this.dom.spawn(this.element, "DIV", ["topBar"]);
    
    this.fileWidget = this.dom.spawnController(topBar, FileWidget);
    this.fileWidget.onload = (serial) => this.onLoadFile(serial);
    this.fileWidget.onsave = () => this.onSaveFile();
    
    this.toolbar = this.dom.spawnController(topBar, Toolbar);
    
    this.editor = this.dom.spawnController(this.element, Editor);
    
    this.toolbar.oninput = (model) => this.onToolbarInput(model);
    this.toolbar.onoptimize = () => this.editor.onOptimize();
    this.toolbar.onquantize = () => this.editor.onQuantize();
  }
  
  /* UI events.
   **********************************************************/
   
  onLoadFile(serial) {
    this.song = new Song(serial);
    this.editor.reset(this.song);
  }
  
  onSaveFile() {
    if (!this.song) return null;
    return this.song.encode();
  }
  
  onToolbarInput(model) {
    this.editor.onToolbarInput(model);
  }
  
  onKeyDown(event) {
    // !!! Avoid interfering with regular DOM stuff. Don't use Tab, Space, Arrows, etc !!!
    if (this.dom.getTopModal()) {
      // ...and don't do anything but dismiss-modal, if a modal is visible.
      switch (event.key) {
        case "Escape": this.dom.dismissModal(); event.preventDefault(); break;
      }
      return;
    }
    switch (event.key) {
      case "Backspace":
      case "Delete": this.editor.onDelete(); event.preventDefault(); break;
      case "d": this.editor.onOpenDetails(); event.preventDefault(); break;
      case "t": this.editor.tattleSelected(); event.preventDefault(); break;
    }
  }
}
