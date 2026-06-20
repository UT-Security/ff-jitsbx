#!/bin/bash

# Setup Xvfb
if [ -z "$(pgrep Xvfb)" ]; then \
    Xvfb :99 & \
fi
export DISPLAY=:99

# disable_cpufreq
sudo cpufreq-set -c 2 -g performance
sudo cpufreq-set -c 2 --min 2200MHz --max 2200MHz

# disable_hyperthreading
export DEACTIVATED_HYPERTHREADS=0
if [ "$(cat /sys/devices/system/cpu/smt/active)" != "0" ]; then
  export DEACTIVATED_HYPERTHREADS=1;
  sudo bash -c "echo off > /sys/devices/system/cpu/smt/control"
fi

# isolate_cpu2
CGROOT=/sys/fs/cgroup
CGDIR=$CGROOT/benchmark.slice

sudo mkdir -p $CGDIR

sudo systemctl stop benchmark.slice # extra check to stop any other programs

if ! grep -q "cpuset" ${CGROOT}/cgroup.subtree_control; then
  /bin/echo "+cpuset" | sudo tee $CGROOT/cgroup.subtree_control
fi

POLICY=$(cat ${CGDIR}/cpuset.cpus.partition)
if [ "$POLICY" != "root" ]; then
  /bin/echo root | sudo tee $CGDIR/cpuset.cpus.partition
fi
/bin/echo 2 | sudo tee $CGDIR/cpuset.cpus.exclusive
/bin/echo 2 | sudo tee $CGDIR/cpuset.cpus

# Run the benchmark
CURR_TIME=$(date --iso=seconds)
mkdir -p ../benchmarks

CONFIGS_TO_TEST="stock wasm lfi largelfi"
if [[ "$(uname -m)" != "x86_64" ]]; then
  CONFIGS_TO_TEST="stock wasm lfi"
fi

./testsRunBenchmark "../benchmarks/firefox_lfi_graphite_$CURR_TIME" "graphite_perf_test" "$CONFIGS_TO_TEST"

# restore_cpu2
sudo systemctl stop benchmark.slice # extra check to stop any other programs
/bin/echo "" | sudo tee $CGDIR/cpuset.cpus.exclusive
/bin/echo "" | sudo tee $CGDIR/cpuset.cpus

# restore_cpufreq
POLICYINFO=($(cpufreq-info -c 0 -p)) && \
sudo cpufreq-set -c 2 -g ${POLICYINFO[2]} && \
sudo cpufreq-set -c 2 --min ${POLICYINFO[0]}MHz --max ${POLICYINFO[1]}MHz

# restore_hyperthreading
if [ "$DEACTIVATED_HYPERTHREADS" == "1" ]; then
  sudo bash -c "echo on > /sys/devices/system/cpu/smt/control";
fi
unset DEACTIVATED_HYPERTHREADS

# Kill Xvfb
unset DISPLAY
pkill -f Xvfb
