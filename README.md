# axium

> *Before a kernel panics, the system is not chaotic.*
>
> *panix* captures the moment of failure. *axium* is concerned with what exists before that moment — the unnamed assumptions, the implicit structures, and the minimal conditions under which an exploit becomes possible.

## Usage

Axium makes it easy to iterate on exploits without fighting the build system.

### Build the workspace

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

### Bundling for submission

Axium can generate a single "amalgamated" C file that includes both your script and the entire library source. This is ideal for CTF write-ups or environments where you cannot easily upload multiple files.

```bash
# Generates exp_bundled.c (from exp.c)
make bundle

# Or for a specific script (generates my_exp_bundled.c)
make SRC=my_exp.c bundle
```

*Note: Building any script (for example, `make my_exp`) automatically generates its bundled version.*

### Cleanup

```bash
make clean
```

### For Developers

```bash
bear -- make
```

#### Run test suites

Every file in `src/tests/**/*.c` is treated as an independent binary.

```bash
make test
```

## Screenshots

Dashboard for attack single process self:

<center>
  <img src="https://github.com/CuB3y0nd/picx-images-hosting/raw/master/.9kgoy7vdgk.avif" alt="" />
</center>

Dashboard for attack different process:

<center>
  <img src="https://github.com/CuB3y0nd/picx-images-hosting/raw/master/.77e2h0hk9y.avif" alt="" />
</center>

## Sponsors

Special thanks to **Vlex** for donating the domain [hyphony.com](https://hyphony.com) to this project.
