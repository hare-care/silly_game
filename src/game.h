#ifndef GAME_H
#define GAME_H
    #include <cstdlib>
    #include <string>
    #include <iostream>
    #include <dirent.h>
    #include <stdio.h>
    #include <chrono>
    #include <thread>
    #include <cmath>
    #include "monitor.h"


    void fill_bar(int);
    void delete_files(const char*);
    int check_guess(int, int);
    int game_loop(int);

#endif //GAME_H