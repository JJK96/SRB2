#include "speedrun.h"
#include "console.h"
#include "doomdef.h"
#include "g_game.h"
#include "r_things.h"
#include <stdio.h>
#include <mysql.h>
#include <stdbool.h>
#include "credentials.h"

#define QUERY_LEN 100
#define TIME_STRING_LEN 10

// define the macros for the statements
#define GET_MAP "select * from maps where id = %d"
#define INSERT_MAP "insert into maps (id, name) values (?, ?)"
#define GET_SCORE "select time from highscores where username = ? and skin = ? and map_id = ?"
#define INSERT_SCORE "insert into highscores (time, time_string, username, skin, map_id, datetime) values (?, ?, ?, ?, ?, NOW())"
#define UPDATE_SCORE "update highscores set time = ?, time_string = ?, datetime = NOW() where username = ? and skin = ? and map_id = ?"

// Exits with an error
void finish_with_error(MYSQL *con)
{
  fprintf(stderr, "%s\n", mysql_error(con));
  mysql_close(con);
  //exit(1);
}

// Converts the time from tics into a string mm:ss.cc
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

// Insert score into the database
void insert_score(MYSQL *con, int mapnum, char* username, char* skin, int time, bool time_found)
{
    // setup the statement handler and statement binds
    MYSQL_STMT* stmt = mysql_stmt_init(con);
    MYSQL_BIND bind[5];

    // reset the binds
    memset(bind, 0, sizeof(bind));
    unsigned long username_length = strlen(username);
    unsigned long skin_length = strlen(skin);
    char * time_string = time_to_string(time);
    unsigned long  time_string_length = strlen(time_string);

    // change the statement based on if another time was found or not
    if (time_found) {
        // a previous time was found
        if (mysql_stmt_prepare(stmt, UPDATE_SCORE, strlen(UPDATE_SCORE))) {
            finish_with_error(con);
        }
    } else {
        // no previous time was found
        if (mysql_stmt_prepare(stmt, INSERT_SCORE, strlen(INSERT_SCORE))) {
            finish_with_error(con);
        }
    }

    // bind the new time(int)
    bind[0].buffer_type = MYSQL_TYPE_LONG;
    bind[0].buffer = (char *)&time;
    bind[0].is_null = 0;
    bind[0].length = 0;

    // bind the new time(string mm:ss.cc)
    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].buffer = time_string;
    bind[1].buffer_length = time_string_length;
    bind[1].is_null = 0;
    bind[1].length = &time_string_length;

    // bind the player's username
    bind[2].buffer_type = MYSQL_TYPE_STRING;
    bind[2].buffer = username;
    bind[2].buffer_length = username_length;
    bind[2].is_null = 0;
    bind[2].length = &username_length;

    // bind the player's current character
    bind[3].buffer_type = MYSQL_TYPE_STRING;
    bind[3].buffer = skin;
    bind[3].buffer_length = skin_length;
    bind[3].is_null = 0;
    bind[3].length = &skin_length;

    // bind the map's number
    bind[4].buffer_type = MYSQL_TYPE_LONG;
    bind[4].buffer = (char *)&mapnum;
    bind[4].is_null = 0;
    bind[4].length = 0;

    // execute the statement
    if (mysql_stmt_bind_param(stmt, bind)) {
        finish_with_error(con);
    }
    if (mysql_stmt_execute(stmt)) {
        finish_with_error(con);
    }

    // close the statement
    if (mysql_stmt_close(stmt)) {
        fprintf(stderr, "%s\n", mysql_error(con));
        finish_with_error(con);
    }

    // deallocate the memory for the time(string mm:ss.cc)
    free(time_string);

}

/*
 * Returns the current best time for the given username
 * 
 * @Returns -1 if no time found
 */
int get_time(MYSQL* con, int mapnum, char* username, char* skin)
{
    // setup the statement handler and statement binds
    MYSQL_STMT* stmt;
    MYSQL_BIND bind[3];

    // initialize the statement handler
    stmt = mysql_stmt_init(con);
    if (mysql_stmt_prepare(stmt, GET_SCORE, strlen(GET_SCORE))) {
        finish_with_error(con);
    }

    // reset the binds
    memset(bind, 0, sizeof(bind));
    unsigned long str_length = strlen(username);
    unsigned long skin_length = strlen(skin);

    // bind the username of the player
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = username;
    bind[0].buffer_length = str_length;
    bind[0].is_null = 0;
    bind[0].length = &str_length;

    // bind the character of the player
    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].buffer = skin;
    bind[1].buffer_length = skin_length;
    bind[1].is_null = 0;
    bind[1].length = &skin_length;

    // bind the mapnum of the finished map
    bind[2].buffer_type = MYSQL_TYPE_LONG;
    bind[2].buffer = (char *)&mapnum;
    bind[2].is_null = 0;
    bind[2].length = 0;

    // insert the binds through the statement
    if (mysql_stmt_bind_param(stmt, bind)) {
        finish_with_error(con);
    }
    if (mysql_stmt_execute(stmt)) {
        finish_with_error(con);
    }

    // get the current saved time
    int time;
    bind[2].buffer = (char *) &time;
    if (mysql_stmt_bind_result(stmt, &bind[2])) {
        finish_with_error(con);
    }

    // get the result of the time
    int res = mysql_stmt_fetch(stmt);
    if(res == 1) {
        finish_with_error(con);
    } else if (res == MYSQL_NO_DATA) {
        //No best time yet
        time = -1;
    }

    // close the statement handler
    if (mysql_stmt_close(stmt)) {
        fprintf(stderr, "%s\n", mysql_error(con));
        finish_with_error(con);
    }

    return time;
}

