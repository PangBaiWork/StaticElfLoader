# Static ELF Loader
Frida cant work with statically linked binary, so I created this loader to load the elf file and it works  fine.
note: please note that you need to hook after load_elf() executed.

```bash
./loader static_elf args
```