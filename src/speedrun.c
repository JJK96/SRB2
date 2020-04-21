#include "speedrun.h"
#include "console.h"
#include "doomdef.h"
#include "g_game.h"
#include "r_things.h"
#include <stdio.h>
#include <mysql.h>
#include "credentials.h"

#define QUERY_LEN 100
#define TIME_STRING_LEN 10

#define GET_MAP "select * from maps where id = %d"
#define INSERT_MAP "insert into maps (id, name) values (?, ?)"
#define GET_SCORE "select time from highscores where username = ? and skin = ? and map_id = ?"
#define INSERT_SCORE "insert into highscores (username, skin, map_id, time, time_string) values (?, ?, ?, ?, ?)"
#define UPDATE_SCORE "update highscores set time = ?, time_string = ? where username = ? and skin = ? and map_id = ?"

void finish_with_error(MYSQL *con)
{
  fprintf(stderr, "%s\n", mysql_error(con));
  mysql_close(con);
  //exit(1);
}

char *time_to_string(int time)
{
    char *time_string = malloc(TIME_STRING_LEN);
    snprintf(time_string, TIME_STRING_LEN, 
        "%i:%02i.%02i"
        , G_TicsToMinutes(time,true)
        , G_TicsToSeconds(time)
        , G_TicsToCentiseconds(time));
    return time_string;
}

void update_score(MYSQL *con, int mapnum, char* username, char* skin, int time)
{
    MYSQL_STMT* stmt = mysql_stmt_init(con);
    MYSQL_BIND bind[5];
    memset(bind, 0, sizeof(bind));
    unsigned long username_length = strlen(username);
    unsigned long skin_length = strlen(skin);
    char * time_string = time_to_string(time);
    unsigned long time_string_length = strlen(time_string);

    if (mysql_stmt_prepare(stmt, UPDATE_SCORE, strlen(UPDATE_SCORE))) {
        finish_with_error(con);
    }

    bind[0].buffer_type = MYSQL_TYPE_LONG;
    bind[0].buffer = (char *)&time;
    bind[0].is_null = 0;
    bind[0].length = 0;

    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].buffer = time_string;
    bind[1].buffer_length = time_string_length;
    bind[1].is_null = 0;
    bind[1].length = &time_string_length;

    bind[2].buffer_type = MYSQL_TYPE_STRING;
    bind[2].buffer = username;
    bind[2].buffer_length = username_length;
    bind[2].is_null = 0;
    bind[2].length = &username_length;

    bind[3].buffer_type = MYSQL_TYPE_STRING;
    bind[3].buffer = skin;
    bind[3].buffer_length = skin_length;
    bind[3].is_null = 0;
    bind[3].length = &skin_length;

    bind[4].buffer_type = MYSQL_TYPE_LONG;
    bind[4].buffer = (char *)&mapnum;
    bind[4].is_null = 0;
    bind[4].length = 0;

    if (mysql_stmt_bind_param(stmt, bind)) {
        finish_with_error(con);
    }

    if (mysql_stmt_execute(stmt)) {
        finish_with_error(con);
    }

    if (mysql_stmt_close(stmt)) {
        fprintf(stderr, "%s\n", mysql_error(con));
        finish_with_error(con);
    }
    free(time_string);

}

/*
 * Insert score into database
 */
void insert_score(MYSQL *con, int mapnum, char* username, char* skin, int time)
{
    MYSQL_STMT* stmt = mysql_stmt_init(con);
    MYSQL_BIND bind[5];
    memset(bind, 0, sizeof(bind));
    unsigned long username_length = strlen(username);
    unsigned long skin_length = strlen(skin);

    char * time_string = time_to_string(time);
    unsigned long  time_string_length = strlen(time_string);

    if (mysql_stmt_prepare(stmt, INSERT_SCORE, strlen(INSERT_SCORE))) {
        finish_with_error(con);
    }

    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = username;
    bind[0].buffer_length = username_length;
    bind[0].is_null = 0;
    bind[0].length = &username_length;

    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].buffer = skin;
    bind[1].buffer_length = skin_length;
    bind[1].is_null = 0;
    bind[1].length = &skin_length;

    bind[2].buffer_type = MYSQL_TYPE_LONG;
    bind[2].buffer = (char *)&mapnum;
    bind[2].is_null = 0;
    bind[2].length = 0;

    bind[3].buffer_type = MYSQL_TYPE_LONG;
    bind[3].buffer = (char *)&time;
    bind[3].is_null = 0;
    bind[3].length = 0;

    bind[4].buffer_type = MYSQL_TYPE_STRING;
    bind[4].buffer = time_string;
    bind[4].buffer_length = time_string_length;
    bind[4].is_null = 0;
    bind[4].length = &time_string_length;

    if (mysql_stmt_bind_param(stmt, bind)) {
        finish_with_error(con);
    }

    if (mysql_stmt_execute(stmt)) {
        finish_with_error(con);
    }

    if (mysql_stmt_close(stmt)) {
        fprintf(stderr, "%s\n", mysql_error(con));
        finish_with_error(con);
    }
    free(time_string);

}

