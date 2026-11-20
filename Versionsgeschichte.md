# Versionsgeschichte

## Version 2.3 (November 2026)
- Reiter Lift für den Liftdecoder: StartHoming (CV33), IR_Detect (CV34), LCD_Display (CV35) und Serial_Line (CV36).
- Drei Reiter für die Servo-2-, Servo-3- und Servo-6-Decoder:
  - Servo: SkipUnEven (CV21), SwitchCount (CV35), SwitchType (CV36: Normal, DKW oder Hosentrager) und die Anzahl der Servos auf der Platine (CV64, nur lesen).
  - Servo positions: Live-Einstellung von Min, Max und Speed eines Servos, wobei der Servo jede Änderung sofort zeigt (ab Decodersoftware Version 24). Während einer Live-Einstellung sind CANCEL und DONE rot, und es können keine anderen Reiter, Decoder oder Servos gewählt werden.
  - Servo CVs: Kurven, InvertServoDir, InvertRelais (nur Servo-2), ServoType und die Puls-/Versorgungs-CVs eines Servos.
- Nach SELECT werden die decodertypspezifischen CVs (33 und höher) nur für den gefundenen Decodertyp gelesen.
- Die decoderspezifischen Reiter (Switch, Lift und die Servo-Reiter) werden über ihren Identifier gefunden, sodass mehr als ein Reiter ausgeblendet werden kann.
- Repository: Die macOS-Version steht im Verzeichnis `macOS`, sodass später eine Windows- und eine Linux-Version hinzukommen können. Die Screenshots stehen in `Documentation/Screenshots`. Die Dokumentation gibt es auf Englisch, Deutsch und Niederländisch.

## Version 2.2 (Oktober 2026)
- RS-Bus-Time-out auf 1 Sekunde geändert.

## Version 2.1 (Oktober 2026)
- Reiter RS-Bus erweitert: RSParity (CV22) und RSPulsCount (CV24) legen fest, wie der Decoder auf RS-Bus-Paritäts- und Pulszählfehler reagiert. Die Anzahl der Fehler seit dem letzten Neustart des Decoders wird ebenfalls angezeigt: ParityErrors (CV31) und PulseErrors (CV32), beide nur lesen. Decoder mit der älteren OpenDecoder22-Software unterstützen diese CVs nicht; bei diesen Decodern wird 0 angezeigt.
- Auf dem Reiter RS-Bus heißt RSFEC jetzt RSRetry (CV20), wie auf dem Reiter Control; beide Reiter zeigen denselben Wert. Die Tasten GET, DEFAULTS und SET des Reiters RS-Bus wirken jetzt auf die RS-Bus-CVs.
- Der Servo-6-Decoder wird erkannt. Die Liste Decoder Type auf dem Reiter Control enthält jetzt Servo-2, Servo-3 und Servo-6; die Auswahl von Lift oder Function fällt nicht mehr auf Switch zurück.

## Version 2.0 (Oktober 2026)
- Auf Xcode 26 aktualisiert; Universal Binary (Apple Silicon und Intel); benötigt macOS 12 oder neuer.
- TCP-Verbindungen über das Network-Framework von Apple, statt über die NSStream-Funktionen, die Apple abkündigen wird. Das Programm hängt nicht mehr, wenn ein Lenz-Interface nicht antwortet.
- Die Verbindung wird erneut aufgebaut, wenn eine Decoderadresse eingegeben oder SELECT gedrückt wird.
- XpressNet-Keep-alive für die Lenz-Interfaces alle 10 Sekunden.
- Benutzeroberfläche an aktuelle macOS-Versionen angepasst.
- Mehrere kleine Fehlerkorrekturen in der Kommunikation mit den Lenz-Interfaces.

Frühere Versionen: siehe die Commit-Historie.
