# axium

> *Before a kernel panics, the system is not chaotic.*
>
> *panix* captures the moment of failure. *axium* is concerned with what exists before that moment — the unnamed assumptions, the implicit structures, and the minimal conditions under which an exploit becomes possible.

## Philosophy

Axium is not a framework, and it is not a collection of tricks. It is a set of low-level primitives refined through repetition: touching memory, interfering with object lifetimes, and shifting the boundaries between data flow and control flow.

There is no “automatic exploitation” here. Only a dialogue between you and the kernel about causality, assumptions, and cost.

## Usage

Axium makes it easy to iterate on exploits without fighting the build system.

### Build the Workspace

The default workspace is `exp.c`.

```bash
make
```

### Build your exploitation script

Build and link any `.c` file with axium primitives on the fly:

```bash
# To build my_exp.c into the executable 'my_exp'
make my_exp
```

Or specify the source explicitly:

```bash
make SRC=my_exp.c
```

### Bundling for Submission

Axium can generate a single "amalgamated" C file that includes both your script and the entire library source. This is ideal for CTF write-ups or environments where you cannot easily upload multiple files.

```bash
# Generates exp_bundled.c (from exp.c)
make bundle

# Or for a specific script (generates my_exp_bundled.c)
make SRC=my_exp.c bundle
```

*Note: Building any script (e.g., `make my_exp`) automatically generates its bundled version.*

### Run Test Suites

Every file in `tests/*.c` is treated as an independent binary.

```bash
make test
```

### Cleanup

```bash
make clean
```
