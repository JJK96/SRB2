# SRB2 highscore tracker

On finishing a map the score is saved in the database.
The best score per username, skin and map is kept.

This data can be visualized using https://github.com/jjk96/srb2_highscores

## How to run

1. Create a mysql database
2. Apply the files in `sql/` to the database in the order of creation.
3. Create `src/credentials.h` with contents:
```
#define USERNAME "<username>" 
#define PASSWORD "<password> 
#define DATABASE "<database>"
```
4. Compile and run
