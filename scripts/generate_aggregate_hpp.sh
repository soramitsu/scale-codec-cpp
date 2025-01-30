#!/bin/sh

# This script modifies a specified C++ source file by replacing a generated section.
# It reads the file, finds the markers '-BEGIN-GENERATED-SECTION-' and '-END-GENERATED-SECTION-',
# generates C++ code based on a given integer N, and writes the modified content back to the file.
# If the markers are not found, the script exits with an error without modifying the file.
# If the file is modified, the specified stamp file is updated (if provided).
# Usage: ./generate_aggregate_hpp.sh <filename> <N> [stampfile]
# Where <filename> is the path to the C++ source file, <N> is an integer between 0 and 1000,
# and [stampfile] is an optional path to the stamp file to be updated.

# Validate input
if [ "$#" -lt 2 ] || [ "$#" -gt 3 ]; then
    echo "Usage: $0 <filename> <N> [stampfile]"
    exit 1
fi

FILENAME="$1"
N="$2"
STAMPFILE="$3"

if [ ! -f "$FILENAME" ]; then
    echo "Error: File '$FILENAME' not found."
    exit 1
fi

if echo "$N" | grep -q "[^0-9]" || [ "$N" -lt 0 ] || [ "$N" -gt 1000 ]; then
    echo "N must be an integer between 0 and 1000"
    exit 1
fi

# Find the section markers
BEGIN_LINE=$(awk '/-BEGIN-GENERATED-SECTION-/ {print NR; exit}' "$FILENAME")
END_LINE=$(awk '/-END-GENERATED-SECTION-/ {print NR; exit}' "$FILENAME")

if [ -z "$BEGIN_LINE" ] || [ -z "$END_LINE" ]; then
    echo "Error: Required markers not found in the file."
    exit 1
fi

# Create temporary file
tempfile=$(mktemp) || { echo "Failed to create temporary file"; exit 1; }

# Trap to ensure the temporary file is deleted in case of errors
trap 'rm -f "$tempfile"' EXIT

# Write the modified content to the temporary file
awk -v begin="$BEGIN_LINE" -v end="$END_LINE" -v n="$N" '
BEGIN { in_generated_section = 0; }
{
    if (NR == begin) {
        print; # Print the BEGIN marker
        in_generated_section = 1;
        for (i = 1; i <= n; i++) {
            printf "    } else if constexpr (N == %d) {\n", i;
            printf "      auto &[";
            for (j = 1; j <= i; j++) {
                printf "v%d%s", j, (j < i ? ", " : "");
            }
            printf "] = v;\n";
            printf "      return f(";
            for (j = 1; j <= i; j++) {
                printf "v%d%s", j, (j < i ? ", " : "");
            }
            printf ");\n";
        }
        next;
    }
    if (NR == end) {
        print; # Print the END marker
        in_generated_section = 0;
        next;
    }
    if (!in_generated_section) {
        print;
    }
}' "$FILENAME" > "$tempfile" || { echo "Error processing file"; exit 1; }

# Check if the file has changed before overwriting
if ! cmp -s "$tempfile" "$FILENAME"; then
    mv "$tempfile" "$FILENAME"
    if [ -n "$STAMPFILE" ]; then
        touch "$STAMPFILE"
        echo "File '$FILENAME' successfully updated. Stamp file '$STAMPFILE' modified."
    else
        echo "File '$FILENAME' successfully updated."
    fi
else
    rm -f "$tempfile"
    echo "No changes detected."
fi

# Remove trap on success
trap - EXIT

exit 0
