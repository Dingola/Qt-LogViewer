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

---

## Messlauf 2026-11-09T19:43:16.895Z

- Quellstand: `9bc1e3d40d49+Arbeitskopie`
- Datensatzgröße: 1074000000 Bytes (1024.25 Mebibyte)
- Ergebnis-CSV: `LargeFileBenchmarkResults.csv`

| Messphase | Bedeutung | Dauer | Verarbeitete Logeinträge | Verarbeitete Daten pro Sekunde | Arbeitsspeicher am Phasenende |
|---|---|---:|---:|---:|---:|
| Erstimport | Datei vollständig einlesen, analysieren und speichern | 60.415 s | 4000000 | 16.95 Mebibyte pro Sekunde | 22.01 Mebibyte |
| Cache-Treffer | Bereits aufgebauten Cache öffnen und erste Seite laden | 1.903 s | 4000000 | 538.13 Mebibyte pro Sekunde | 22.03 Mebibyte |
| Zweite Seite öffnen | Von Seite 1 auf die direkt benachbarte Seite 2 wechseln | 0.968 s | 4000000 | – | 22.04 Mebibyte |
| Erste Seite wieder öffnen | Von Seite 2 zur ersten Ergebnisseite zurückkehren | 0.967 s | 4000000 | – | 22.05 Mebibyte |
| Mittlere Seite öffnen | Von der ersten auf die mittlere Ergebnisseite wechseln | 0.999 s | 4000000 | – | 22.21 Mebibyte |
| Letzte Seite öffnen | Von der mittleren auf die letzte Ergebnisseite wechseln | 1.034 s | 4000000 | – | 22.05 Mebibyte |
| Zeitstempel aufsteigend sortieren | Alle Einträge nach dem ältesten Zeitstempel zuerst sortieren | 1.884 s | 4000000 | – | 22.05 Mebibyte |
| Anwendung sortieren | Alle Einträge alphabetisch nach Anwendung sortieren | 1.997 s | 4000000 | – | 22.05 Mebibyte |
| Log-Level sortieren | Alle Einträge alphabetisch nach Log-Level sortieren | 1.990 s | 4000000 | – | 22.05 Mebibyte |
| Nachricht sortieren | Alle Einträge alphabetisch nach Nachricht sortieren | 1236.915 s | 4000000 | – | 48.31 Mebibyte |
| Standardsortierung wiederherstellen | Neueste Zeitstempel wieder zuerst anzeigen | 2.021 s | 4000000 | – | 49.18 Mebibyte |
| Anwendungsfilter setzen | Nur Logeinträge der Anwendung App01 laden | 0.664 s | 1000000 | – | 47.96 Mebibyte |
| Anwendungsfilter entfernen | Wieder Logeinträge aller Anwendungen laden | 1.905 s | 4000000 | – | 48.44 Mebibyte |
| Log-Level-Filter setzen | Nur Logeinträge mit Level ERROR laden | 0.657 s | 1000000 | – | 48.15 Mebibyte |
| Log-Level-Filter entfernen | Wieder Logeinträge aller Level laden | 1.880 s | 4000000 | – | 47.13 Mebibyte |
| Textsuche ausführen | Nach dem Text scenario=large-file in Meldungen suchen | 2.361 s | 4000000 | – | 48.67 Mebibyte |
| Textsuche entfernen | Textsuche leeren und ungefilterte erste Seite laden | 1.897 s | 4000000 | – | 48.51 Mebibyte |
| Regex-Suche ausführen | Regulären Ausdruck über alle vier Millionen Meldungen prüfen | 53.390 s | 4000000 | – | 52.37 Mebibyte |
| Regex-Suche entfernen | Regulären Ausdruck leeren und erste Seite neu laden | 0.000 s | 0 | – | 52.43 Mebibyte |

## Bedeutung der Einheiten

- **Mebibyte pro Sekunde:** Wie viele Daten pro Sekunde verarbeitet wurden. Ein Mebibyte entspricht 1.048.576 Bytes. Ein höherer Wert ist besser.
- **Arbeitsspeicher am Phasenende:** Physischer Speicher, den der Benchmark-Prozess beim Abschluss der jeweiligen Phase belegt hat. Ein niedrigerer Wert ist besser.
- **Erstimport:** Misst den vollständigen Weg ohne vorhandenen Cache.
- **Cache-Treffer:** Misst das erneute Öffnen derselben Datei über den bereits erzeugten persistenten Cache.
- **Leeres Feld bei Daten pro Sekunde:** Für Filter, Suche und Seitenwechsel ist eine Dateidatenrate fachlich nicht sinnvoll; dort zählt ausschließlich die gemessene Dauer.

