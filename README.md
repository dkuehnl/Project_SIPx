# Project SIPx

## About

Project SIPx is **not** meant to be another ready-to-use SIP framework like the ones already available out there.  
The goal here is to build a **low-level SIP toolkit** aimed at developers, DevOps engineers, and anyone who needs **full control over a SIP stack**.

The framework is intentionally designed to impose as little logic as possible.  
Instead of forcing predefined behaviors, SIPx hands most decisions over to the user.  
Every component is kept minimal and implements only the absolute basics.

The goal is to provide a SIP stack that makes it easy not only to generate and handle standard SIP calls, but also to simulate **edge cases**, protocol violations, and non-RFC-compliant behavior.  
This makes testing other SIP components (e.g. IMS P-CSCF, S-CSCF, AS, etc.) in error and corner-case scenarios extremely straightforward.

> **Note:**  
> Users of this framework should have solid knowledge of SIP and IMS.

---

## Modules

The following modules will be available in the final version:

- **SIP Parser** (`/parser`)
- **Log Writer** (`/logwriter`)
- **Register Logic** (`/register`)
- **Call Logic** (`/call`)
- **Media Handling** (`/media`)
- **Core Application** (`/app`)
- **Network Module** (`/network`)

Each module is fully standalone and can be extracted and integrated into other projects.  
(Some minor modifications might be required depending on your setup.)

Additional details for each module can be found in their respective subdirectories.

---

## Roadmap

| Module    | Status        |
|-----------|---------------|
| Parser    | ✔️ done        |
| Logwriter | 🔧 in progress |
| Call      | ❌ not started |
| Register  | ❌ not started |
| Media     | ❌ not started |
| Network   | ❌ not started |
| App       | 🔧 in progress |

---

## Support

If you encounter any issues, feel free to open an Issue in the repository.  
Please provide as much detail as possible so I can look into the problem quickly.

---

## Contribution

Pull requests are welcome!  
However, I reserve the right to modify or decline proposals depending on how well they fit the project’s goals.
