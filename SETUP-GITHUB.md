# Setup: GitHub + Visual Studio 2022 (Windows)

## 1) Git & GitHub anmelden
- Installiere **Git for Windows** (falls noch nicht vorhanden).
- Öffne **Visual Studio 2022** → Menü **Git** → **Git-Einstellungen** → bei **Anbieteranmeldungen** mit deinem **GitHub**-Account anmelden.

## 2) Neues Repo erstellen (Variante A: direkt in VS)
- **Git** → **Repository erstellen...**
- Tab **GitHub** wählen, Name z. B. `ternary-computer`.
- **Public** oder **Private** wählen → **Erstellen und pushen**.
- Danach im Explorer das lokale Repo öffnen.

## 3) Bestehendes Repo verbinden (Variante B: GitHub zuerst)
- Auf github.com ein leeres Repository anlegen (ohne README/License).
- Lokal einen Ordner erstellen, den Inhalt des **T27_Starter_OpenPack** hinein kopieren.
- Rechtsklick → **Git Bash Here** oder Terminal:
  ```bash
  git init -b main
  git add .
  git commit -m "chore: bootstrap T27 starter open pack"
  git remote add origin https://github.com/<user-or-org>/<repo>.git
  git push -u origin main
  ```

## 4) In bestehende VS-Lösung integrieren
- Öffne deine `.sln` in VS2022.
- Statusleiste → **Zu Quellcodeverwaltung hinzufügen** → **Git**.
- **Git Changes**-Fenster: Dateien prüfen, Commit-Message eingeben, **Commit & Push**.
- Falls das Remote noch fehlt: **Git** → **Einstellungen** → **Repository-Einstellungen** → **Remotes** → `origin` hinzufügen.

## 5) CI aktivieren
- Auf GitHub → **Actions**-Tab → den vorgeschlagenen Workflow **Basic hygiene** aktivieren (sofern nötig).
- Der Workflow zeigt informativ Dateien ohne SPDX-Header an (Build schlägt **nicht** fehl).

## 6) Branch-Schutz (optional, empfohlen)
- GitHub → **Settings** → **Branches** → **Branch protection rule** für `main`:
  - Require pull request before merging
  - Require status checks to pass (später aktivieren)
  - Require DCO (per Probot DCO App) – optional

## 7) Nächste Schritte
- README anpassen (Repo-URL, Badge, Roadmap).
- `SECURITY.md` Kontaktadresse ersetzen.
- Erste Beispiele/HDL/Hardware-Strukturen hinzufügen – mit **SPDX**-Headern.
