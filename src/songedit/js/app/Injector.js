export class Injector {
  static getDependencies() {
    return [Window, Document];
  }
  constructor(window, document) {
    this.window = window;
    this.document = document;
    
    this.singletons = {
      Injector: this,
      Window: this.window,
      Document: this.document,
    };
    this.inProgress = [];
  }
  
  getInstance(cls, ...overrides) {
    return this._instantiate(cls, overrides || []);
  }
  
  _instantiate(cls, overrides) {
    if (this.singletons[cls.name]) return this.singletons[cls.name];
    
    for (const override of overrides) {
      if (override.constructor === cls) return override;
      if (cls.isPrototypeOf(override.constructor)) return override;
    }
    
    if (this.inProgress.indexOf(cls.name) >= 0) {
      throw new Error(`Dependency loop involving these classes: ${JSON.stringify(this.inProgress)}`);
    }
    this.inProgress.push(cls.name);
    
    const deps = [];
    for (const depClass of cls.getDependencies?.() || []) {
      const dep = this._instantiate(depClass, overrides);
      deps.push(dep);
    }
    
    const instance = new cls(...deps);
    
    if (this.inProgress.pop() !== cls.name) {
      throw new Error(`Dependency loop tracker out of sync, popping '${cls.name}'`);
    }
    if (cls.singleton) this.singletons[cls.name] = instance;
    return instance;
  }
  
}

Injector.singleton = true;
