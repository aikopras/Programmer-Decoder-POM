# Versiegeschiedenis

## Versie 2.3 (november 2026)
- Tab Lift voor de liftdecoder: StartHoming (CV33), IR_Detect (CV34), LCD_Display (CV35) en Serial_Line (CV36).
- Drie tabs voor de servo-2-, servo-3- en servo-6-decoders:
  - Servo: SkipUnEven (CV21), SwitchCount (CV35), SwitchType (CV36: Normal, DKW of Hosentrager) en het aantal servo's op het board (CV64, alleen lezen).
  - Servo positions: live instellen van Min, Max en Speed van één servo, waarbij de servo elke wijziging direct laat zien (vanaf decodersoftware versie 24). Tijdens het live instellen zijn CANCEL en DONE rood, en kunnen geen andere tabs, decoders of servo's gekozen worden.
  - Servo CVs: curves, InvertServoDir, InvertRelais (alleen servo-2), ServoType en de puls-/voedings-CV's van één servo.
- Na SELECT worden de decodertype-specifieke CV's (33 en hoger) alleen gelezen voor het gevonden decodertype.
- De decoder-specifieke tabs (Switch, Lift en de Servo-tabs) worden gevonden via hun identifier, zodat meer dan één tab verborgen kan worden.
- Repository: de macOS-versie staat in de directory `macOS`, zodat er later een Windows- en een Linux-versie bij kunnen komen. De screenshots staan in `Documentation/Screenshots`. De documentatie is er in het Engels, Duits en Nederlands.

## Versie 2.2 (oktober 2026)
- RS-Bus-time-out gewijzigd naar 1 seconde.

## Versie 2.1 (oktober 2026)
- Tab RS-Bus uitgebreid: RSParity (CV22) en RSPulsCount (CV24) bepalen hoe de decoder reageert op RS-Bus-pariteits- en pulstelfouten. Het aantal fouten sinds de laatste herstart van de decoder wordt ook getoond: ParityErrors (CV31) en PulseErrors (CV32), beide alleen lezen. Decoders met de oudere OpenDecoder22-software ondersteunen deze CV's niet; bij deze decoders wordt 0 getoond.
- Op de tab RS-Bus heet RSFEC nu RSRetry (CV20), net als op de tab Control; beide tabs tonen dezelfde waarde. De knoppen GET, DEFAULTS en SET van de tab RS-Bus werken nu op de RS-Bus-CV's.
- De servo-6-decoder wordt herkend. De lijst Decoder Type op de tab Control bevat nu servo-2, servo-3 en servo-6; het kiezen van lift of function valt niet meer terug op switch.

## Versie 2.0 (oktober 2026)
- Bijgewerkt naar Xcode 26; universal binary (Apple silicon en Intel); vereist macOS 12 of nieuwer.
- TCP-verbindingen via het Network framework van Apple, in plaats van de NSStream-functies die Apple gaat uitfaseren. Het programma blijft niet meer hangen als een Lenz-interface niet antwoordt.
- De verbinding wordt opnieuw gemaakt als een decoderadres wordt ingevoerd of SELECT wordt gedrukt.
- XpressNet keep-alive voor de Lenz-interface(s) elke 10 seconden.
- Gebruikersinterface aangepast aan de huidige macOS-versies.
- Een aantal kleine foutcorrecties in de communicatie met de Lenz-interface(s).

Eerdere versies: zie de commitgeschiedenis.
