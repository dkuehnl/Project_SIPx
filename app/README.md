# SIPxApp

## Overview

The `SIPxApp` module acts as the **central control unit** for the entire SIPx framework.  
Its purpose is to combine and manage all modules that the developer wants to activate, based on a configuration struct passed during instantiation.

Instead of enforcing predefined behavior, `SIPxApp` simply coordinates the modules you explicitly enable — giving you full control over how much automation the application should handle.

---

## Module Flags

Modules are activated using a simple configuration struct:

```cpp
struct ModuleFlags {
    bool network = false;
    bool parser = false;
    bool call = false;
    bool reg = false;
    bool media = false;
    bool logwriter = false;
};
``` 

If a flag is set to `true`, the corresponding module is initialized and made available inside the `SIPxApp` instance.

By default, **all modules are disabled**, ensuring that nothing is initialized behind your back.
This makes the module ideal for developers who want fine-grained control and prefer enabling features explicitly.