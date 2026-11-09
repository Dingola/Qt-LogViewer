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

---

## Messlauf 2026-11-09T16:14:07.736Z

- Git-Commit: `0f78da4dffee`
- Datensatzgröße: 1074000000 Bytes (1024.25 Mebibyte)
- Ergebnis-CSV: `LargeFileBenchmarkResults.csv`

| Messphase | Bedeutung | Dauer | Verarbeitete Logeinträge | Verarbeitete Daten pro Sekunde | Arbeitsspeicher am Phasenende |
|---|---|---:|---:|---:|---:|
| Erstimport | Datei vollständig einlesen, analysieren und speichern | 57.214 s | 4000000 | 17.90 Mebibyte pro Sekunde | 21.94 Mebibyte |
| Cache-Treffer | Bereits aufgebauten Cache öffnen und erste Seite laden | 1.954 s | 4000000 | 524.10 Mebibyte pro Sekunde | 22.16 Mebibyte |
| Mittlere Seite öffnen | Von der ersten auf die mittlere Ergebnisseite wechseln | 1.054 s | 4000000 | – | 22.09 Mebibyte |
| Letzte Seite öffnen | Von der mittleren auf die letzte Ergebnisseite wechseln | 1.032 s | 4000000 | – | 21.96 Mebibyte |
| Anwendungsfilter setzen | Nur Logeinträge der Anwendung App01 laden | 0.654 s | 1000000 | – | 21.91 Mebibyte |
| Anwendungsfilter entfernen | Wieder Logeinträge aller Anwendungen laden | 1.874 s | 4000000 | – | 21.91 Mebibyte |
| Log-Level-Filter setzen | Nur Logeinträge mit Level ERROR laden | 0.655 s | 1000000 | – | 22.07 Mebibyte |
| Log-Level-Filter entfernen | Wieder Logeinträge aller Level laden | 1.878 s | 4000000 | – | 21.95 Mebibyte |
| Textsuche ausführen | Nach dem Text scenario=large-file in Meldungen suchen | 2.356 s | 4000000 | – | 22.11 Mebibyte |
| Textsuche entfernen | Textsuche leeren und ungefilterte erste Seite laden | 1.877 s | 4000000 | – | 22.12 Mebibyte |
| Regex-Suche ausführen | Regulären Ausdruck über alle vier Millionen Meldungen prüfen | 113.810 s | 4000000 | – | 51.20 Mebibyte |
| Regex-Suche entfernen | Regulären Ausdruck leeren und erste Seite neu laden | 1.941 s | 4000000 | – | 51.87 Mebibyte |

## Bedeutung der Einheiten

- **Mebibyte pro Sekunde:** Wie viele Daten pro Sekunde verarbeitet wurden. Ein Mebibyte entspricht 1.048.576 Bytes. Ein höherer Wert ist besser.
- **Arbeitsspeicher am Phasenende:** Physischer Speicher, den der Benchmark-Prozess beim Abschluss der jeweiligen Phase belegt hat. Ein niedrigerer Wert ist besser.
- **Erstimport:** Misst den vollständigen Weg ohne vorhandenen Cache.
- **Cache-Treffer:** Misst das erneute Öffnen derselben Datei über den bereits erzeugten persistenten Cache.
- **Leeres Feld bei Daten pro Sekunde:** Für Filter, Suche und Seitenwechsel ist eine Dateidatenrate fachlich nicht sinnvoll; dort zählt ausschließlich die gemessene Dauer.

---

## Messlauf 2026-11-09T19:07:39.805Z

- Git-Commit: `7f88c2fc8d93`
- Datensatzgröße: 1074000000 Bytes (1024.25 Mebibyte)
- Ergebnis-CSV: `LargeFileBenchmarkResults.csv`

| Messphase | Bedeutung | Dauer | Verarbeitete Logeinträge | Verarbeitete Daten pro Sekunde | Arbeitsspeicher am Phasenende |
|---|---|---:|---:|---:|---:|
| Erstimport | Datei vollständig einlesen, analysieren und speichern | 73.253 s | 4000000 | 13.98 Mebibyte pro Sekunde | 21.85 Mebibyte |
| Cache-Treffer | Bereits aufgebauten Cache öffnen und erste Seite laden | 1.910 s | 4000000 | 536.13 Mebibyte pro Sekunde | 22.04 Mebibyte |
| Mittlere Seite öffnen | Von der ersten auf die mittlere Ergebnisseite wechseln | 1.006 s | 4000000 | – | 21.83 Mebibyte |
| Letzte Seite öffnen | Von der mittleren auf die letzte Ergebnisseite wechseln | 1.040 s | 4000000 | – | 21.59 Mebibyte |
| Anwendungsfilter setzen | Nur Logeinträge der Anwendung App01 laden | 0.657 s | 1000000 | – | 21.61 Mebibyte |
| Anwendungsfilter entfernen | Wieder Logeinträge aller Anwendungen laden | 1.897 s | 4000000 | – | 21.64 Mebibyte |
| Log-Level-Filter setzen | Nur Logeinträge mit Level ERROR laden | 0.659 s | 1000000 | – | 21.80 Mebibyte |
| Log-Level-Filter entfernen | Wieder Logeinträge aller Level laden | 1.887 s | 4000000 | – | 21.63 Mebibyte |
| Textsuche ausführen | Nach dem Text scenario=large-file in Meldungen suchen | 2.426 s | 4000000 | – | 21.79 Mebibyte |
| Textsuche entfernen | Textsuche leeren und ungefilterte erste Seite laden | 1.896 s | 4000000 | – | 21.91 Mebibyte |
| Regex-Suche ausführen | Regulären Ausdruck über alle vier Millionen Meldungen prüfen | 117.926 s | 4000000 | – | 51.70 Mebibyte |
| Regex-Suche entfernen | Regulären Ausdruck leeren und erste Seite neu laden | 1.887 s | 4000000 | – | 52.38 Mebibyte |

## Bedeutung der Einheiten

- **Mebibyte pro Sekunde:** Wie viele Daten pro Sekunde verarbeitet wurden. Ein Mebibyte entspricht 1.048.576 Bytes. Ein höherer Wert ist besser.
- **Arbeitsspeicher am Phasenende:** Physischer Speicher, den der Benchmark-Prozess beim Abschluss der jeweiligen Phase belegt hat. Ein niedrigerer Wert ist besser.
- **Erstimport:** Misst den vollständigen Weg ohne vorhandenen Cache.
- **Cache-Treffer:** Misst das erneute Öffnen derselben Datei über den bereits erzeugten persistenten Cache.
- **Leeres Feld bei Daten pro Sekunde:** Für Filter, Suche und Seitenwechsel ist eine Dateidatenrate fachlich nicht sinnvoll; dort zählt ausschließlich die gemessene Dauer.
