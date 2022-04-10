import { Injector } from "/js/app/Injector.js";
import { Dom } from "/js/app/Dom.js";
import { RootController } from "/js/ui/RootController.js";

window.addEventListener("load", () => {
  const injector = new Injector(window, document);
  const dom = injector.getInstance(Dom, document.body);
  const root = dom.spawnController(document.body, RootController);
});
