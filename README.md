# Firewall101

### Cybersecurity Incident Response Framework for an Educational Institution

Firewall101 is a college-project prototype designed to help students, faculty, administrators, and IT personnel understand, report, and respond to cybersecurity incidents on campus. It provides an educational website covering common threats, response procedures, responsibilities, escalation, and cybersecurity awareness, together with a lightweight C++ HTTP backend for storing incident reports.

> **Project status:** Educational prototype. Some uploaded frontend files are missing, and the provided admin dashboard is currently a browser-local demo rather than an integrated view of the C++ backend. Do not deploy this prototype as a production incident-response system.

## Objectives

- Encourage early identification and reporting of cybersecurity incidents.
- Explain an incident-response lifecycle adapted for educational institutions.
- Provide actionable guidance for common campus cybersecurity threats.
- Clarify incident-response roles, severity levels, and escalation paths.
- Demonstrate report submission, local storage, and incident status tracking.
- Improve cybersecurity awareness among students and staff.

## Features

| Feature | Description |
| --- | --- |
| Incident-response lifecycle | Interactive six-phase guide: Preparation, Detection and Analysis, Containment, Eradication, Recovery, and Post-Incident Review. |
| Incident types | Descriptions and warning signs for phishing, ransomware, malware, account compromise, data breaches, lost devices, denial-of-service attacks, and website defacement. |
| Response playbooks | Detailed guidance for phishing, ransomware, and lost/stolen devices; brief guidance for other incidents. |
| Roles and responsibilities | Duties of the Incident Response Team, Incident Manager, IT Help Desk, College Management, Data Protection/Legal Officer, and Communication Team. |
| Severity and escalation | Low, Medium, High, and Critical classifications with illustrative response targets and notification requirements. |
| Awareness resources | Password, two-factor authentication, suspicious-link, software-update, and backup advice; FAQs and downloadable sample templates. |
| Admin dashboard demo | Displays locally stored sample reports, summary counts, and status controls in the browser. |
| C++ reporting API | Accepts validated reports, optionally saves attachments, lists reports for authorized API requests, and updates report status. |

### Incident-response lifecycle

The framework presents the following six educational phases:

1. **Preparation:** Assign roles, prepare tools, maintain backups, and train users.
2. **Detection and Analysis:** Identify incidents, assess severity, and document evidence.
3. **Containment:** Limit the spread and preserve evidence.
4. **Eradication:** Remove threats and address their causes.
5. **Recovery:** Restore and monitor affected services.
6. **Post-Incident Review:** Document lessons learned and improve procedures.

The website describes this as a NIST SP 800-61-based framework adapted for campus use.

### Sample severity matrix

| Severity | Example | Illustrative response target |
| --- | --- | --- |
| Low | Suspicious email without interaction | Within 24 hours |
| Medium | One compromised account | Within 4 hours |
| High | Malware on a lab or office computer | Within 1 hour |
| Critical | Ransomware, major data breach, or campus-wide disruption | Within 15 minutes |

These are **sample project targets**, not guaranteed service-level agreements.

## Technology stack

- **Frontend:** HTML5, CSS3, and JavaScript.
- **Backend:** C++ with standard-library file handling and native socket APIs (Winsock on Windows; POSIX sockets on Unix-like systems).
- **Storage:** Tab-separated report records (`data/reports.tsv`) and uploaded evidence files (`uploads/`).
- **Environment:** Local development server, defaulting to `http://localhost:8080`.
- **Database:** No external database required by the provided backend.

## Project structure

The following is a **recommended layout** for the uploaded files and referenced assets. Entries marked *missing from upload* must be supplied before the full website can work as intended.

```text
Firewall101/
├── index.html
├── about.html
├── admin.html
├── contact.html
├── incidents.html
├── lifecycle.html
├── playbooks.html
├── resources.html
├── roles.html
├── severity.html
├── report.html              # Missing from upload
├── css/
│   └── style.css            # Missing from upload
├── js/
│   └── main.js              # Missing from upload
└── backend/
    ├── server.cpp           # Rename uploaded server(1).cpp
    ├── server.exe           # Optional Windows executable
    ├── data/                # Created when server starts
    │   └── reports.tsv      # Created after reports are saved
    └── uploads/             # Created when server starts
```

The backend uses relative paths, so **start it from inside the `backend` directory**. By default it serves website files from the parent directory (`..`).

## Installation and running (Windows 11)

### Prerequisites

- Windows 10/11
- A C++ compiler such as MinGW-w64 `g++`
- A modern browser (Chrome, Edge, or Firefox)
- VS Code (optional)

### 1. Arrange the project files

Create the folder structure shown above. Place the uploaded C++ source file in `backend/` and rename it to `server.cpp`. Ensure that the referenced CSS, JavaScript, and report page are available if you want to use the complete website.

### 2. Open PowerShell in the backend folder

```powershell
cd "C:\path\to\Firewall101\backend"
```

### 3. Compile the server

```powershell
g++ -std=c++11 server.cpp -o server.exe -lws2_32
```

`-lws2_32` links the Windows Winsock library used by the HTTP server.

### 4. Start the server

```powershell
.\server.exe
```

The default site directory is `..`, so this command should be run from `backend/`. To specify the site directory explicitly:

```powershell
.\server.exe ..
```

### 5. Open the website

Visit:

```text
http://localhost:8080
```

To stop the server, press `Ctrl+C` in PowerShell.

### Optional: change the admin API key

The source defaults to the demo key `admin123`. Override it before launching the server:

```powershell
$env:ADMIN_KEY = "replace-with-a-strong-unique-key"
.\server.exe
```

This is only a basic shared-key mechanism, **not** production-grade authentication. Do not expose the prototype to the public internet.

