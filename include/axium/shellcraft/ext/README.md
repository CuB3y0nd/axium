# Shellcraft Extensions

This directory is dedicated to user-defined Shellcode Snippets.

### Why store them here?

1. **Decoupling**: Avoid modifying the core `axium` library files.
2. **Convenience**: By keeping snippets in `.h` files, you can simply include them in your exploit script without modifying the Makefile.
3. **Safety**: It is recommended to use `static` for template definitions to prevent symbol collisions in multi-file projects.

### How to get started?

1. Refer to `template.h` in this directory.
2. Create your own header file, e.g., `my_snippets.h`.
3. Include it in your `exp.c`: `#include <axium/shellcraft/ext/my_snippets.h>`.
