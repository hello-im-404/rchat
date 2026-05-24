#ifndef SERVER_AUTH_H
#define SERVER_AUTH_H

#define AUTH_DB_FILE "users.db"
#define EXPIRE_DAYS 30

void auth_init(void);
int auth_is_registered(const char *nick);
int auth_login(const char *nick, const char *client_hash);
int auth_register(const char *nick, const char *client_hash);
void auth_update_timestamp(const char *nick);
void auth_cleanup_expired(void);

#endif