/*
 * Returns the current best time for the given username
 * 
 * @Returns -1 if no time found
 */
int get_time(MYSQL* con, int mapnum, char* username, char* skin)
{
    MYSQL_STMT* stmt;
    MYSQL_BIND bind[3];

    stmt = mysql_stmt_init(con);
    if (mysql_stmt_prepare(stmt, GET_SCORE, strlen(GET_SCORE))) {
        finish_with_error(con);
    }
    memset(bind, 0, sizeof(bind));
    unsigned long str_length = strlen(username);
    unsigned long skin_length = strlen(skin);

    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = username;
    bind[0].buffer_length = str_length;
    bind[0].is_null = 0;
    bind[0].length = &str_length;

    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].buffer = skin;
    bind[1].buffer_length = skin_length;
    bind[1].is_null = 0;
    bind[1].length = &skin_length;

    bind[2].buffer_type = MYSQL_TYPE_LONG;
    bind[2].buffer = (char *)&mapnum;
    bind[2].is_null = 0;
    bind[2].length = 0;

    if (mysql_stmt_bind_param(stmt, bind)) {
        finish_with_error(con);
    }

    if (mysql_stmt_execute(stmt)) {
        finish_with_error(con);
    }

    int time;
    bind[1].buffer = (char *) &time;

    if (mysql_stmt_bind_result(stmt, &bind[1])) {
        finish_with_error(con);
    }

    int res = mysql_stmt_fetch(stmt);
    if(res == 1) {
        finish_with_error(con);
    } else if (res == MYSQL_NO_DATA) {
        //No best time yet
        time = -1;
    }

    if (mysql_stmt_close(stmt)) {
        fprintf(stderr, "%s\n", mysql_error(con));
        finish_with_error(con);
    }

    return time;
}

/*
 * Insert the map if it is not yet in the database
 */
void insert_map(MYSQL *con, int mapnum, char *mapname)
{
    char query[QUERY_LEN];
    MYSQL_RES *result;
    MYSQL_STMT* stmt;
    MYSQL_BIND bind[2];

    snprintf(query, QUERY_LEN, GET_MAP, mapnum);
    if (mysql_query(con, query)) {
        finish_with_error(con);
    }

    result = mysql_store_result(con);

    if (result == NULL) 
    {
        finish_with_error(con);
    }

    if (mysql_num_rows(result) == 0) {
        //The map does not exist yet.
        //insert it
        stmt = mysql_stmt_init(con);
        if (mysql_stmt_prepare(stmt, INSERT_MAP, strlen(INSERT_MAP))) {
            finish_with_error(con);
        }
        memset(bind, 0, sizeof(bind));
        unsigned long str_length = strlen(mapname);

        bind[0].buffer_type = MYSQL_TYPE_LONG;
        bind[0].buffer = (char *)&mapnum;
        bind[0].is_null = 0;
        bind[0].length = 0;

        bind[1].buffer_type = MYSQL_TYPE_STRING;
        bind[1].buffer = mapname;
        bind[1].buffer_length = strlen(mapname);
        bind[1].is_null = 0;
        bind[1].length = &str_length;

        if (mysql_stmt_bind_param(stmt, bind)) {
            finish_with_error(con);
        }

        if (mysql_stmt_execute(stmt)) {
            finish_with_error(con);
        }

        if (mysql_stmt_close(stmt)) {
            fprintf(stderr, "%s\n", mysql_error(con));
            finish_with_error(con);
        }
    }

    mysql_free_result(result);
}

void process_time(MYSQL *con, int playernum, int mapnum) {
    int time = players[playernum].realtime;
    char* username = player_names[playernum];
    char* skin = ((skin_t *)players[playernum].mo->skin)->name;
    int res = get_time(con, mapnum, username, skin);
    int best_time;

    if (res == -1) {
        insert_score(con, mapnum, username, skin, time);
    } else {
        best_time = res;
        if (time < best_time) {
            //Need to update
            update_score(con, mapnum, username, skin, time);
        }
    }
}

void speedrun_map_completed()
{
    MYSQL *con = mysql_init(NULL);

    int mapnum = gamemap-1;
    char mapname[30];
    if (mapheaderinfo[gamemap-1]->actnum) {
        snprintf(mapname, 30, "%s %d", mapheaderinfo[gamemap-1]->lvlttl, mapheaderinfo[gamemap-1]->actnum);
    } else {
        snprintf(mapname, 30, "%s", mapheaderinfo[gamemap-1]->lvlttl);
    }

    if (con == NULL) 
    {
        fprintf(stderr, "%s\n", mysql_error(con));
        exit(1);
    }

    if (mysql_real_connect(con, "localhost", USERNAME, PASSWORD, 
            DATABASE, 0, NULL, 0) == NULL) 
    {
        finish_with_error(con);
    }  

    insert_map(con, mapnum, mapname);
	for (int playernum = 0; playernum < MAXPLAYERS; playernum++) {
		if (!playeringame[playernum] || players[playernum].spectator || players[playernum].pflags & PF_GAMETYPEOVER)
			continue;
    	process_time(con, playernum, mapnum);
	}

    mysql_close(con);

}
