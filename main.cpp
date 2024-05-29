#include "game.h"
#include <iostream>

int main() {
    std::string initialWord;
    int numPlayers;

    std::cout << "Enter the initial 5-letter word: ";
    std::cin >> initialWord;


    do {
        std::cout << "Enter the number of players (2-4): ";
        std::cin >> numPlayers;
    } while (numPlayers < 2 || numPlayers > 4);

    std::string dictionaryFile = "../engwords.txt";  // Убедитесь, что файл находится в правильном месте

    try {
        Game game(initialWord, dictionaryFile, numPlayers);

        while (!game.isGameOver()) {
            game.displayBoard();
            int row, col;
            char letter;
            std::cout << "Player " << game.getCurrentPlayer() + 1 << "'s turn. Enter row, col, and letter (or -1 to skip): ";
            std::cin >> row;
            if (row == -1) {
                game.skipTurn();
                game.displayScores();
                continue;
            }
            std::cin >> col >> letter;
            if (!game.addLetter(row, col, letter)) {
                std::cout << "Invalid move. Try again.\n";
            } else {
                game.displayScores();
            }
        }

        std::cout << "Game over!\n";
        game.displayScores();
        std::cout << initialWord;
    }
    catch (const std::runtime_error& e) {
        std::cerr << "Failed to initialize the game: " << e.what() << std::endl;
    }

    return 0;
}
