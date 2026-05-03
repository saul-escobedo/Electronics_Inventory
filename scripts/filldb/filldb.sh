#!/usr/bin/bash

if [ ! -n "$1" ]; then
    echo "Usage: fill_db.sh <output_sqlite_file>"
    exit 1
fi

if [ "${1:0:1}" = "/" ] || [ [ "${1:0:1}" = "~" ] ];  then
    output_sqlite_file="$1"
else
    output_sqlite_file="$(pwd)/$1"
fi

python_exec="python3"

if [ -n "$(which python)" ]; then
    python_exec="python"
fi

cd "$(dirname "$0")"

$python_exec csv_import.py resistor Resistors.csv "$output_sqlite_file"
$python_exec csv_import.py capacitor Capacitors.csv "$output_sqlite_file"
$python_exec csv_import.py inductor Inductors.csv "$output_sqlite_file"
$python_exec csv_import.py diode Diodes.csv "$output_sqlite_file"
$python_exec csv_import.py bjt BJTransistors.csv "$output_sqlite_file"
$python_exec csv_import.py fet FETransistors.csv "$output_sqlite_file"
$python_exec csv_import.py ic IntegratedCircuits.csv "$output_sqlite_file"
