import { Injector } from "/js/app/Injector.js";

export class Dom {
  static getDependencies() {
    return [Window, Document, HTMLElement, Injector];
  }
  constructor(window, document, element, injector) {
    this.window = window;
    this.document = document;
    this.element = element;
    this.injector = injector;
    
    this.element.innerHTML = "";
    this.mutationObserver = new this.window.MutationObserver((events) => this.onMutation(events));
    this.mutationObserver.observe(this.element, { childList: true });
  }
  
  /* Convenience to create a new DOM element and append to some parent.
   */
  spawn(parent, tag, classes, attributes, text) {
    const element = this.document.createElement(tag);
    if (classes) for (const cname of classes) element.classList.add(cname);
    if (attributes) for (const k of Object.keys(attributes)) {
      element.setAttribute(k, attributes[k]);
    }
    if (text) element.innerText = text;
    parent.appendChild(element);
    return element;
  }
  
  /* Create a new element, attach to parent, and attach a new controller instance to it.
   */
  spawnController(parent, cls, ...overrides) {
    const tag = this._htmlTagForControllerClass(cls);
    const element = this.spawn(parent, tag, [cls.name]);
    const controller = this.injector.getInstance(cls, element, ...overrides);
    this._attachController(element, controller);
    return controller;
  }
  
  /* Create a new modal container and attach a new controller to it.
   */
  spawnModal(cls, ...overrides) {
    const frame = this._newModalFrame();
    return this.spawnController(frame, cls, ...overrides);
  }
  
  /* Return an existing modal controller of the given class, if one exists.
   * (Javascript class, not CSS class).
   */
  findModalOfClass(cls) {
    const element = this.document.body.querySelector(`.modalFrame .${cls.name}`);
    if (!element) return null;
    return element._romassist_controller;
  }
  
  getTopModal() {
    const frames = Array.from(this.document.body.querySelectorAll(".modalContainer .modalFrame"));
    if (frames.length < 1) return null;
    return frames[frames.length - 1];
  }
  
  /* Provide a controller to dismiss specifically that one, otherwise dismisses the top.
   */
  dismissModal(controller) {
    const container = this.document.body.querySelector(".modalContainer");
    if (!container) return;
    const frames = Array.from(container.querySelectorAll(".modalFrame"));
    if (controller) {
      const p = frames.findIndex((frame) => frame.querySelector(`.${controller.constructor.name}`)?._romassist_controller === controller);
      if (p < 0) return;
      if (p < frames.length - 1) {
        container.remove(frames[p]);
      } else if (frames.length === 1) {
        container.remove();
      } else {
        const newTop = frames[frames.length - 2];
        frames[frames.length - 1].remove();
        const blotter = container.querySelector(".modalBlotter");
        container.insertBefore(blotter, newTop);
      }
    } else if (frames.length < 2) {
      container.remove();
    } else {
      const newTop = frames[frames.length - 2];
      frames[frames.length - 1].remove();
      const blotter = container.querySelector(".modalBlotter");
      container.insertBefore(blotter, newTop);
    }
  }
  
  /* Convenience to spawn a button with label and callback.
   */
  spawnButton(parent, label, callback) {
    const button = this.spawn(parent, "INPUT", null, { type: "button", value: label });
    if (callback) button.addEventListener("click", callback);
    return button;
  }
  
  /* Kind of like modals.
   * Show a small text tooltip near the pointer, and arrange for it to disappear after a tasteful interval.
   * Damn, it's not possible to poll the last mouse position. Sorry. You have to provide it.
   */
  startMouseyToast(message, x, y) {
    if (!message) return;
    const toast = this.spawn(this.document.body, "DIV", ["toast"], null, message);
    toast.style.left = `${x}px`;
    toast.style.top = `${y}px`;
    toast.addEventListener("animationend", () => toast.remove());
  }
  
  /* Attach existing DOM element to existing controller instance.
   * You must also provide this element to the controller's constructor.
   */
  _attachController(element, controller) {
    element._romassist_controller = controller;
  }
  
  _htmlTagForControllerClass(cls) {
    for (const depClass of cls.getDependencies?.()) {
      const match = depClass.name.match(/^HTML([a-zA-Z]*)Element$/);
      if (!match) continue;
      if (!match[1]) return "DIV"; // HTMLElement => DIV
      if (match[1] === "UList") return "UL";
      return match[1].toUpperCase();
    }
    return "DIV"; // unknown => DIV
  }
  
  _newModalFrame() {
    const container = this._requireModalContainer();
    const frame = this.spawn(container, "DIV", ["modalFrame"]);
    frame.addEventListener("click", (event) => event.stopPropagation());
    return frame;
  }
  
  _requireModalContainer() {
    let container = this.document.body.querySelector(".modalContainer");
    if (!container) {
      container = this.spawn(this.document.body, "DIV", ["modalContainer"]);
    }
    let blotter = container.querySelector(".modalBlotter");
    if (blotter) {
      container.insertBefore(blotter, null);
    } else {
      blotter = this.spawn(container, "DIV", ["modalBlotter"]);
      blotter.addEventListener("click", () => this.dismissModal());
    }
    return container;
  }
  
  onMutation(events) {
    for (const event of events) {
      if (!event.removedNodes) continue;
      for (const element of event.removedNodes) {
        const controller = element._romassist_controller;
        if (!controller) continue;
        delete element._romassist_controller;
        if (controller.onRemoveFromDom) {
          controller.onRemoveFromDom();
        }
      }
    }
  }
}

Dom.singleton = true;
