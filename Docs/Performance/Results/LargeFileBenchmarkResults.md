# Large-File-Benchmark – Ergebnisse

---

## Messlauf 2026-11-09T14:59:41.394Z

- Git-Commit: `c9005590b57d`
- Datensatzgröße: 1074000000 Bytes (1024.25 Mebibyte)
- Ergebnis-CSV: `LargeFileBenchmarkResults.csv`

| Messphase | Bedeutung | Dauer | Verarbeitete Logeinträge | Verarbeitete Daten pro Sekunde | Arbeitsspeicher am Phasenende |
|---|---|---:|---:|---:|---:|
| Erstimport | Datei vollständig einlesen, analysieren und speichern | 53.753 s | 4000000 | 19.05 Mebibyte pro Sekunde | 21.88 Mebibyte |
| Cache-Treffer | Bereits aufgebauten Cache öffnen und erste Seite laden | 2.101 s | 4000000 | 487.58 Mebibyte pro Sekunde | 21.83 Mebibyte |
| Mittlere Seite öffnen | Von der ersten auf die mittlere Ergebnisseite wechseln | 8.831 s | 4000000 | – | 22.28 Mebibyte |
| Letzte Seite öffnen | Von der mittleren auf die letzte Ergebnisseite wechseln | 13.883 s | 4000000 | – | 23.09 Mebibyte |
| Anwendungsfilter setzen | Nur Logeinträge der Anwendung App01 laden | 0.679 s | 1000000 | – | 23.32 Mebibyte |
| Anwendungsfilter entfernen | Wieder Logeinträge aller Anwendungen laden | 2.186 s | 4000000 | – | 23.34 Mebibyte |
| Log-Level-Filter setzen | Nur Logeinträge mit Level ERROR laden | 0.680 s | 1000000 | – | 23.36 Mebibyte |
| Log-Level-Filter entfernen | Wieder Logeinträge aller Level laden | 2.238 s | 4000000 | – | 23.36 Mebibyte |
| Textsuche ausführen | Nach dem Text scenario=large-file in Meldungen suchen | 2.501 s | 4000000 | – | 23.38 Mebibyte |
| Textsuche entfernen | Textsuche leeren und ungefilterte erste Seite laden | 2.201 s | 4000000 | – | 23.28 Mebibyte |
| Regex-Suche ausführen | Regulären Ausdruck über alle vier Millionen Meldungen prüfen | 116.190 s | 4000000 | – | 50.56 Mebibyte |
| Regex-Suche entfernen | Regulären Ausdruck leeren und erste Seite neu laden | 2.176 s | 4000000 | – | 49.83 Mebibyte |

## Bedeutung der Einheiten

- **Mebibyte pro Sekunde:** Wie viele Daten pro Sekunde verarbeitet wurden. Ein Mebibyte entspricht 1.048.576 Bytes. Ein höherer Wert ist besser.
- **Arbeitsspeicher am Phasenende:** Physischer Speicher, den der Benchmark-Prozess beim Abschluss der jeweiligen Phase belegt hat. Ein niedrigerer Wert ist besser.
- **Erstimport:** Misst den vollständigen Weg ohne vorhandenen Cache.
- **Cache-Treffer:** Misst das erneute Öffnen derselben Datei über den bereits erzeugten persistenten Cache.
- **Leeres Feld bei Daten pro Sekunde:** Für Filter, Suche und Seitenwechsel ist eine Dateidatenrate fachlich nicht sinnvoll; dort zählt ausschließlich die gemessene Dauer.

---

## Messlauf 2026-11-09T15:43:08.008Z

- Git-Commit: `02a8cef947f3`
- Datensatzgröße: 1074000000 Bytes (1024.25 Mebibyte)
- Ergebnis-CSV: `LargeFileBenchmarkResults.csv`

| Messphase | Bedeutung | Dauer | Verarbeitete Logeinträge | Verarbeitete Daten pro Sekunde | Arbeitsspeicher am Phasenende |
|---|---|---:|---:|---:|---:|
| Erstimport | Datei vollständig einlesen, analysieren und speichern | 54.319 s | 4000000 | 18.86 Mebibyte pro Sekunde | 21.83 Mebibyte |
| Cache-Treffer | Bereits aufgebauten Cache öffnen und erste Seite laden | 2.108 s | 4000000 | 485.79 Mebibyte pro Sekunde | 21.43 Mebibyte |
| Mittlere Seite öffnen | Von der ersten auf die mittlere Ergebnisseite wechseln | 3.745 s | 4000000 | – | 22.32 Mebibyte |
| Letzte Seite öffnen | Von der mittleren auf die letzte Ergebnisseite wechseln | 3.797 s | 4000000 | – | 22.27 Mebibyte |
| Anwendungsfilter setzen | Nur Logeinträge der Anwendung App01 laden | 0.653 s | 1000000 | – | 22.08 Mebibyte |
| Anwendungsfilter entfernen | Wieder Logeinträge aller Anwendungen laden | 2.091 s | 4000000 | – | 22.09 Mebibyte |
| Log-Level-Filter setzen | Nur Logeinträge mit Level ERROR laden | 0.656 s | 1000000 | – | 21.71 Mebibyte |
| Log-Level-Filter entfernen | Wieder Logeinträge aller Level laden | 2.089 s | 4000000 | – | 21.85 Mebibyte |
| Textsuche ausführen | Nach dem Text scenario=large-file in Meldungen suchen | 2.367 s | 4000000 | – | 21.92 Mebibyte |
| Textsuche entfernen | Textsuche leeren und ungefilterte erste Seite laden | 2.090 s | 4000000 | – | 21.92 Mebibyte |
| Regex-Suche ausführen | Regulären Ausdruck über alle vier Millionen Meldungen prüfen | 121.368 s | 4000000 | – | 50.46 Mebibyte |
| Regex-Suche entfernen | Regulären Ausdruck leeren und erste Seite neu laden | 2.470 s | 4000000 | – | 49.78 Mebibyte |

## Bedeutung der Einheiten

- **Mebibyte pro Sekunde:** Wie viele Daten pro Sekunde verarbeitet wurden. Ein Mebibyte entspricht 1.048.576 Bytes. Ein höherer Wert ist besser.
- **Arbeitsspeicher am Phasenende:** Physischer Speicher, den der Benchmark-Prozess beim Abschluss der jeweiligen Phase belegt hat. Ein niedrigerer Wert ist besser.
- **Erstimport:** Misst den vollständigen Weg ohne vorhandenen Cache.
- **Cache-Treffer:** Misst das erneute Öffnen derselben Datei über den bereits erzeugten persistenten Cache.
- **Leeres Feld bei Daten pro Sekunde:** Für Filter, Suche und Seitenwechsel ist eine Dateidatenrate fachlich nicht sinnvoll; dort zählt ausschließlich die gemessene Dauer.
