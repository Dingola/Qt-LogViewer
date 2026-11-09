# Automatischer Large-File-Benchmark

Der Benchmark ist kein Bestandteil der Anwendung und wird weder beim normalen Release-Build noch
beim normalen Test-Build konfiguriert, gebaut oder gestartet. Das folgende Kommando aktiviert das
separate Performance-Target ausdrücklich, baut es als Release und führt die Messung aus:

```powershell
& ".\Scripts\Win\Performance\Invoke-LargeFileBenchmark.ps1" -DatasetPath "$env:USERPROFILE\Desktop\TestLogs\qt-logviewer-perf-4m-run1.log"
```

Das Skript erstellt eine eindeutige temporäre Kopie des Datensatzes. Damit misst die erste Phase
garantiert einen Fresh-Import; unmittelbar danach wird dieselbe Kopie als Cache-Hit geöffnet. Die
Die Ergebnisse landen standardmäßig gesammelt unter `Docs/Performance/Results`. Die CSV
`LargeFileBenchmarkResults.csv` enthält verständlich ausgeschriebene deutsche Spaltennamen und
Werte. Daneben wird `LargeFileBenchmarkResults.md` mit einer direkt lesbaren Ergebnistabelle,
Szenarioerklärungen und Erläuterungen aller Einheiten erzeugt. Beide Dateien entstehen vollständig
automatisch. Weitere Läufe werden als neue Zeilen beziehungsweise als eigener Messlauf angehängt;
vorhandene Ergebnisse werden nicht überschrieben.

Jeder Lauf misst Erstimport, Cache-Treffer, Wechsel auf mittlere und letzte Seite,
Anwendungsfilter, Log-Level-Filter, normale Textsuche und Regex-Suche. Für Filter und Suchen werden
sowohl das Setzen als auch das anschließende Entfernen gemessen.

Der Runner misst weiterhin keine tatsächliche Fenster-Zeichenlatenz und keinen Wechsel zwischen
zwei sichtbaren Tabs. Diese UI-spezifischen Werte gelten daher noch nicht als erledigt.
