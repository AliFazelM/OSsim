# OSsim — OS Process Scheduler & Deadlock Detection Simulator

A single-threaded operating system simulator written in C. It schedules a
set of processes competing for a fixed pool of resource types, and detects
(and recovers from) deadlocks using an algorithm based on the **Banker's
Algorithm**.

## Overview

The simulator reads a workload description from stdin: a number of
processes, a number of resource types with their available instance
counts, and — for each process — a list of instructions to execute. It
then runs a simulation loop that:

- Picks the next ready process (shortest scheduled run time first, via a
  binary-tree-based ready queue)
- Executes its instructions one at a time (`Run`, `Allocate`, `Free`,
  `Sleep`, plus stubs for `Read`/`Write`)
- Blocks a process on a wait queue when it requests more of a resource
  than is currently available, or when it calls `Sleep`
- After every step, runs a Banker's-Algorithm-style safety check across
  all processes to detect deadlock; if a deadlock is found, it force-frees
  the blocking allocation from a deadlocked process to break the cycle
- Wakes up sleeping/blocked processes once their conditions are satisfied
- Logs every scheduling event (`EXECUTE`, `GIVE`, `TAKE`, `WAIT`,
  `DID NOT EXECUTE`, `DEADLOCK TAKE`, `EMERGENCY TAKE`) with timestamps

## Files

| File | Description |
|---|---|
| `os.c` | Entry point and the core scheduling / deadlock-detection loop |
| `process.c` | Process struct helpers, instruction parsing, and the ready-queue (binary tree) implementation |
| `process.h` | Shared type and function declarations |
| `code.c` | Small helper/test source file |
| `input`, `input2`, `input3` | Sample input files for manual testing |
| `os`, `a.out` | Precompiled binaries (already built from `os.c`; included as reference/checkpoint artifacts) |

## Building

```bash
gcc -o os os.c process.c
```

## Running

```bash
./os < input
```

## Input format

```
n                    # number of processes
m                    # number of resource types
r1 r2 ... rm         # available instances of each resource type
<per-process blocks, one per process, n times>:
    ic               # number of instructions for this process
    <instruction 1>
    <instruction 2>
    ...
```

Each instruction is one of:

| Instruction | Args | Meaning |
|---|---|---|
| `Run t` | duration | Occupy the CPU for `t` time units |
| `Sleep t` | duration | Block for `t` time units, then return to ready queue |
| `Allocate n r` | amount, resource id | Request `n` units of resource `r`; blocks if unavailable |
| `Free n r` | amount, resource id | Release `n` units of resource `r` back to the pool |

See `input`, `input2`, and `input3` for worked examples.

## Output format

The simulator prints the number of log lines produced, followed by one
event per line, e.g.:

```
EXECUTE <pid> <start_time> <end_time>
GIVE <pid> <amount> <resource> <time>
TAKE <pid> <amount> <resource> <time>
WAIT <pid> <start_time> <end_time>
DID NOT EXECUTE <pid>
DEADLOCK TAKE <pid> <amount> <resource> <time>
EMERGENCY TAKE <pid> <amount> <resource> <time>
```

## Context

This project was built as coursework for an Operating Systems course,
focused on process scheduling and deadlock avoidance/detection.
