/* Toolbar.js
 * Covers most of the narrow top bar (excluding FileWidget).
 */

import { Dom } from "/js/app/Dom.js";

export class Toolbar {
  static getDependencies() {
    return [HTMLElement, Dom];
  }
  constructor(element, dom) {
    this.element = element;
    this.dom = dom;
    
    this.oninput = (model) => {};
    
    // Actions menu...
    this.onoptimize = () => {};
    this.onquantize = () => {};
    this.onzapinput = () => {};
    this.onreformat = () => {};
    
    this.buildUi();
  }
  
  buildUi() {
    this.element.innerHTML = "";
    
    const form = this.dom.spawn(this.element, "FORM");
    
    const xlabel = this.dom.spawn(form, "LABEL", null, null, "X");
    const xinput = this.dom.spawn(xlabel, "INPUT", null, { name: "x", type: "range", min: 0, max: 1, step: 0.01, value: 0.5 });
    
    const ylabel = this.dom.spawn(form, "LABEL", null, null, "Y");
    const yinput = this.dom.spawn(ylabel, "INPUT", null, { name: "y", type: "range", min: 0, max: 1, step: 0.01, value: 0.5 });
    
    const channelSelect = this.dom.spawn(form, "SELECT", null, { name: "channel" });
    this.dom.spawn(channelSelect, "OPTION", null, { value: "all", selected: true }, "All");
    for (let i=0; i<16; i++) this.dom.spawn(channelSelect, "OPTION", null, { value: i }, i.toString());
    
    const eventSelect = this.dom.spawn(form, "SELECT", null, { name: "events" });
    this.dom.spawn(eventSelect, "OPTION", null, { value: "all", selected: true }, "All");
    this.dom.spawn(eventSelect, "OPTION", null, { value: "notes" }, "Notes");
    this.dom.spawn(eventSelect, "OPTION", null, { value: "inputs" }, "Inputs");
    
    form.addEventListener("input", (event) => this.onInput(event));
    
    const actionSelect = this.dom.spawn(this.element, "SELECT", ["action"]);
    this.dom.spawn(actionSelect, "OPTION", null, { value: "default", disabled: true, selected: true }, "Actions...");
    this.dom.spawn(actionSelect, "OPTION", null, { value: "optimize" }, "Optimize...");
    this.dom.spawn(actionSelect, "OPTION", null, { value: "quantize" }, "Quantize...");
    this.dom.spawn(actionSelect, "OPTION", null, { value: "zapinput" }, "Clear Inputs");
    this.dom.spawn(actionSelect, "OPTION", null, { value: "reformat" }, "Reformat");
    actionSelect.addEventListener("change", () => this.onAction());
  }
  
  onInput(event) {
    const form = this.element.querySelector("form");
    const model = Array.from(form).reduce((a, v) => (a[v.name] = v.value, a), {});
    this.oninput(model);
  }
  
  onAction() {
    const select = this.element.querySelector("select.action");
    const action = select.value;
    select.value = "default";
    this.performAction(action);
  }
  
  performAction(action) {
    switch (action) {
      case "optimize": this.onoptimize(); break;
      case "quantize": this.onquantize(); break;
      case "zapinput": this.onzapinput(); break;
      case "reformat": this.onreformat(); break;
    }
  }
}
