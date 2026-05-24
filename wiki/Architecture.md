# Architecture & Security

## Cryptography
RChat relies entirely on `libsodium` for cryptography.
- **E2E Algorithm:** `crypto_secretbox_easy` (XSalsa20 stream cipher, Poly1305 MAC).
- **Password Hashing:** `crypto_generichash` (BLAKE2b).

## Network Layer
- **TCP Fragmentation:** A raw TCP socket does not guarantee complete messages. RChat implements a dynamic ring buffer (`ByteBuffer`) on both the client and the server. Messages are delimited by `\n`. If a packet is fragmented, the buffer waits until the newline character arrives before passing the payload to the event loop.
- **Asynchronous I/O:** The server utilizes `epoll`, while the client utilizes `select()`. Both use `O_NONBLOCK` sockets to prevent deadlocks.

## Threat Models Handled
- **Replay Attacks:** The client caches the last 2048 nonces. If a server administrator attempts to replay a captured encrypted packet to the room, the client will silently drop it and warn the user.
- **Traffic Analysis:** Before encryption, the plaintext is padded with null bytes up to the nearest 256-byte block. An adversary cannot determine the length of your messages.
