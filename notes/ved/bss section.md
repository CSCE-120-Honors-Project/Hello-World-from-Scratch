uninitialized variables

compiler finds global/static vars that are uninitialized

LINKER DECIDES TO NOT HARDCODE THESE EMPTY BYTES
instead it just says "i want this many bytes zeroed for uninitialized vars"

so we dont waste disk space

OS start.s code zeros out that space in ram during runtime
