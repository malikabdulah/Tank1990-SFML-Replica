#ifndef persistence_h
#define persistence_h

bool save_game();
bool load_game();
void save_high_score(int score);
void load_high_scores(int scores[], int max);

#endif