Central App

Dieses Modul kann dazu verwendet werden, um als zentrale Steuerungseinheit der Einzel-
komponenten zu agieren. 
Es soll bei instanziierung des Objektes durch eine ausgefüllte struct mitgeteilt werden, 
welche Module vorhanden sind und gesteuert werden sollen: 

struct ModuleFlags {
    bool network = false, 
    bool parser = false,
    bool call = false, 
    bool reg = false,
    bool media = false, 
    bool logwriter = false
}

Per default sind alle Module deaktiviert und es liegt in der Verantwortung des Entwicklers
welche Module aktiviert werden und wie die Aufgaben der nicht-aktiven umgesetzt werden. 


Wenn die Central-App einzeln genutzt werden soll, muss der Code entsprechend angepasst und 
überarbeitet werden. Eine Ready-To-Use-Möglichkeit ist aktuell nicht im Scope. 