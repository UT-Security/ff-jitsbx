#!/bin/bash

tasksetoutput=$(taskset -c -p 1 2>&1)
if [[ ",${tasksetoutput}," == *",2,"* ]]; then
  echo "WARNING: CPU 2 does not appear to be isolated when testing with the command 'taskset -c -p 1'. Isolate CPU 2 with isolcpus=2 kernel boot parameter. Will continue anyway, but benchmark results may not be as accurate."
  sleep 5
fi

# Setup Xvfb
if [ -z "$(pgrep Xvfb)" ]; then \
    Xvfb :99 & \
fi
export DISPLAY=:99

# disable_cpufreq
export CPUPOLICYINFO=($(cpufreq-info -c 0 -p))
sudo cpufreq-set -c 2 -g performance
sudo cpufreq-set -c 2 --min $((${CPUPOLICYINFO[1]}/2)) --max $((${CPUPOLICYINFO[1]}/2))

# disable_hyperthreading
export DEACTIVATED_HYPERTHREADS=0
if [ "$(cat /sys/devices/system/cpu/smt/active)" != "0" ]; then
  export DEACTIVATED_HYPERTHREADS=1;
  sudo bash -c "echo off > /sys/devices/system/cpu/smt/control"
fi

# Run the benchmark
CURR_TIME=$(date --iso=seconds)
mkdir -p ../benchmarks

CONFIGS_TO_TEST="stock wasm lfi largelfi"

./testsRunBenchmark "../benchmarks/firefox_lfi_graphite_$CURR_TIME" "graphite_perf_test" "$CONFIGS_TO_TEST"

# taskset -c 2 echo "hi"

# restore_cpufreq
sudo cpufreq-set -c 2 -g ${CPUPOLICYINFO[2]}
sudo cpufreq-set -c 2 --min ${CPUPOLICYINFO[0]} --max ${CPUPOLICYINFO[1]}
unset CPUPOLICYINFO

# restore_hyperthreading
if [ "$DEACTIVATED_HYPERTHREADS" == "1" ]; then
  sudo bash -c "echo on > /sys/devices/system/cpu/smt/control";
fi
unset DEACTIVATED_HYPERTHREADS

# Kill Xvfb
unset DISPLAY
pkill -f Xvfb
