#include "speedrun.h"
#include "console.h"
#include "doomdef.h"
#include "g_game.h"
#include "r_things.h"
#include <stdio.h>

void speedrun_end(player_t *player)
{
    FILE * fp;
    fp = fopen("/tmp/srb2_speedrun_time", "w");
    fprintf(fp, 
        "map='%s'\n"
        "skin='%s'\n"
        "time='%i:%02i.%02i'\n"
        , mapheaderinfo[gamemap-1]->lvlttl
        , ((skin_t *)player->mo->skin)->name
        , G_TicsToMinutes(player->realtime,true)
        , G_TicsToSeconds(player->realtime)
        , G_TicsToCentiseconds(player->realtime));
    fclose(fp);
}
