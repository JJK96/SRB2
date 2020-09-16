#ifndef speedrun_h_INCLUDED
#define speedrun_h_INCLUDED

#include "p_local.h"
#include <mysql.h>

struct string {
  char *ptr;
  size_t len;
};

void speedrun_map_completed();
void send_best_time();
void finish_with_error(MYSQL *con);
char *time_to_string(int time);
void insert_score(MYSQL *con, int mapnum, char* username, char* skin, int time);
void insert_map(MYSQL *con, int mapnum, char *mapname);
void process_time(MYSQL *con, int playernum, int mapnum);
void speedrun_map_completed();
void init_string(struct string *s);
size_t write_to_string(void *ptr, size_t size, size_t nmemb, struct string *s);

#endif // speedrun_h_INCLUDED

