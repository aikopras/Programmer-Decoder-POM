# Version history

## Version 2.3 (November 2026)
- Lift tab for the lift decoder: StartHoming (CV33), IR_Detect (CV34), LCD_Display (CV35) and Serial_Line (CV36).
- Three tabs for the servo-2, servo-3 and servo-6 decoders:
  - Servo: SkipUnEven (CV21), SwitchCount (CV35), SwitchType (CV36: Normal, DKW or Hosentrager) and the number of servos on the board (CV64, read-only).
  - Servo positions: live setting of Min, Max and Speed of one servo, while the servo shows every change directly (requires decoder software version 24 or higher). During a live setting CANCEL and DONE are shown in red, and other tabs, decoders and servos can not be selected.
  - Servo CVs: curves, InvertServoDir, InvertRelais (servo-2 only), ServoType and the pulse / power CVs of one servo.
- After SELECT, the decoder type specific CVs (33 and higher) are only read for the decoder type that is found.
- The decoder specific tabs (Switch, Lift and the Servo tabs) are found via their identifier, so more than one tab can be hidden.
- New: Windows and Linux version, written in C++ with Qt 6 (directory `src/Qt`), with the same functionality as the macOS version. The communication with the Lenz interface(s) uses the XpressNetQt library; a single TCP connection is used if the same interface is used for sending and for feedback. The IP addresses are stored in the settings of the program. The Windows version is an x64 program, that also runs on Windows 11 on ARM.
- A non-initialised decoder (PoM address 6999) is reported in the status line, and the Initialise tab is shown. The address field is empty at startup; SELECT with an empty address field searches again for a non-initialised decoder.
- The range of switch addresses depends on the command station: 1..2048 for command station software 3.8 or higher, otherwise 1..1024.
- Switch tab: the Yes / No choices are centered, the sliders are placed more to the right, and the explanation mentions RocRail instead of Railware.
- CV21 is called SkipUnEven on the Switch tab as well (before: SkipEven), as in the AP_DCC_Decoder_Core library.
- Decoder addresses up to 511 are accepted (switch addresses up to 2048); before, the limit was 255.
- Repository: new structure, with the source code in `src` (`src/Xcode` for macOS, `src/Qt` for Windows and Linux), the compiled programs in `Compiled-Applications` and the manual in `Manual`. The README and the manual are in English, German and Dutch; `Manual/details/FeedbackProblems.md` (English) explains why SkipUnEven may be needed.

## Version 2.2 (October 2026)
- RS-Bus time-out changed to 1 second.

## Version 2.1 (October 2026)
- RS-Bus tab extended: RSParity (CV22) and RSPulsCount (CV24) define how the decoder reacts to RS-Bus parity and pulse count errors. The number of errors since the last decoder restart is shown as well: ParityErrors (CV31) and PulseErrors (CV32), both read-only. Decoders based on the older OpenDecoder22 software do not support these CVs; for these decoders the values are shown as 0.
- On the RS-Bus tab RSFEC is now called RSRetry (CV20), like on the Control tab; both tabs show the same value. The GET, DEFAULTS and SET buttons of the RS-Bus tab now act on the RS-Bus CVs.
- The Servo-6 decoder is recognised. The Decoder Type list on the Control tab now contains servo-2, servo-3 and servo-6; selecting lift or function no longer falls back to switch.

## Version 2.0 (October 2026)
- Updated to Xcode 26; universal binary (Apple silicon and Intel); requires macOS 12 or later.
- TCP connections via Apple's Network framework, instead of the NSStream functions that Apple will deprecate. The program no longer hangs if a Lenz interface does not respond.
- The connection is made again when a decoder address is entered or SELECT is pressed.
- XpressNet keep-alive for the Lenz interface(s) every 10 seconds.
- User interface adapted to current macOS versions.
- Several small bug fixes in the communication with the Lenz interface(s).

Earlier versions: see the commit history.
