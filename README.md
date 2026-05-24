# RChat

RChat is a modern, ultra-lightweight, and fully anonymous terminal chat application written in pure C.

## Installation & Build

### 1. Requirements
- Linux or any UNIX-like OS (Server requires Linux for `epoll`)
- `gcc` or `clang`
- `make`
- `libsodium-dev`
- `libncurses-dev`

On Ubuntu/Debian:
```bash
sudo apt update
sudo apt install build-essential libsodium-dev libncurses-dev
```

### 2. Compile
```bash
make
```

### 3. Run
**Server:**
```bash
./build/bin/server 4040
```
**Client:**
```bash
./build/bin/client
```

---
**Documentation:** Please see the [Wiki](wiki/README.md) for comprehensive guides on usage, commands, hosting your own server, and architecture details.
