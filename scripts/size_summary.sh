#!/usr/bin/env bash

FILE="$1"
SIZE_FILE="${FILE}.size"
SIZE_PREV="${FILE}.size.prev"

# GOWIN M1 memory layout from linker script
# ITCM (rx)  : ORIGIN = 0x00000000 (labeled FLASH in linker)
# DTCM (rwx) : ORIGIN = 0x20000000 (labeled RAM in linker)
TOTAL_ITCM_BYTES=64000
TOTAL_DTCM_BYTES=64000

arm-none-eabi-size "$FILE" > "$SIZE_FILE"

echo ""
echo "=== size summary for $FILE ==="

if [[ -f "$SIZE_PREV" ]]; then
    prev_line=$(tail -n 1 "$SIZE_PREV")
    curr_line=$(tail -n 1 "$SIZE_FILE")

    # arm-none-eabi-size outputs: text data bss dec hex filename
    set -- $prev_line; p_text=$1; p_data=$2; p_bss=$3;
    set -- $curr_line; c_text=$1; c_data=$2; c_bss=$3;

    echo ""
    echo "=== size diff ==="
    printf "  text: %+d bytes\n" $((c_text - p_text))
    printf "  data: %+d bytes\n" $((c_data - p_data))
    printf "   bss: %+d bytes\n" $((c_bss - p_bss))
    printf " total: %+d bytes\n" $(( (c_text+c_data+c_bss) - (p_text+p_data+p_bss) ))
fi

# Show current memory usage with percentage
curr_line=$(tail -n 1 "$SIZE_FILE")
set -- $curr_line; c_text=$1; c_data=$2; c_bss=$3;

USED_ITCM=$((c_text + c_data))
USED_DTCM=$((c_data + c_bss))

ITCM_PERC=$(echo "scale=2; $USED_ITCM*100/$TOTAL_ITCM_BYTES" | bc -l 2>/dev/null || echo "N/A")
DTCM_PERC=$(echo "scale=2; $USED_DTCM*100/$TOTAL_DTCM_BYTES" | bc -l 2>/dev/null || echo "N/A")

echo ""
echo "=== current memory usage ==="
printf "  ITCM: %d / %d bytes (%.2f%%)\n" $USED_ITCM $TOTAL_ITCM_BYTES $ITCM_PERC
printf "  DTCM: %d / %d bytes (%.2f%%)\n" $USED_DTCM $TOTAL_DTCM_BYTES $DTCM_PERC

cp "$SIZE_FILE" "$SIZE_PREV"
