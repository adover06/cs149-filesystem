# cs149-filesystem

Simulated file system for CS-149 (spec: Canvas Week 14 "Project Description").

| File | Owner |
|---|---|
| `filesystem.h` | shared contract (Dover) |
| `main.c`, `test.c` | Dover |
| `disk.c` | Porasl (free list, initDir, initDisk). Until it exists: `stub_disk.c` |
| `commands.c` | De Luna (CREATE, DELETE, OPEN, ...). Until it exists: `stub_commands.c` |

Build (swap each stub for the real file once it's written, then delete the stub):

```
gcc -Wall -o fs   main.c stub_disk.c stub_commands.c
gcc -Wall -o test test.c stub_disk.c stub_commands.c
```

Final build: `gcc -Wall -o fs main.c disk.c commands.c`
