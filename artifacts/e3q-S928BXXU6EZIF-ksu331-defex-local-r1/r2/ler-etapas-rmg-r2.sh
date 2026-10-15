#!/system/bin/sh
# Reads the r2 phase logs as shell; never starts root, the exploit, or KernelSU.
umask 077
report="/sdcard/Download/RMG-etapas-r2-$(/system/bin/date +%Y%m%d-%H%M%S)-$$.txt"
if ! (set -C; : > "$report") 2>/dev/null; then
    printf 'Nao foi possivel criar o relatorio em Download.\n' >&2
    exit 1
fi
found=0
for log in /data/local/tmp/rmg332-diag-*/phase.log; do
    [ -f "$log" ] || continue
    found=$((found + 1))
    printf '\n===== Tentativa r2 #%s =====\n' "$found" >> "$report"
    /system/bin/cat "$log" >> "$report" 2>&1
done
if [ "$found" = 0 ]; then
    printf 'Nenhum arquivo de etapas r2 foi encontrado.\n' >> "$report"
fi
printf 'Relatorio salvo em %s\n' "$report"
