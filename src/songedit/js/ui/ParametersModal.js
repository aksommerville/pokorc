/* ParametersModal.js
 * Generic input modal.
 * You provide all the content.
 * Fires a callback if submitted. If cancelled, you won't hear about it.
 */
 
import { Dom } from "/js/app/Dom.js";

export class ParametersModal {
  static getDependencies() {
    return [HTMLElement, Dom];
  }
  constructor(element, dom) {
    this.element = element;
    this.dom = dom;
    
    this.onsubmit = (model) => {};
    
    this.inputs = null;
    this.idDiscriminator = 1;
    this.instanceDiscriminator = ParametersModal.getInstanceDiscriminator();
  }
  
  static getInstanceDiscriminator() {
    if (!ParametersModal.nextInstanceDiscriminator) {
      ParametersModal.nextInstanceDiscriminator = 1;
    }
    return ParametersModal.nextInstanceDiscriminator++;
  }
  
  /* (prompt) is a plain text string.
   * (inputs) is an object whose members are:
   *   - Any <input type> string, eg "checkbox", "number", "text"
   *   - An object containing <input> attributes.
   *   - An array of strings, to produce a <select>. Strings are both value and label.
   * (model) is an option object conforming to (inputs).
   * The instance (model) is immutable to us; you'll get a fresh but similar instance back at submit.
   */
  setup(prompt, inputs, model) {
    this.inputs = inputs;
    this.buildUi(prompt, inputs);
    this.populate(model);
  }
  
  buildUi(prompt, inputs) {
    this.element.innerHTML = "";
    if (prompt) this.dom.spawn(this.element, "DIV", ["prompt"], null, prompt);
    const form = this.dom.spawn(this.element, "FORM");
    const table = this.dom.spawn(form, "TABLE");
    for (const key of Object.keys(inputs)) {
      this.addInput(table, key, inputs[key]);
    }
    const submitButton = this.dom.spawn(form, "INPUT", null, { type: "submit", value: "OK" });
    form.addEventListener("submit", (event) => this.onSubmit(event));
  }
  
  addInput(table, key, settings) {
    const id = `ParametersModal-${this.instanceDiscriminator}-${this.idDiscriminator++}`;
    const tr = this.dom.spawn(table, "TR");
    const tdKey = this.dom.spawn(tr, "TD", ["key"]);
    this.dom.spawn(tdKey, "LABEL", null, { for: id }, key);
    const tdValue = this.dom.spawn(tr, "TD", ["value"]);
    let input;
    
    // <input type>
    if (typeof(settings) === "string") {
      input = this.dom.spawn(tdValue, "INPUT", null, { id, name: key, type: settings });
      
    } else if (Array.isArray(settings)) {
      input = this.dom.spawn(tdValue, "SELECT", null, { id, name: key });
      for (const value of settings) {
        const option = this.dom.spawn(input, "OPTION", null, { value }, value);
      }
      
    } else if (typeof(settings) === "object") {
      input = this.dom.spawn(tdValue, "INPUT", null, { ...settings, id });
      
    } else {
      throw new Error(`ParametersModal: inappropriate value for '${key}'`);
    }
    
    return input;
  }
  
  populate(model) {
    if (!model) model = {};
    for (const input of this.element.querySelectorAll("td.value > input,td.value > select")) {
      const name = input.getAttribute("name");
      if (!name) continue;
      const value = model[name];
      switch (input.tagName) {
        case "INPUT": switch (input.getAttribute("type")) {
            case "checkbox": input.checked = value || false; break;
            default: input.value = value || "";
          } break;
        case "SELECT": input.value = value || ""; break;
      }
    }
  }
  
  readModel() {
    const model = {};
    for (const input of this.element.querySelectorAll("td.value > input,td.value > select")) {
      const name = input.getAttribute("name");
      if (!name) continue;
      switch (input.tagName) {
        case "INPUT": switch (input.getAttribute("type")) {
            case "checkbox": model[name] = !!input.checked; break;
            default: model[name] = input.value;
          } break;
        case "SELECT": model[name] = input.value; break;
      }
      switch (this.inputs[name]) {
        case "checkbox": model[name] = Boolean(model[name]); break;
        case "number": model[name] = +model[name]; break;
        case "text": model[name] = model[name]?.toString(); break;
      }
    }
    return model;
  }
  
  onSubmit(event) {
    event.preventDefault();
    const model = this.readModel();
    this.dom.dismissModal(this);
    this.onsubmit(model);
  }
}
