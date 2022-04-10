/* FileWidget.js
 * Presents "Load" and "Save" buttons, records the file's path, loads and saves.
 * General purpose.
 */

import { Dom } from "/js/app/Dom.js";

export class FileWidget {
  static getDependencies() {
    return [HTMLElement, Dom, Window];
  }
  constructor(element, dom, window) {
    this.element = element;
    this.dom = dom;
    this.window = window;
    
    this.onload = (arrayBuffer) => {};
    this.onsave = () => null/* ArrayBuffer | Promise<ArrayBuffer> | null */;
    
    this.fileName = "";
    
    this.buildUi();
  }
  
  buildUi() {
    this.element.innerHTML = "";
    
    const saveButton = this.dom.spawn(this.element, "INPUT", ["saveButton"], { type: "button", value: "Save" });
    saveButton.addEventListener("click", () => this.onSave());
    
    const fileInput = this.dom.spawn(this.element, "INPUT", ["fileInput"], { type: "file" });
    fileInput.addEventListener("change", (event) => this.onFileChange(event));
  }
  
  onFileChange(event) {
    if (event.target.files?.length !== 1) return;
    this.fileName = event.target.files[0].name;
    const url = this.window.URL.createObjectURL(event.target.files[0]);
    this.window.fetch(url)
      .then((response) => response.arrayBuffer())
      .then((serial) => this.onload(serial))
      .then(() => this.onLoadOk())
      .catch((error) => this.onLoadError(error));
  }
  
  onSave() {
    let promise = this.onsave();
    if (!promise) return;
    if (!(promise instanceof Promise)) promise = Promise.resolve(promise);
    promise.then((serial) => {
      const blob = new this.window.File([serial], this.fileName || "untitled.mid", { type: "audio/midi" });
      const url = this.window.URL.createObjectURL(blob);
      this.window.open(url)?.addEventListener("load", () => {
        this.window.URL.revokeObjectURL(url);
      });
    }).catch((error) => this.onSaveError(error));
  }
  
  onLoadOk() {
  }
  
  onLoadError(error) {
    console.log(`FileWidget.onLoadError`, error);
  }
  
  onSaveError(error) {
    console.log(`FileWidget.onSaveError`, error);
  }
}