---

## Messlauf 2026-11-09T20:31:14.042Z

- Quellstand: `9bc1e3d40d49+Arbeitskopie`
- Nachrichtensortierung: ausgelassen (opt-in)
- Datensatzgröße: 1074000000 Bytes (1024.25 Mebibyte)
- Ergebnis-CSV: `LargeFileBenchmarkResults.csv`

| Messphase | Bedeutung | Dauer | Verarbeitete Logeinträge | Verarbeitete Daten pro Sekunde | Arbeitsspeicher am Phasenende |
|---|---|---:|---:|---:|---:|
| Erstimport | Datei vollständig einlesen, analysieren und speichern | 55.192 s | 4000000 | 18.56 Mebibyte pro Sekunde | 21.84 Mebibyte |
| Cache-Treffer | Bereits aufgebauten Cache öffnen und erste Seite laden | 1.893 s | 4000000 | 541.15 Mebibyte pro Sekunde | 21.48 Mebibyte |
| Zweite Seite öffnen | Von Seite 1 auf die direkt benachbarte Seite 2 wechseln | 0.960 s | 4000000 | – | 22.00 Mebibyte |
| Erste Seite wieder öffnen | Von Seite 2 zur ersten Ergebnisseite zurückkehren | 0.953 s | 4000000 | – | 21.94 Mebibyte |
| Mittlere Seite öffnen | Von der ersten auf die mittlere Ergebnisseite wechseln | 0.994 s | 4000000 | – | 21.98 Mebibyte |
| Letzte Seite öffnen | Von der mittleren auf die letzte Ergebnisseite wechseln | 1.035 s | 4000000 | – | 21.88 Mebibyte |
| Zeitstempel aufsteigend sortieren | Alle Einträge nach dem ältesten Zeitstempel zuerst sortieren | 1.896 s | 4000000 | – | 22.00 Mebibyte |
| Anwendung sortieren | Alle Einträge alphabetisch nach Anwendung sortieren | 2.020 s | 4000000 | – | 21.88 Mebibyte |
| Log-Level sortieren | Alle Einträge alphabetisch nach Log-Level sortieren | 2.016 s | 4000000 | – | 21.95 Mebibyte |
| Standardsortierung wiederherstellen | Neueste Zeitstempel wieder zuerst anzeigen | 1.895 s | 4000000 | – | 21.96 Mebibyte |
| Anwendungsfilter setzen | Nur Logeinträge der Anwendung App01 laden | 0.663 s | 1000000 | – | 21.87 Mebibyte |
| Anwendungsfilter entfernen | Wieder Logeinträge aller Anwendungen laden | 1.887 s | 4000000 | – | 21.94 Mebibyte |
| Log-Level-Filter setzen | Nur Logeinträge mit Level ERROR laden | 0.652 s | 1000000 | – | 22.02 Mebibyte |
| Log-Level-Filter entfernen | Wieder Logeinträge aller Level laden | 1.900 s | 4000000 | – | 21.96 Mebibyte |
| Textsuche ausführen | Nach dem Text scenario=large-file in Meldungen suchen | 2.369 s | 4000000 | – | 22.05 Mebibyte |
| Textsuche entfernen | Textsuche leeren und ungefilterte erste Seite laden | 1.962 s | 4000000 | – | 21.99 Mebibyte |
| Regex-Suche ausführen | Regulären Ausdruck über alle vier Millionen Meldungen prüfen | 113.768 s | 4000000 | – | 52.11 Mebibyte |
| Regex-Suche entfernen | Regulären Ausdruck leeren und erste Seite neu laden | 1.869 s | 4000000 | – | 52.47 Mebibyte |

## Bedeutung der Einheiten

- **Mebibyte pro Sekunde:** Wie viele Daten pro Sekunde verarbeitet wurden. Ein Mebibyte entspricht 1.048.576 Bytes. Ein höherer Wert ist besser.
- **Arbeitsspeicher am Phasenende:** Physischer Speicher, den der Benchmark-Prozess beim Abschluss der jeweiligen Phase belegt hat. Ein niedrigerer Wert ist besser.
- **Erstimport:** Misst den vollständigen Weg ohne vorhandenen Cache.
- **Cache-Treffer:** Misst das erneute Öffnen derselben Datei über den bereits erzeugten persistenten Cache.
- **Leeres Feld bei Daten pro Sekunde:** Für Filter, Suche und Seitenwechsel ist eine Dateidatenrate fachlich nicht sinnvoll; dort zählt ausschließlich die gemessene Dauer.


---

