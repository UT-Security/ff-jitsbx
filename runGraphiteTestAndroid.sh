#!/usr/bin/env bash

# Android counterpart of runGraphiteTest.sh + testsRunBenchmark.
#
# Talos does not drive GeckoView, so this replaces it: for each config we
# install the APK, load graphite_perf_test_android.html, and read the
# Capture_Time samples that CheckGraphiteString emits via printf_stderr
# (-> logcat, tag "Gecko"). Termination is by sample count, the analogue of
# talos's tppagecycles -- the page never signals completion and never closes
# the browser.

set -e
set -o pipefail

cd "$(dirname "$0")"
FFBuildDir=$(realpath ./)

SAMPLES="${SAMPLES:-100}"
PORT="${PORT:-8000}"
CONFIGS_TO_TEST="${CONFIGS_TO_TEST:-stock wasm lfi largelfi}"

PIN_CORE="${PIN_CORE:-8}"

APP=org.mozilla.geckoview_example
ACTIVITY="$APP/.GeckoViewActivity"
TEST_URL="http://localhost:$PORT/tests/graphite_perf_test/graphite_perf_test_android.html"

PAGE_ITERATIONS=110
SECONDS_PER_SAMPLE=16
RUN_TIMEOUT="${RUN_TIMEOUT:-$((SAMPLES * SECONDS_PER_SAMPLE + 240))}"

###################################### some checks

if [ "$SAMPLES" -gt "$PAGE_ITERATIONS" ]; then
    echo "SAMPLES=$SAMPLES exceeds the $PAGE_ITERATIONS iterations hardcoded in" >&2
    echo "graphite_perf_test_android.html; the run would time out." >&2
    exit 1
fi

# Fail fast rather than after hours of benchmarking: the analysis step needs it.
if ! python3 -c "import simplejson" >/dev/null 2>&1; then
    echo "python3 module 'simplejson' is missing (needed by testsAnalyzeExtractedLogs.py)." >&2
    echo "Use the nix devShell: nix develop ./flake#android" >&2
    exit 1
fi

if [ -z "$(adb devices | sed -n '2p')" ]; then
    echo "No adb device attached." >&2
    exit 1
fi

# param: config name -> path of the APK built by buildAndroid.sh
function apkFor() {
    echo "$FFBuildDir/obj_firefox_latest_android_${1}_release/gradle/build/mobile/android/geckoview_example/outputs/apk/withGeckoBinaries/debug/geckoview_example-withGeckoBinaries-debug.apk"
}

for cfg in $CONFIGS_TO_TEST; do
    if [ ! -f "$(apkFor "$cfg")" ]; then
        echo "Missing APK for $cfg: $(apkFor "$cfg")" >&2
        echo "Run ./buildAndroid.sh first." >&2
        exit 1
    fi
done

CURR_TIME=$(date --iso=seconds)
mkdir -p ../benchmarks
OUTPUTPATH="$(realpath ../benchmarks)/firefox_lfi_graphite_android_$CURR_TIME"
mkdir -p "$OUTPUTPATH"

###################################### Pre-benchmark setup

HTTPD_PID=""
LOGCAT_PID=""
ORIG_STAYON=$(adb shell settings get global stay_on_while_plugged_in 2>/dev/null | tr -d '\r')

function cleanup() {
    set +e
    if [ -n "$LOGCAT_PID" ]; then
        kill "$LOGCAT_PID" >/dev/null 2>&1
    fi
    adb shell am force-stop "$APP" >/dev/null 2>&1
    if [ -n "$ORIG_STAYON" ]; then
        adb shell settings put global stay_on_while_plugged_in "$ORIG_STAYON" >/dev/null 2>&1
    fi
    adb reverse --remove "tcp:$PORT" >/dev/null 2>&1
    if [ -n "$HTTPD_PID" ]; then
        kill "$HTTPD_PID" >/dev/null 2>&1
    fi
}
trap cleanup EXIT
# Ctrl-C / kill: exit so the EXIT trap runs cleanup exactly once. Without this a
# long run interrupted midway can leave the http server, the adb reverse mapping
# and the stay_on override behind.
trap 'exit 130' INT TERM

# I don't know why file:// does not load in this build, so the test is served
# over adb reverse. `--directory testing/talos/talos` makes the device path
# match tests.manifest.
python3 -m http.server "$PORT" --bind 127.0.0.1 --directory ./testing/talos/talos \
    >"$OUTPUTPATH/httpd.log" 2>&1 &
HTTPD_PID=$!
sleep 2

if ! kill -0 "$HTTPD_PID" 2>/dev/null; then
    echo "Failed to start http server on port $PORT:" >&2
    cat "$OUTPUTPATH/httpd.log" >&2
    exit 1
fi

adb reverse "tcp:$PORT" "tcp:$PORT" >/dev/null

######################################

