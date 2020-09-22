#ifndef speedrun_h_INCLUDED
#define speedrun_h_INCLUDED

#include "p_local.h"
#include <mysql.h>

#define QUERY_LEN 100
#define TIME_STRING_LEN 10
#define MSG_LEN 254
#define MSG_BUF_LEN 20

// define the macros for the statements
#define GET_MAP "select * from maps where id = %d"
#define INSERT_MAP "insert into maps (id, name) values (?, ?)"
#define GET_SCORE "select time from highscores where username = ? and skin = ? and map_id = ?"
#define INSERT_SCORE "insert into highscores (time, time_string, username, skin, map_id, datetime) values (?, ?, ?, ?, ?, NOW())"

#define BEST_SCORE_ON_MAP_URL "https://srb2circuit.eu/highscores/api/bestformaps?map_id=%d&all_skins=on"

struct string {
  char *ptr;
  size_t len;
};

typedef struct {
    char *msgs[MSG_BUF_LEN];
    size_t index;
} msg_buf_t;

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
void add_message(char *msg);
void send_message();

#endif // speedrun_h_INCLUDED