## Messlauf 2026-11-09T21:06:59.073Z

- Quellstand: `515530238538+Arbeitskopie`
- Nachrichtensortierung: ausgelassen (opt-in)
- Datensatzgröße: 1074000000 Bytes (1024.25 Mebibyte)
- Ergebnis-CSV: `LargeFileBenchmarkResults.csv`

| Messphase | Bedeutung | Dauer | Verarbeitete Logeinträge | Verarbeitete Daten pro Sekunde | Arbeitsspeicher am Phasenende |
|---|---|---:|---:|---:|---:|
| Erstimport | Datei vollständig einlesen, analysieren und speichern | 54.845 s | 4000000 | 18.68 Mebibyte pro Sekunde | 22.13 Mebibyte |
| Cache-Treffer | Bereits aufgebauten Cache öffnen und erste Seite laden | 1.886 s | 4000000 | 543.18 Mebibyte pro Sekunde | 22.25 Mebibyte |
| Zweite Seite öffnen | Von Seite 1 auf die direkt benachbarte Seite 2 wechseln | 0.958 s | 4000000 | – | 22.10 Mebibyte |
| Erste Seite wieder öffnen | Von Seite 2 zur ersten Ergebnisseite zurückkehren | 0.965 s | 4000000 | – | 22.05 Mebibyte |
| Mittlere Seite öffnen | Von der ersten auf die mittlere Ergebnisseite wechseln | 0.997 s | 4000000 | – | 22.09 Mebibyte |
| Letzte Seite öffnen | Von der mittleren auf die letzte Ergebnisseite wechseln | 1.036 s | 4000000 | – | 22.17 Mebibyte |
| Zeitstempel aufsteigend sortieren | Alle Einträge nach dem ältesten Zeitstempel zuerst sortieren | 0.964 s | 4000000 | – | 21.99 Mebibyte |
| Anwendung sortieren | Alle Einträge alphabetisch nach Anwendung sortieren | 1.063 s | 4000000 | – | 22.03 Mebibyte |
| Log-Level sortieren | Alle Einträge alphabetisch nach Log-Level sortieren | 1.048 s | 4000000 | – | 22.00 Mebibyte |
| Standardsortierung wiederherstellen | Neueste Zeitstempel wieder zuerst anzeigen | 0.973 s | 4000000 | – | 22.28 Mebibyte |
| Anwendungsfilter setzen | Nur Logeinträge der Anwendung App01 laden | 0.642 s | 1000000 | – | 21.98 Mebibyte |
| Anwendungsfilter entfernen | Wieder Logeinträge aller Anwendungen laden | 1.881 s | 4000000 | – | 22.32 Mebibyte |
| Log-Level-Filter setzen | Nur Logeinträge mit Level ERROR laden | 0.653 s | 1000000 | – | 22.32 Mebibyte |
| Log-Level-Filter entfernen | Wieder Logeinträge aller Level laden | 1.885 s | 4000000 | – | 22.30 Mebibyte |
| Textsuche ausführen | Nach dem Text scenario=large-file in Meldungen suchen | 2.367 s | 4000000 | – | 22.13 Mebibyte |
| Textsuche entfernen | Textsuche leeren und ungefilterte erste Seite laden | 1.877 s | 4000000 | – | 22.36 Mebibyte |
| Regex-Suche ausführen | Regulären Ausdruck über alle vier Millionen Meldungen prüfen | 112.072 s | 4000000 | – | 51.20 Mebibyte |
| Regex-Suche entfernen | Regulären Ausdruck leeren und erste Seite neu laden | 1.888 s | 4000000 | – | 52.19 Mebibyte |

## Bedeutung der Einheiten

- **Mebibyte pro Sekunde:** Wie viele Daten pro Sekunde verarbeitet wurden. Ein Mebibyte entspricht 1.048.576 Bytes. Ein höherer Wert ist besser.
- **Arbeitsspeicher am Phasenende:** Physischer Speicher, den der Benchmark-Prozess beim Abschluss der jeweiligen Phase belegt hat. Ein niedrigerer Wert ist besser.
- **Erstimport:** Misst den vollständigen Weg ohne vorhandenen Cache.
- **Cache-Treffer:** Misst das erneute Öffnen derselben Datei über den bereits erzeugten persistenten Cache.
- **Leeres Feld bei Daten pro Sekunde:** Für Filter, Suche und Seitenwechsel ist eine Dateidatenrate fachlich nicht sinnvoll; dort zählt ausschließlich die gemessene Dauer.

---

## Messlauf 2026-11-09T21:39:53.318Z

