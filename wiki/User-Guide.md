# User Guide

## Getting Started
Run `./build/bin/client` to start. You will be greeted by the ASCII art and prompted for a nickname.

## Commands

- `/connect <ip>:<port>` - Connects to an RChat server. (e.g. `/connect 127.0.0.1:4040`)
- `/list` - If disconnected, lists your previously visited servers. If connected, lists active rooms on the server.
- `/nickname <new_nick>` - Changes your nickname on the fly.
- `/join <room> [password]` - Enters a chat room. **If you provide a password, all messages will be End-to-End Encrypted.** The server will not be able to read them.
- `/register <password>` - Registers your current nickname on the server so no one else can take it.
- `/login <password>` - Authenticates you if your nickname is registered.
- `/exit` or `/quit` - Closes the client.

## Nickname Protection (NickServ)
When you connect to a server, you can protect your identity. By typing `/register my_secret_password`, the server will securely save a hash of your password. 
If you don't log in for 30 days, your nickname registration will expire and someone else can claim it.

*Note: Your password is mathematically hashed on your client before being sent. The server administrator cannot see or reverse-engineer your password.*

## E2E Encryption
When you use `/join secret_room my_password`, your client uses `libsodium` (XSalsa20-Poly1305) to encrypt every message before it leaves your computer. Anyone else who joins the room must provide the exact same password to read the messages.
