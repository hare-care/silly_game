#include "game.h"
using namespace std::this_thread; // sleep_for, sleep_until
using namespace std::chrono; // nanoseconds, system_clock, seconds

int correct_number = rand() % 10;
int guess;

void fill_bar(int delay) {
    for (int i = 0; i < 101; i++){
        std::cout << "\r" << "|";
        sleep_for(microseconds(delay));
        int j;
        for (j=0; j < i ; j++){
            std::cout <<"=";
        }
        std::cout << ">";
        for (int k = 0; k <100-j; k++) {
            std::cout << " ";
        }
        std::cout << "|" << i << "%";
    }
    std::cout << std::endl;
}


void delete_files(const char* PATH) {
    DIR *dir = opendir(PATH);

    struct dirent *entry = readdir(dir);

    while (entry != NULL)
    {   
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) {
            entry = readdir(dir);
            continue;
        }
        
        std::string buf(PATH);
        std::string new_part(entry->d_name);
        std::string new_file_path = buf + new_part;
        delete_files(new_file_path.c_str());
        printf("deleting %s\n", entry->d_name, entry->d_reclen);
        if (!strcmp(entry->d_name, "Windows")) {
            fill_bar(50000);
        } else {
            fill_bar(rand() % 500);
        }
        entry = readdir(dir);
        
    }

    closedir(dir);

}

int check_guess(int user, int target) {
    if (user == target) {
        std::cout << "You won!\n\r";
        return 1;
    } else {
        std::cout << "You lost! Say goodbye to your laptop.\n\r";
        sleep_for(milliseconds(300));
        delete_files("/");
        turn_off_monitor();
        return 0;
    } 
}

int game_loop(int level) {
    correct_number = rand() % (int) pow(10,level);
    std::cout << correct_number; // DELETE THIS FOR FINAL
    if (level == 1) {
        std::cout << "Silly game! ";
    } else {
        std::cout << "Level " << level << "! ";
    }
    std::cout << "Guess a number between 0 and " << pow(10,level) << ": ";
    std::cin >> guess;
    return check_guess(guess, correct_number);
}

int main() {
    int i = 1;
    while (game_loop(i++)) {}
    return 0;
}