#!/system/bin/sh
# Read-only collection. Does not acquire root, run the payload, change module
# state, stop services, mount anything, clear logs, or restart the device.
umask 077
export PATH=/system/bin:/system/xbin

base=/sdcard/Download
[ -d "$base" ] || base=/data/local/tmp
stamp=$(date +%Y%m%d-%H%M%S) || exit 1
output=$(mktemp -d "$base/RMG-S24-diagnostics-$stamp-XXXXXX") || exit 1
report=$output/report.txt

section() { printf '\n===== %s =====\n' "$1"; }
read_file() {
    printf '\n--- %s ---\n' "$1"
    if [ -f "$1" ] && [ -r "$1" ]; then
        cat "$1" 2>&1
    else
        printf 'Missing or unreadable in this context.\n'
    fi
}

{
    section 'DEVICE AND CONTEXT'
    date
    id
    uname -a
    getenforce
    for key in ro.product.model ro.build.PDA ro.build.fingerprint \
        ro.build.version.release ro.build.version.security_patch sys.boot_completed; do
        printf '%s=%s\n' "$key" "$(getprop "$key")"
    done
    read_file /proc/sys/kernel/random/boot_id
    read_file /proc/uptime
    read_file /proc/self/attr/current
    printf '\nCurrent namespace: '
    readlink /proc/self/ns/mnt
    printf 'Init namespace: '
    readlink /proc/1/ns/mnt

    section 'PAYLOAD R2 AND R3 PHASE LOGS'
    found=0
    for log in /data/local/tmp/rmg332-diag-*/phase.log /data/local/tmp/rmg333-diag-*/phase.log; do
        [ -f "$log" ] || continue
        found=1
        read_file "$log"
    done
    [ "$found" = 1 ] || printf 'No r2/r3 phase.log found.\n'

    section 'KERNELSU AND DEFEX (READ ONLY)'
    read_file /sys/module/kernelsu/parameters/samsung_defex_broad_root
    read_file /sys/module/kernelsu/parameters/allow_shell
    read_file /proc/modules
    printf '\nParameter and daemon files:\n'
    ls -ldZ /sys/module/kernelsu /sys/module/kernelsu/parameters \
        /sys/module/kernelsu/parameters/samsung_defex_broad_root \
        /data/adb/ksud /data/system/rmgnext-ksud \
        /data/local/tmp/ksud-s25u-kdp 2>&1
    for daemon in /data/adb/ksud /data/system/rmgnext-ksud \
        /data/local/tmp/ksud-s25u-kdp; do
        if [ -f "$daemon" ] && [ -r "$daemon" ]; then
            sha256sum "$daemon" 2>&1
        fi
    done

    section 'PROCESSES RETAINING PAYLOAD STATE'
    ps -A -o PID,PPID,USER,NAME 2>&1 | \
        grep -E 'PID|cve43499|ksud|rmg331|rmg332'

    section 'MODULE MARKERS (NO SCRIPT EXECUTION)'
    if [ -d /data/adb/modules ] && [ -r /data/adb/modules ]; then
        for module in /data/adb/modules/*; do
            [ -d "$module" ] || continue
            printf '\nmodule=%s\n' "${module##*/}"
            for marker in disable remove update; do
                [ ! -e "$module/$marker" ] || printf 'marker=%s\n' "$marker"
            done
            if [ -r "$module/module.prop" ]; then
                grep -E '^(id|name|version|versionCode)=' "$module/module.prop"
            fi
        done
    else
        printf 'Directory missing or unreadable in this context.\n'
    fi

    section 'METAMODULE AND PENDING UPDATES (READ ONLY)'
    printf '\nMetamodule target:\n'
    ls -ldZ /data/adb/metamodule 2>&1
    readlink /data/adb/metamodule 2>&1
    for metadata in /data/adb/metamodule/module.prop \
        /data/adb/modules/meta-overlayfsx/module.prop \
        /data/adb/modules_update/meta-overlayfsx/module.prop; do
        read_file "$metadata"
    done
    for directory in /data/adb/metamodule \
        /data/adb/modules/meta-overlayfsx /data/adb/modules_update/meta-overlayfsx; do
        printf '\nMarkers in %s:\n' "$directory"
        for marker in update remove disable; do
            ls -l "$directory/$marker" 2>&1
        done
    done
    printf '\nRelated mounts:\n'
    grep -E 'overlay|/data/adb|/dev/block/loop' /proc/self/mountinfo 2>&1
    printf '\nMetamodule image and mount point:\n'
    ls -ldZ /data/adb/metamodule/modules.img /data/adb/metamodule/mnt 2>&1
    printf '\nOverlayFS Enhanced log (last 262144 bytes):\n'
    if [ -r /data/adb/metamodule/overlayfsx.log ]; then
        tail -c 262144 /data/adb/metamodule/overlayfsx.log 2>&1
    else
        printf 'Log unavailable in this context.\n'
    fi
    for script in module.prop post-fs-data.sh metamount.sh metainstall.sh utils.sh; do
        path=/data/adb/metamodule/$script
        if [ -f "$path" ] && [ -r "$path" ]; then
            sha256sum "$path" 2>&1
        fi
    done

    section 'CURRENT KERNEL LOG'
    dmesg 2>&1 | tail -n 1600

    section 'PREVIOUS BOOT CRASH LOG'
    ls -la /sys/fs/pstore 2>&1
    found=0
    for file in /sys/fs/pstore/console-ramoops* /sys/fs/pstore/dmesg-ramoops*; do
        [ -f "$file" ] || continue
        found=1
        printf '\n--- %s (last 1048576 bytes) ---\n' "$file"
        if [ -r "$file" ]; then
            tail -c 1048576 "$file" 2>&1
        else
            printf 'Read permission denied in this context.\n'
        fi
    done
    [ "$found" = 1 ] || printf 'No console/dmesg ramoops visible.\n'

    section 'KERNEL CONFIGURATION'
    if [ -r /proc/config.gz ]; then
        if cat /proc/config.gz > "$output/config.gz"; then
            printf 'Copy saved as config.gz.\n'
        else
            printf 'Could not copy config.gz.\n'
        fi
    else
        printf '/proc/config.gz unavailable in this context.\n'
    fi
} > "$report" 2>&1

printf 'Collection complete: %s\n' "$output"
printf 'Share report.txt and config.gz if present.\n'
printf 'Permission failures are recorded; this collector does not request root.\n'