function verifyPinned() {
    adb shell su -c true >/dev/null 2>&1 || return 0

    local tabpid
    tabpid=$(adb shell ps -A -o PID,NAME 2>/dev/null | grep "${APP}:tab" | awk '{print $1}' | head -1)
    [ -n "$tabpid" ] || return 0

    if adb shell su -c "grep -h Cpus_allowed_list /proc/$tabpid/task/*/status" 2>/dev/null \
        | grep -qE "Cpus_allowed_list:[[:space:]]*$PIN_CORE\$"; then
        echo "    pinned to core $PIN_CORE (content pid $tabpid)"
    else
        echo "No thread in the content process is pinned to core $PIN_CORE." >&2
        echo "GRAPHITE_PIN_CORE did not take effect; results would be unpinned." >&2
        exit 1
    fi
}

# param: config name
function runTest() {
    local cfg="$1"
    local outfile="$OUTPUTPATH/${cfg}_terminal_output.txt"

    echo "--- $cfg: installing"

    adb uninstall "$APP" >/dev/null 2>&1 || true

    # Install the APK buildAndroid.sh already produced, its faster than
    # `mach android install-geckoview_example`.
    if ! adb install -r "$(apkFor "$cfg")" 2>&1 | tee "$OUTPUTPATH/${cfg}_install.log" | grep -q "^Success"; then
        echo "adb install failed for $cfg (see $OUTPUTPATH/${cfg}_install.log)." >&2
        exit 1
    fi

    # Screen on
    adb shell input keyevent KEYCODE_WAKEUP >/dev/null 2>&1
    # unlock screen (let us use foreground cores)
    adb shell wm dismiss-keyguard >/dev/null 2>&1
    # stay awake while it has a usb cable plugged
    adb shell svc power stayon usb >/dev/null 2>&1
    # kill the app
    adb shell am force-stop "$APP" >/dev/null 2>&1
    adb logcat -c
    sleep 2

    echo "--- $cfg: collecting $SAMPLES samples on core $PIN_CORE (~$((SAMPLES * SECONDS_PER_SAMPLE / 60)) min)"

    adb shell am start -n "$ACTIVITY" -d "$TEST_URL" \
        --es env0 "GRAPHITE_PIN_CORE=$PIN_CORE" \
        --es env1 "MOZ_LOG=MTETestLog:5" >/dev/null
    sleep 5

    if ! adb shell dumpsys window 2>/dev/null | grep -E "mCurrentFocus|mFocusedApp" | grep -q "$APP"; then
        echo "$APP is not the focused window; results would be invalid (background throttling)." >&2
        echo "Unlock the device (remove the PIN/swipe lock) and retry." >&2
        exit 1
    fi

    # Keep polling the background logcat until we reach the sample size.
    local rawlog="$OUTPUTPATH/${cfg}_logcat.txt"
    adb logcat -s Gecko:I > "$rawlog" 2>/dev/null &
    LOGCAT_PID=$!

    local deadline=$((SECONDS + RUN_TIMEOUT))
    local got=0
    local prev=-1
    local pinChecked=0
    while [ "$got" -lt "$SAMPLES" ] && [ "$SECONDS" -lt "$deadline" ]; do
        sleep 5
        got=$(grep -c 'Capture_Time:Graphite' "$rawlog" 2>/dev/null) || got=0
        if [ "$got" -ne "$prev" ]; then
            echo "    $got/$SAMPLES samples"
            prev=$got
        fi
        # Verify the pin once shaping has actually happened -- the affinity is set
        # when GrSandboxData is first constructed, which is not until the first
        # sample, so checking any earlier gives a false negative.
        if [ "$got" -ge 1 ] && [ "$pinChecked" -eq 0 ]; then
            pinChecked=1
            verifyPinned
        fi
    done

    kill "$LOGCAT_PID" >/dev/null 2>&1 || true
    wait "$LOGCAT_PID" 2>/dev/null || true
    LOGCAT_PID=""

    grep -oE 'Capture_Time:Graphite,[0-9]+,[0-9]+,[0-9]+\|' "$rawlog" \
        | head -n "$SAMPLES" > "$outfile" || true

    adb shell am force-stop "$APP" >/dev/null 2>&1

    got=$(wc -l < "$outfile")
    if [ "$got" -lt "$SAMPLES" ]; then
        echo "Only collected $got/$SAMPLES samples for $cfg (timeout ${RUN_TIMEOUT}s)." >&2
        exit 1
    fi
    echo "--- $cfg: $got samples -> $outfile"
}

for cfg in $CONFIGS_TO_TEST; do
    runTest "$cfg"
done

######################################

for cfg in $CONFIGS_TO_TEST; do
    ./testsExtractFromLogs.py "$OUTPUTPATH/${cfg}_terminal_output.txt" \
        > "$OUTPUTPATH/${cfg}_terminal_analysis.json"
done

./testsAnalyzeExtractedLogs.py "$OUTPUTPATH/"

echo "-----------------------------------"
echo "Results are saved in \"$OUTPUTPATH/\""
echo "-----------------------------------"
cat "$OUTPUTPATH/compare_stock_terminal_analysis.json.dat"