- Quellstand: `f47791f53eca+Arbeitskopie`
- Nachrichtensortierung: enthalten
- Datensatzgröße: 1074000000 Bytes (1024.25 Mebibyte)
- Ergebnis-CSV: `LargeFileBenchmarkResults.csv`

| Messphase | Bedeutung | Dauer | Verarbeitete Logeinträge | Verarbeitete Daten pro Sekunde | Arbeitsspeicher am Phasenende |
|---|---|---:|---:|---:|---:|
| Erstimport | Datei vollständig einlesen, analysieren und speichern | 55.308 s | 4000000 | 18.52 Mebibyte pro Sekunde | 21.90 Mebibyte |
| Cache-Treffer | Bereits aufgebauten Cache öffnen und erste Seite laden | 1.947 s | 4000000 | 526.00 Mebibyte pro Sekunde | 22.14 Mebibyte |
| Zweite Seite öffnen | Von Seite 1 auf die direkt benachbarte Seite 2 wechseln | 0.984 s | 4000000 | – | 21.92 Mebibyte |
| Erste Seite wieder öffnen | Von Seite 2 zur ersten Ergebnisseite zurückkehren | 0.988 s | 4000000 | – | 21.88 Mebibyte |
| Mittlere Seite öffnen | Von der ersten auf die mittlere Ergebnisseite wechseln | 1.035 s | 4000000 | – | 21.91 Mebibyte |
| Letzte Seite öffnen | Von der mittleren auf die letzte Ergebnisseite wechseln | 1.058 s | 4000000 | – | 21.81 Mebibyte |
| Zeitstempel aufsteigend sortieren | Alle Einträge nach dem ältesten Zeitstempel zuerst sortieren | 0.984 s | 4000000 | – | 21.82 Mebibyte |
| Anwendung sortieren | Alle Einträge alphabetisch nach Anwendung sortieren | 1.090 s | 4000000 | – | 21.88 Mebibyte |
| Log-Level sortieren | Alle Einträge alphabetisch nach Log-Level sortieren | 1.078 s | 4000000 | – | 21.90 Mebibyte |
| Nachricht sortieren | Alle Einträge alphabetisch nach Nachricht sortieren | 32.469 s | 4000000 | – | 22.89 Mebibyte |
| Standardsortierung wiederherstellen | Neueste Zeitstempel wieder zuerst anzeigen | 0.999 s | 4000000 | – | 22.73 Mebibyte |
| Anwendungsfilter setzen | Nur Logeinträge der Anwendung App01 laden | 0.663 s | 1000000 | – | 22.75 Mebibyte |
| Anwendungsfilter entfernen | Wieder Logeinträge aller Anwendungen laden | 1.921 s | 4000000 | – | 22.45 Mebibyte |
| Log-Level-Filter setzen | Nur Logeinträge mit Level ERROR laden | 0.664 s | 1000000 | – | 22.86 Mebibyte |
| Log-Level-Filter entfernen | Wieder Logeinträge aller Level laden | 1.924 s | 4000000 | – | 22.71 Mebibyte |
| Textsuche ausführen | Nach dem Text scenario=large-file in Meldungen suchen | 2.389 s | 4000000 | – | 22.99 Mebibyte |
| Textsuche entfernen | Textsuche leeren und ungefilterte erste Seite laden | 1.927 s | 4000000 | – | 22.73 Mebibyte |
| Regex-Suche ausführen | Regulären Ausdruck über alle vier Millionen Meldungen prüfen | 115.014 s | 4000000 | – | 48.59 Mebibyte |
| Regex-Suche entfernen | Regulären Ausdruck leeren und erste Seite neu laden | 1.936 s | 4000000 | – | 49.84 Mebibyte |

## Bedeutung der Einheiten

- **Mebibyte pro Sekunde:** Wie viele Daten pro Sekunde verarbeitet wurden. Ein Mebibyte entspricht 1.048.576 Bytes. Ein höherer Wert ist besser.
- **Arbeitsspeicher am Phasenende:** Physischer Speicher, den der Benchmark-Prozess beim Abschluss der jeweiligen Phase belegt hat. Ein niedrigerer Wert ist besser.
- **Erstimport:** Misst den vollständigen Weg ohne vorhandenen Cache.
- **Cache-Treffer:** Misst das erneute Öffnen derselben Datei über den bereits erzeugten persistenten Cache.
- **Leeres Feld bei Daten pro Sekunde:** Für Filter, Suche und Seitenwechsel ist eine Dateidatenrate fachlich nicht sinnvoll; dort zählt ausschließlich die gemessene Dauer.

---

## Messlauf 2026-11-10T02:19:11.733Z

