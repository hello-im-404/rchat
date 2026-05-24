# Hosting a Server

RChat's server is built on pure C using `epoll` (Edge Triggered), similar to Nginx. It is capable of handling over 10,000 concurrent connections on a single CPU thread.

## Running the Server
```bash
./build/bin/server 4040
```

## OS Tuning for 10k+ Connections
By default, Linux limits a single process to 1024 open file descriptors. To host a massive server, you must raise this limit.
Before starting the server, run:
```bash
ulimit -n 65535
```

## Security & Anonymity
- **Zero Logging:** The server software intentionally lacks any `fprintf` or logging functions that record user IPs or message metadata.
- **Blind Routing:** When users use encrypted rooms, the server only sees padded hexadecimal strings. It acts solely as a blind router.
- **Password Safety:** When users register their nicknames, the client hashes the password *before* sending it. The server then hashes this hash again before saving it to `users.db`. Even if your server is compromised, the attackers cannot extract the users' original passwords.

## Database
The server maintains a lightweight local file called `users.db` in the working directory. It stores:
`Nickname | Double-Hash | Last Login Timestamp`
Inactive accounts are automatically purged after 30 days.

## Room Administration
The first user to join an empty room automatically becomes its Administrator. They can use the `/ban <nickname>` command to ban abusive users by IP (bans are stored in RAM and clear on server restart).
