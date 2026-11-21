# Source code of the Windows and Linux versions (EN)
The C++ / Qt 6 sources of the Windows and Linux versions. The compiled programs and the installation instructions are in [Compiled-Applications/Windows](../../Compiled-Applications/Windows/README.md) and [Compiled-Applications/Linux](../../Compiled-Applications/Linux/README.md). The functionality is the same as that of the macOS version ([src/Xcode](../Xcode/README.md)).

| Directory | Contents |
|---|---|
| `src` | Main window (`MainWindow`), PoM messages and RS-Bus feedback (`PomConnection`), CV store (`DccDecoder`), CV names (`CvNames.h`), preferences, help and the centered combo box |
| `src/tabs` | One class per tab; the three servo tabs are in `ServoTabs` |
| `ui` | The screens (Qt Designer files), originally converted from `MainMenu.xib` of the macOS version |
| `resources` | Icon of the program |
| `XpressNetQt` | Copy of the XpressNetQt library of [XpressNet](https://github.com/aikopras/XpressNet) |

The help pages are not in this directory: the program uses the help pages of the macOS version (`../Xcode/Helpfiles/Decoder-POMHelp`). The complete `src` directory is therefore needed to build the program.

## Building on Windows ##
Needed: Visual Studio 2019 and Qt 6.7 for MSVC 2019. Qt 6.8 and later only have packages for MSVC 2022; with Visual Studio 2022 Qt 6.8 or later can be used in the same way (with `msvc2022_64` instead of `msvc2019_64`). Visual Studio 2019 also runs (under emulation) on Windows 11 on ARM; the result is the same x64 program.

### 1. Visual Studio 2019 ###
In the Visual Studio Installer, choose Modify for Visual Studio 2019 and check:
- Workload "Desktop development with C++".
- Component "C++ CMake tools for Windows" (tab Individual components, or in the details of the workload).

### 2. Qt ###
1. Download the Qt Online Installer for Windows x64 from [qt.io/download-qt-installer-oss](https://www.qt.io/download-qt-installer-oss). The installer asks for a (free) Qt account.
2. Choose "Custom installation". Select "Archive" (menu Show, right of the search field) to see the older Qt versions; loading may take several minutes. Click "None" to deselect everything.
3. Under Qt for Development → Qt → Qt 6.7.3 check only:
   - MSVC 2019 64-bit
   - Additional Libraries → Qt Serial Port
4. Install to the default folder `C:\Qt` (about 2 GB). The commands below assume `C:\Qt\6.7.3\msvc2019_64`; adjust them if you choose another folder or version.

### 3. Build ###
Open "x64 Native Tools Command Prompt for VS 2019" (Start menu → Visual Studio 2019) and go to the directory `src\Qt` of this repository:

    cmake -S . -B build -G "Visual Studio 16 2019" -A x64 -DCMAKE_PREFIX_PATH=C:\Qt\6.7.3\msvc2019_64
    cmake --build build --config Release

The program is `build\Release\Programmer Decoder-POM.exe`. After a change of a source or .ui file, only the second command is needed.

### 4. Deploy ###
The program needs the Qt DLLs. `windeployqt` copies these next to the program, in the folder `deploy\Programmer Decoder-POM`. Then delete the DirectX shader compilers that windeployqt may add; the program does not use them:

    mkdir "deploy\Programmer Decoder-POM"
    copy /y "build\Release\Programmer Decoder-POM.exe" "deploy\Programmer Decoder-POM"
    C:\Qt\6.7.3\msvc2019_64\bin\windeployqt.exe --release --no-translations --no-opengl-sw --no-system-d3d-compiler --no-compiler-runtime "deploy\Programmer Decoder-POM\Programmer Decoder-POM.exe"
    del "deploy\Programmer Decoder-POM\dxcompiler.dll" "deploy\Programmer Decoder-POM\dxil.dll"

Test the program in `deploy\Programmer Decoder-POM`. Create the ZIP file for [Compiled-Applications/Windows](../../Compiled-Applications/Windows) with the `tar` program of Windows:

    tar -a -c -f "..\..\Compiled-Applications\Windows\Programmer Decoder-POM.zip" -C deploy "Programmer Decoder-POM"

### Editing and debugging in Visual Studio ###
After step 3, open `build\ProgrammerDecoderPoM.sln` in Visual Studio 2019. The program is the startup project, and while debugging the Qt DLLs are found automatically. After adding or removing a source file, add it to `CMakeLists.txt`; after adding a help page, run the first cmake command of step 3 again. The screens (`ui/*.ui`) can be edited with Qt Designer (`C:\Qt\6.7.3\msvc2019_64\bin\designer.exe`); keep the names (objectName) of the widgets, since the code uses them.

## Building on Linux ##
Needed: CMake (3.16 or later), a C++17 compiler and Qt 6 (Widgets, Network and SerialPort). On Ubuntu / Debian:

    sudo apt install cmake g++ qt6-base-dev libqt6serialport6-dev

From the directory `src/Qt` of this repository:

    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build

The program is `build/Programmer Decoder-POM`.

---

# Quellcode der Windows- und Linux-Versionen (DE)
Die C++-/Qt-6-Quellen der Windows- und der Linux-Version. Die übersetzten Programme und die Installationsanleitungen stehen in [Compiled-Applications/Windows](../../Compiled-Applications/Windows/README.md) und [Compiled-Applications/Linux](../../Compiled-Applications/Linux/README.md). Die Funktionen sind dieselben wie die der macOS-Version ([src/Xcode](../Xcode/README.md)). Die Tabelle oben (EN) beschreibt die Verzeichnisse.

Die Hilfeseiten stehen nicht in diesem Verzeichnis: Das Programm verwendet die Hilfeseiten der macOS-Version (`../Xcode/Helpfiles/Decoder-POMHelp`). Zum Übersetzen wird deshalb das ganze Verzeichnis `src` benötigt.

## Übersetzen unter Windows ##
Benötigt: Visual Studio 2019 und Qt 6.7 für MSVC 2019. Ab Qt 6.8 gibt es nur Pakete für MSVC 2022; mit Visual Studio 2022 kann Qt 6.8 oder neuer auf dieselbe Weise verwendet werden (mit `msvc2022_64` statt `msvc2019_64`). Visual Studio 2019 läuft (mit Emulation) auch unter Windows 11 auf ARM; das Ergebnis ist dasselbe x64-Programm.

### 1. Visual Studio 2019 ###
Im Visual Studio Installer bei Visual Studio 2019 „Ändern“ wählen und ankreuzen (englische Bezeichnungen; in einer deutschen Installation entsprechend übersetzt):
- Workload „Desktop development with C++“.
- Komponente „C++ CMake tools for Windows“ (Reiter Individual components, oder in den Details des Workloads).

### 2. Qt ###
1. Den Qt Online Installer für Windows x64 von [qt.io/download-qt-installer-oss](https://www.qt.io/download-qt-installer-oss) herunterladen. Der Installer fragt nach einem (kostenlosen) Qt-Konto.
2. „Custom installation“ wählen. „Archive“ wählen (Menü Show, rechts neben dem Suchfeld), um die älteren Qt-Versionen zu sehen; das Laden kann einige Minuten dauern. Auf „None“ klicken, um alles abzuwählen.
3. Unter Qt for Development → Qt → Qt 6.7.3 nur ankreuzen:
   - MSVC 2019 64-bit
   - Additional Libraries → Qt Serial Port
4. In den Standardordner `C:\Qt` installieren (etwa 2 GB). Die Befehle unten gehen von `C:\Qt\6.7.3\msvc2019_64` aus; bei einem anderen Ordner oder einer anderen Version anpassen.

### 3. Übersetzen ###
„x64 Native Tools Command Prompt for VS 2019“ öffnen (Startmenü → Visual Studio 2019) und in das Verzeichnis `src\Qt` dieses Repositorys wechseln:

    cmake -S . -B build -G "Visual Studio 16 2019" -A x64 -DCMAKE_PREFIX_PATH=C:\Qt\6.7.3\msvc2019_64
    cmake --build build --config Release

Das Programm ist `build\Release\Programmer Decoder-POM.exe`. Nach einer Änderung einer Quell- oder .ui-Datei genügt der zweite Befehl.

### 4. Weitergeben ###
Das Programm braucht die Qt-DLLs. `windeployqt` kopiert diese neben das Programm, in den Ordner `deploy\Programmer Decoder-POM`. Danach die DirectX-Shader-Compiler löschen, die windeployqt eventuell hinzufügt; das Programm verwendet sie nicht:

    mkdir "deploy\Programmer Decoder-POM"
    copy /y "build\Release\Programmer Decoder-POM.exe" "deploy\Programmer Decoder-POM"
    C:\Qt\6.7.3\msvc2019_64\bin\windeployqt.exe --release --no-translations --no-opengl-sw --no-system-d3d-compiler --no-compiler-runtime "deploy\Programmer Decoder-POM\Programmer Decoder-POM.exe"
    del "deploy\Programmer Decoder-POM\dxcompiler.dll" "deploy\Programmer Decoder-POM\dxil.dll"

Das Programm in `deploy\Programmer Decoder-POM` testen. Die ZIP-Datei für [Compiled-Applications/Windows](../../Compiled-Applications/Windows) mit dem Programm `tar` von Windows erstellen:

    tar -a -c -f "..\..\Compiled-Applications\Windows\Programmer Decoder-POM.zip" -C deploy "Programmer Decoder-POM"

### Bearbeiten und Debuggen in Visual Studio ###
Nach Schritt 3 `build\ProgrammerDecoderPoM.sln` in Visual Studio 2019 öffnen. Das Programm ist das Startprojekt, und beim Debuggen werden die Qt-DLLs automatisch gefunden. Nach dem Hinzufügen oder Entfernen einer Quelldatei diese in `CMakeLists.txt` eintragen; nach dem Hinzufügen einer Hilfeseite den ersten cmake-Befehl aus Schritt 3 erneut ausführen. Die Fenster (`ui/*.ui`) können mit Qt Designer bearbeitet werden (`C:\Qt\6.7.3\msvc2019_64\bin\designer.exe`); die Namen (objectName) der Widgets beibehalten, da der Code sie verwendet.

## Übersetzen unter Linux ##
Benötigt: CMake (3.16 oder neuer), ein C++17-Compiler und Qt 6 (Widgets, Network und SerialPort). Unter Ubuntu / Debian:

    sudo apt install cmake g++ qt6-base-dev libqt6serialport6-dev

Im Verzeichnis `src/Qt` dieses Repositorys:

    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build

Das Programm ist `build/Programmer Decoder-POM`.

---

# Broncode van de Windows- en Linux-versies (NL)
De C++-/Qt-6-broncode van de Windows- en de Linux-versie. De gecompileerde programma's en de installatie-instructies staan in [Compiled-Applications/Windows](../../Compiled-Applications/Windows/README.md) en [Compiled-Applications/Linux](../../Compiled-Applications/Linux/README.md). De functionaliteit is dezelfde als die van de macOS-versie ([src/Xcode](../Xcode/README.md)). De tabel hierboven (EN) beschrijft de directories.

De helppagina's staan niet in deze directory: het programma gebruikt de helppagina's van de macOS-versie (`../Xcode/Helpfiles/Decoder-POMHelp`). Om te bouwen is daarom de hele directory `src` nodig.

## Bouwen op Windows ##
Nodig: Visual Studio 2019 en Qt 6.7 voor MSVC 2019. Vanaf Qt 6.8 zijn er alleen pakketten voor MSVC 2022; met Visual Studio 2022 kan Qt 6.8 of nieuwer op dezelfde manier gebruikt worden (met `msvc2022_64` in plaats van `msvc2019_64`). Visual Studio 2019 draait (onder emulatie) ook op Windows 11 op ARM; het resultaat is hetzelfde x64-programma.

### 1. Visual Studio 2019 ###
Kies in de Visual Studio Installer bij Visual Studio 2019 "Wijzigen" en vink aan (Engelse namen; in een Nederlandse installatie vertaald):
- Workload "Desktop development with C++".
- Onderdeel "C++ CMake tools for Windows" (tab Individual components, of in de details van de workload).

### 2. Qt ###
1. Download de Qt Online Installer voor Windows x64 van [qt.io/download-qt-installer-oss](https://www.qt.io/download-qt-installer-oss). De installer vraagt om een (gratis) Qt-account.
2. Kies "Custom installation". Kies "Archive" (menu Show, rechts naast het zoekveld) om de oudere Qt-versies te zien; het laden kan enkele minuten duren. Klik op "None" om alles uit te vinken.
3. Vink onder Qt for Development → Qt → Qt 6.7.3 alleen aan:
   - MSVC 2019 64-bit
   - Additional Libraries → Qt Serial Port
4. Installeer in de standaardmap `C:\Qt` (ongeveer 2 GB). De commando's hieronder gaan uit van `C:\Qt\6.7.3\msvc2019_64`; pas ze aan bij een andere map of versie.

### 3. Bouwen ###
Open "x64 Native Tools Command Prompt for VS 2019" (Startmenu → Visual Studio 2019) en ga naar de directory `src\Qt` van deze repository:

    cmake -S . -B build -G "Visual Studio 16 2019" -A x64 -DCMAKE_PREFIX_PATH=C:\Qt\6.7.3\msvc2019_64
    cmake --build build --config Release

Het programma is `build\Release\Programmer Decoder-POM.exe`. Na een wijziging van een bron- of .ui-bestand is alleen het tweede commando nodig.

### 4. Verspreiden ###
Het programma heeft de Qt-DLL's nodig. `windeployqt` kopieert die naast het programma, in de map `deploy\Programmer Decoder-POM`. Verwijder daarna de DirectX-shadercompilers die windeployqt mogelijk toevoegt; het programma gebruikt ze niet:

    mkdir "deploy\Programmer Decoder-POM"
    copy /y "build\Release\Programmer Decoder-POM.exe" "deploy\Programmer Decoder-POM"
    C:\Qt\6.7.3\msvc2019_64\bin\windeployqt.exe --release --no-translations --no-opengl-sw --no-system-d3d-compiler --no-compiler-runtime "deploy\Programmer Decoder-POM\Programmer Decoder-POM.exe"
    del "deploy\Programmer Decoder-POM\dxcompiler.dll" "deploy\Programmer Decoder-POM\dxil.dll"

Test het programma in `deploy\Programmer Decoder-POM`. Maak het ZIP-bestand voor [Compiled-Applications/Windows](../../Compiled-Applications/Windows) met het programma `tar` van Windows:

    tar -a -c -f "..\..\Compiled-Applications\Windows\Programmer Decoder-POM.zip" -C deploy "Programmer Decoder-POM"

### Bewerken en debuggen in Visual Studio ###
Open na stap 3 `build\ProgrammerDecoderPoM.sln` in Visual Studio 2019. Het programma is het opstartproject, en tijdens het debuggen worden de Qt-DLL's automatisch gevonden. Voeg na het toevoegen of verwijderen van een bronbestand dat toe in `CMakeLists.txt`; voer na het toevoegen van een helppagina het eerste cmake-commando van stap 3 opnieuw uit. De schermen (`ui/*.ui`) kunnen bewerkt worden met Qt Designer (`C:\Qt\6.7.3\msvc2019_64\bin\designer.exe`); houd de namen (objectName) van de widgets gelijk, want de code gebruikt die.

## Bouwen op Linux ##
Nodig: CMake (3.16 of nieuwer), een C++17-compiler en Qt 6 (Widgets, Network en SerialPort). Op Ubuntu / Debian:

    sudo apt install cmake g++ qt6-base-dev libqt6serialport6-dev

Vanuit de directory `src/Qt` van deze repository:

    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build

Het programma is `build/Programmer Decoder-POM`.