- Quellstand: `da09fc08af0f+Arbeitskopie`
- Nachrichtensortierung: enthalten
- Datensatzgröße: 1074000000 Bytes (1024.25 Mebibyte)
- Ergebnis-CSV: `LargeFileBenchmarkResults.csv`

| Messphase | Bedeutung | Dauer | Verarbeitete Logeinträge | Verarbeitete Daten pro Sekunde | Arbeitsspeicher am Phasenende |
|---|---|---:|---:|---:|---:|
| Erstimport | Datei vollständig einlesen, analysieren und speichern | 54.968 s | 4000000 | 18.63 Mebibyte pro Sekunde | 22.39 Mebibyte |
| Cache-Treffer | Bereits aufgebauten Cache öffnen und erste Seite laden | 1.931 s | 4000000 | 530.39 Mebibyte pro Sekunde | 22.41 Mebibyte |
| Zweite Seite öffnen | Von Seite 1 auf die direkt benachbarte Seite 2 wechseln | 0.971 s | 4000000 | – | 22.41 Mebibyte |
| Erste Seite wieder öffnen | Von Seite 2 zur ersten Ergebnisseite zurückkehren | 0.974 s | 4000000 | – | 22.42 Mebibyte |
| Mittlere Seite öffnen | Von der ersten auf die mittlere Ergebnisseite wechseln | 1.005 s | 4000000 | – | 22.48 Mebibyte |
| Letzte Seite öffnen | Von der mittleren auf die letzte Ergebnisseite wechseln | 1.036 s | 4000000 | – | 22.36 Mebibyte |
| Zeitstempel aufsteigend sortieren | Alle Einträge nach dem ältesten Zeitstempel zuerst sortieren | 0.966 s | 4000000 | – | 22.36 Mebibyte |
| Anwendung sortieren | Alle Einträge alphabetisch nach Anwendung sortieren | 1.079 s | 4000000 | – | 22.36 Mebibyte |
| Log-Level sortieren | Alle Einträge alphabetisch nach Log-Level sortieren | 1.067 s | 4000000 | – | 22.36 Mebibyte |
| Nachricht sortieren | Alle Einträge alphabetisch nach Nachricht sortieren | 31.667 s | 4000000 | – | 23.54 Mebibyte |
| Standardsortierung wiederherstellen | Neueste Zeitstempel wieder zuerst anzeigen | 0.979 s | 4000000 | – | 23.21 Mebibyte |
| Anwendungsfilter setzen | Nur Logeinträge der Anwendung App01 laden | 0.652 s | 1000000 | – | 23.05 Mebibyte |
| Anwendungsfilter entfernen | Wieder Logeinträge aller Anwendungen laden | 1.886 s | 4000000 | – | 23.04 Mebibyte |
| Log-Level-Filter setzen | Nur Logeinträge mit Level ERROR laden | 0.657 s | 1000000 | – | 23.17 Mebibyte |
| Log-Level-Filter entfernen | Wieder Logeinträge aller Level laden | 1.886 s | 4000000 | – | 23.04 Mebibyte |
| Textsuche ausführen | Nach dem Text scenario=large-file in Meldungen suchen | 2.399 s | 4000000 | – | 23.35 Mebibyte |
| Textsuche entfernen | Textsuche leeren und ungefilterte erste Seite laden | 1.901 s | 4000000 | – | 23.00 Mebibyte |
| Regex-Suche ausführen | Regulären Ausdruck über alle vier Millionen Meldungen prüfen | 72.352 s | 4000000 | – | 27.21 Mebibyte |
| Regex-Suche entfernen | Regulären Ausdruck leeren und erste Seite neu laden | 8.510 s | 4000000 | – | 30.11 Mebibyte |

## Bedeutung der Einheiten

- **Mebibyte pro Sekunde:** Wie viele Daten pro Sekunde verarbeitet wurden. Ein Mebibyte entspricht 1.048.576 Bytes. Ein höherer Wert ist besser.
- **Arbeitsspeicher am Phasenende:** Physischer Speicher, den der Benchmark-Prozess beim Abschluss der jeweiligen Phase belegt hat. Ein niedrigerer Wert ist besser.
- **Erstimport:** Misst den vollständigen Weg ohne vorhandenen Cache.
- **Cache-Treffer:** Misst das erneute Öffnen derselben Datei über den bereits erzeugten persistenten Cache.
- **Leeres Feld bei Daten pro Sekunde:** Für Filter, Suche und Seitenwechsel ist eine Dateidatenrate fachlich nicht sinnvoll; dort zählt ausschließlich die gemessene Dauer.