// Insert the map if it is not yet in the database
void insert_map(MYSQL *con, int mapnum, char *mapname)
{
    char query[QUERY_LEN];

    // setup the results of the tables, the statement handler and the statement bind
    MYSQL_RES *result;
    MYSQL_STMT* stmt;
    MYSQL_BIND bind[2];

    // get the map's row in the table
    snprintf(query, QUERY_LEN, GET_MAP, mapnum);
    if (mysql_query(con, query)) {
        finish_with_error(con);
    }

    // get the result of the sql tables
    result = mysql_store_result(con);
    // if no result was found
    if (result == NULL) 
    {
        finish_with_error(con);
    }

    // if the map doesn't exist in the database
    if (mysql_num_rows(result) == 0) {
        // create the inserter
        stmt = mysql_stmt_init(con);

        // insert the map
        if (mysql_stmt_prepare(stmt, INSERT_MAP, strlen(INSERT_MAP))) {
            finish_with_error(con);
        }

        // reset the binds
        memset(bind, 0, sizeof(bind));
        unsigned long str_length = strlen(mapname);

        // add the mapnum to the binds
        bind[0].buffer_type = MYSQL_TYPE_LONG;
        bind[0].buffer = (char *)&mapnum;
        bind[0].is_null = 0;
        bind[0].length = 0;

        // add the map's name to the binds
        bind[1].buffer_type = MYSQL_TYPE_STRING;
        bind[1].buffer = mapname;
        bind[1].buffer_length = strlen(mapname);
        bind[1].is_null = 0;
        bind[1].length = &str_length;

        // set the binds to the statement handler and executes the statement
        if (mysql_stmt_bind_param(stmt, bind)) {
            finish_with_error(con);
        }
        if (mysql_stmt_execute(stmt)) {
            finish_with_error(con);
        }

        // closes the statement handler
        if (mysql_stmt_close(stmt)) {
            fprintf(stderr, "%s\n", mysql_error(con));
            finish_with_error(con);
        }
    }

    // free the memory allocated for the tables result
    mysql_free_result(result);
}

// Process the time of the player
void process_time(MYSQL *con, int playernum, int mapnum) {
    // get the player's time, username, character
    int time = players[playernum].realtime;
    char* username = player_names[playernum];
    char* skin = ((skin_t *)players[playernum].mo->skin)->name;

    //  get the player's best time
    int res = get_time(con, mapnum, username, skin);
    int best_time;

    // if no time was found
    if (res == -1) {
        // insert the current time
        insert_score(con, mapnum, username, skin, time, false);
    } else {
        best_time = res;
        // if the new time is lower than the best time
        if (time < best_time) {
            printf("Username: %s, map_id: %d, skin: %s, old_time: %d, new_time: %d\r\n", username, mapnum, skin, best_time, time);
            // update the time
            insert_score(con, mapnum, username, skin, time, true);
        }
    }
}

// Called when the race has finished
void speedrun_map_completed()
{
    // create the mysql handler
    MYSQL *con = mysql_init(NULL);

    // get the map's number
    int mapnum = gamemap-1;
    char mapname[30];

    // check for the number of the act and prints the level's name
    if (mapheaderinfo[mapnum]->actnum) {
        snprintf(mapname, 30, "%s %d", mapheaderinfo[mapnum]->lvlttl, mapheaderinfo[mapnum]->actnum);
    } else {
        snprintf(mapname, 30, "%s", mapheaderinfo[mapnum]->lvlttl);
    }

    if (con == NULL) 
    {
        // print the mysql error
        fprintf(stderr, "%s\n", mysql_error(con));
        exit(1);
    }

    // connect to the database
    if (mysql_real_connect(con, "localhost", USERNAME, PASSWORD, 
            DATABASE, 0, NULL, 0) == NULL)
    {
        finish_with_error(con);
    }

    // inserts the new map if not found
    insert_map(con, mapnum, mapname);

    //for every player
    for (int playernum = 0; playernum < MAXPLAYERS; playernum++) {
        // if the player is not in game, is a spectator or is dead: skip it
    	if (!playeringame[playernum] 
        	  || players[playernum].spectator 
        	  || players[playernum].pflags & PF_GAMETYPEOVER 
        	  || players[playernum].laps < (unsigned)cv_numlaps.value)
    	    continue;
    	// else save its time
    	process_time(con, playernum, mapnum);
    }
    
    // closes the connection
    mysql_close(con);
}