### Optional: change the port

```powershell
$env:PORT = "8081"
.\server.exe
```

Then visit `http://localhost:8081`.

## How the system works

### Educational website

Users navigate the site to learn about incident types, identify warning signs, consult response playbooks, review responsibilities, and understand escalation rules. The home page links to an incident-report form (`report.html`), but that file was not included in the uploaded set.

### Backend incident-report workflow

The provided C++ server implements this workflow:

```text
Report submitted to POST /api/reports
                  |
                  v
         Validate form fields
                  |
                  v
      Optionally save attachment
                  |
                  v
     Save record in reports.tsv
                  |
                  v
    Return incident ID and status
```

A new report receives an ID in the form `IR-YYYY-0001` and starts with the status `Submitted`.

### Report information

The backend accepts fields including:

- Reporter name, email, and department/role
- Incident type and severity
- Incident date/time
- Affected system or device
- Incident description
- Optional supporting attachment

Supported attachment extensions are PNG, JPG/JPEG, GIF, PDF, TXT, and EML. The source sets a **5 MB per-file limit** and an **8 MB maximum request body**.

### Report statuses

```text
Submitted → Under Review → Contained → Resolved → Closed
```

These are available status labels, not an enforced one-way workflow: the backend currently permits an authorized caller to set any recognized status.

## Admin dashboard and API

**Important distinction:** The uploaded `admin.html` uses JavaScript `Store` methods and explicitly identifies its data as stored in the browser only. Its report table, summary statistics, status buttons, and reset-demo-data action do **not** establish a connection to the C++ reporting API in the provided files.

The C++ server separately exposes:

| Method | Endpoint | Purpose | Admin key required? |
| --- | --- | --- | --- |
| POST | `/api/reports` | Submit a report | No |
| GET | `/api/reports` | List saved reports, newest first | Yes |
| POST | `/api/reports/{id}/status` | Update report status | Yes |

Authorized API calls use the `X-Admin-Key` HTTP header. For example, from PowerShell, after starting the server:

```powershell
Invoke-RestMethod -Uri "http://localhost:8080/api/reports" `
  -Headers @{ "X-Admin-Key" = $env:ADMIN_KEY }
```

Set `$env:ADMIN_KEY` to the actual server key in the PowerShell session making the request. The dashboard would need to be updated to use these endpoints for persistent, server-backed management.

## Data storage

The C++ server creates `backend/data/` and `backend/uploads/` relative to the working directory. Submitted reports are saved to:

```text
backend/data/reports.tsv
```

Evidence files are stored under:

```text
backend/uploads/
```

The TSV file contains report details such as ID, submission timestamp, reporter information, incident classification, description, attachment filename, and status. The server does not require MySQL, SQLite, or cloud storage.

**Privacy notice:** Reports may contain sensitive personal information. Use synthetic demonstration data, restrict local access to the data and uploads folders, and do not commit actual reports or evidence to GitHub.

Suggested `.gitignore`:

```gitignore
backend/data/
backend/uploads/
*.exe
*.o
*.obj
```

## Testing checklist

- [ ] Start the C++ server and open the homepage.
- [ ] Check navigation between the available informational pages.
- [ ] Verify the lifecycle tabs and previous/next controls.
- [ ] Check incident-type links and response playbooks.
- [ ] Review severity and escalation information.
- [ ] Test resource template viewing and downloads.
- [ ] Test report submission after restoring/implementing `report.html`.
- [ ] Confirm a valid API submission creates a record in `reports.tsv`.
- [ ] Verify that an unauthorized report-list request is rejected.
- [ ] Verify that an authorized status update persists.
- [ ] Check that uploaded files respect the size and extension limits.
- [ ] Confirm whether the browser-local admin demo or the server API is being tested; they are currently separate.

These are **suggested tests**, not claims that the tests have been executed.

## Current limitations

- `report.html`, `css/style.css`, and `js/main.js` were referenced but not included in the uploaded files.
- The uploaded admin dashboard is a browser-local demonstration and is not integrated with the C++ API.
- Admin API access uses a simple shared key, with an insecure default suitable only for local demonstration.
- The HTTP server listens on the local loopback interface; it is not configured for public hosting or HTTPS.
- Report storage uses local TSV files, not a transactional database.
- Report ID generation depends on the current report count, so IDs can potentially be reused after records are removed.
- File extension checks alone do not guarantee uploaded evidence is safe.
- The application does not provide automated threat detection, live network monitoring, or real incident-response orchestration.
- The contact page contains explicitly labeled sample contacts, not actual institutional contact information.

## Future enhancements

- Integrate the incident-report form and admin dashboard with the C++ API.
- Add authenticated accounts and role-based access control.
- Use a database with unique incident IDs and audit logging.
- Add HTTPS, stronger input/file validation, and secure evidence handling.
- Add incident assignment, search, filtering, and status history.
- Provide email notifications and escalation reminders.
- Support dashboards, reports, and trend analytics.
- Add more detailed playbooks and institutional policy customization.

## Educational use and references

Firewall101 is intended as an educational prototype for demonstrating cybersecurity incident-response concepts in a college setting. The website describes its incident-response guidance as adapted from NIST SP 800-61. For real-world use, procedures should be reviewed against current institutional policies and applicable standards.

**Reference:** [NIST Computer Security Incident Handling Guide (SP 800-61 Rev. 2)](https://csrc.nist.gov/pubs/sp/800/61/r2/final)

## License

No license was supplied with the uploaded project. Add a `LICENSE` file if you intend to distribute the source under an open-source license.

---

**Firewall101 — Identify. Report. Respond. Recover. Improve.**
