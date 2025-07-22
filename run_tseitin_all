#!/usr/bin/env bash

# Skript: run_tseitin_all.sh
# Durchläuft alle Unterordner in downward-benchmarks, findet jeweils die erste Problem-PDDL-Datei
# (nicht „domain.pddl“) und führt Fast Downward mit der Tseitin-Transformation aus.
# Die Debug-Ausgaben werden in eine zentrale Log-Datei geschrieben.

# Pfad zu deinem Fast Downward-Root-Verzeichnis (passe ggf. an):
FD_ROOT="$(pwd)"  # Annahme: Skript wird im Downward-Hauptverzeichnis ausgeführt

# Verzeichnis mit den Benchmarks (passe ggf. an):
BENCH_DIR="$FD_ROOT/downward-benchmarks"

# Zentrale Log-Datei:
LOGFILE="$FD_ROOT/tseitin_all.log"
> "$LOGFILE"  # Bestehenden Inhalt überschreiben

# Temporäre Datei für einzelne Läufe
TMPLOG="$(mktemp)"

echo "Tseitin-Durchläufe gestartet: $(date)" >> "$LOGFILE"
echo "" >> "$LOGFILE"

# Schleife über alle Unterordner in downward-benchmarks
for dir in "$BENCH_DIR"/*; do
    # Überspringe alles, was kein Verzeichnis ist
    [ -d "$dir" ] || continue

    # Prüfe, ob im Verzeichnis eine domain.pddl existiert
    DOMAIN="$dir/domain.pddl"
    if [ ! -f "$DOMAIN" ]; then
        # Manchmal heißen die Dateien anders, z.B. domain-*.pddl
        DOMAIN_CANDIDATE=$(ls "$dir"/*domain*.pddl 2>/dev/null | head -n1)
        if [ -n "$DOMAIN_CANDIDATE" ]; then
            DOMAIN="$DOMAIN_CANDIDATE"
        else
            echo "[WARNUNG] Kein domain.pddl in '$dir' gefunden, überspringe" >> "$LOGFILE"
            echo "" >> "$LOGFILE"
            continue
        fi
    fi

    # Finde die erste PDDL-Datei, die nicht die Domain ist
    PROBLEM=$(ls "$dir"/*.pddl 2>/dev/null | grep -v "$(basename "$DOMAIN")" | head -n1)
    if [ -z "$PROBLEM" ]; then
        echo "[WARNUNG] Kein Problem-PDDL in '$dir' (außer Domain) gefunden, überspringe" >> "$LOGFILE"
        echo "" >> "$LOGFILE"
        continue
    fi

    # Log-Kopf für dieses Verzeichnis
    echo "=== Benchmark: $(basename "$dir") ===" >> "$LOGFILE"
    echo "Domain:  $DOMAIN" >> "$LOGFILE"
    echo "Problem: $PROBLEM" >> "$LOGFILE"
    echo "" >> "$LOGFILE"

    # Fast Downward-Aufruf mit Tseitin-Transformation
    # Ausgabe von stderr (Debug) in TMPLOG, stdout verwerfen
    "$FD_ROOT/fast-downward.py" "$DOMAIN" "$PROBLEM" \
        --search "astar(lmcut(),transform=tseitin())" \
        > /dev/null 2> "$TMPLOG"
    EXITCODE=$?

    # Kopiere TMPLOG-Inhalt in zentrale Log-Datei
    echo "--- Debug-Ausgabe (stderr) ---" >> "$LOGFILE"
    if [ -s "$TMPLOG" ]; then
        cat "$TMPLOG" >> "$LOGFILE"
    else
        echo "(keine Debug-Ausgabe)" >> "$LOGFILE"
    fi
    echo "--- Ende Debug-Ausgabe ---" >> "$LOGFILE"
    echo "" >> "$LOGFILE"

    # Schreib Exit-Code
    echo "Exit-Code: $EXITCODE" >> "$LOGFILE"
    echo "" >> "$LOGFILE"
done

# Aufräumen
rm "$TMPLOG"

echo "Tseitin-Durchläufe beendet: $(date)" >> "$LOGFILE"
