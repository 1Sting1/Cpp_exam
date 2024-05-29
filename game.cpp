#include "game.h"
#include <iostream>
#include <stdexcept>
#include <algorithm>

Game::Game(const std::string& initialWord, const std::string& dictionaryFile, int numPlayers)
        : currentPlayer(0), skipsInRow(0), numPlayers(numPlayers), scores(numPlayers, 0) {
    // Инициализация игровой доски
    board = std::vector<std::vector<char>>(5, std::vector<char>(5, ' '));
    int middleRow = 2;
    for (int i = 0; i < 5; ++i) {
        board[middleRow][i] = initialWord[i];
    }
    usedWords.push_back(initialWord);

    // Загрузка словаря
    loadDictionary(dictionaryFile);

    // Инициализация лог-файла
    initLogFile();
}

Game::~Game() {
    if (logFile.is_open()) {
        logFile.close();
    }
}

void Game::initLogFile() {
    logFile.open("game_log.txt");
    if (!logFile) {
        throw std::runtime_error("Unable to open log file.");
    }
}

void Game::displayBoard() const {
    for (const auto& row : board) {
        for (char cell : row) {
            std::cout << (cell == ' ' ? '.' : cell) << ' ';
        }
        std::cout << std::endl;
    }
    if (logFile.is_open()) {
        logFile << "Board state:\n";
        for (const auto& row : board) {
            for (char cell : row) {
                logFile << (cell == ' ' ? '.' : cell) << ' ';
            }
            logFile << '\n';
        }
    }
}

void Game::displayScores() const {
    for (int i = 0; i < numPlayers; ++i) {
        std::cout << "Player " << i + 1 << " score: " << scores[i] << std::endl;
        if (logFile.is_open()) {
            logFile << "Player " << i + 1 << " score: " << scores[i] << '\n';
        }
    }
}

bool Game::addLetter(int row, int col, char letter) {
    if (isValidPlacement(row, col, letter)) {
        board[row][col] = letter;
        logFile << "Player " << currentPlayer + 1 << " added letter '" << letter << "' at (" << row << ", " << col << ")\n";
        auto words = findWords();
        for (const auto& word : words) {
            if (isValidWord(word) && std::find(usedWords.begin(), usedWords.end(), word) == usedWords.end()) {
                usedWords.push_back(word);
                // Начисляем очки игроку в зависимости от длины слова
                int wordScore = word.length();
                scores[currentPlayer] += wordScore;
                logFile << "Found valid word: " << word << " - Player " << currentPlayer + 1 << " earns " << wordScore << " points\n";
            }
        }
        skipsInRow = 0;
        nextTurn();
        return true;
    }
    logFile << "Invalid placement by Player " << currentPlayer + 1 << " at (" << row << ", " << col << ")\n";
    return false;
}

bool Game::isValidWord(const std::string& word) const {
    return dictionary.find(word) != dictionary.end();
}

bool Game::isGameOver() const {
    // Проверить, заполнена ли доска или слишком много пропусков
    for (const auto& row : board) {
        for (char cell : row) {
            if (cell == ' ') {
                return false;
            }
        }
    }
    if (logFile.is_open()) {
        logFile << "Game over because all cells are filled.\n";
    }
    if (skipsInRow >= numPlayers) {
        if (logFile.is_open()) {
            logFile << "Game over because too many skips.\n";
        }
        return true;
    }
    return false;
}

int Game::getScore(int player) const {
    return scores[player];
}

int Game::getCurrentPlayer() const {
    return currentPlayer;
}

void Game::setScore(int player, int score) {
    scores[player] = score;
}

void Game::nextTurn() {
    currentPlayer = (currentPlayer + 1) % numPlayers;
}

void Game::skipTurn() {
    skipsInRow++;
    nextTurn();
}

bool Game::isValidPlacement(int row, int col, char letter) const {
    // Проверить, находится ли клетка в пределах доски и пуста ли она
    if (row < 0 || row >= 5 || col < 0 || col >= 5 || board[row][col] != ' ') {
        return false;
    }
    // Дополнительная проверка на допустимость размещения буквы
    // Например, чтобы клетка была смежна с другой буквой
    return isAdjacent(row, col);
}

bool Game::isAdjacent(int row, int col) const {
    // Проверить, есть ли рядом с клеткой другие буквы
    static const std::vector<std::pair<int, int>> directions = {
            {-1, 0}, {1, 0}, {0, -1}, {0, 1}
    };
    for (const auto& dir : directions) {
        int newRow = row + dir.first;
        int newCol = col + dir.second;
        if (newRow >= 0 && newRow < 5 && newCol >= 0 && newCol < 5 && board[newRow][newCol] != ' ') {
            return true;
        }
    }
    return false;
}

std::vector<std::string> Game::findWords() const {
    std::vector<std::string> foundWords;

    // Поиск горизонтальных слов
    for (const auto& row : board) {
        std::string word;
        for (char c : row) {
            if (c != ' ') {
                word += c;
            } else {
                if (!word.empty() && isValidWord(word) && std::find(usedWords.begin(), usedWords.end(), word) == usedWords.end()) {
                    foundWords.push_back(word);
                }
                word.clear();
            }
        }
        if (!word.empty() && isValidWord(word) && std::find(usedWords.begin(), usedWords.end(), word) == usedWords.end()) {
            foundWords.push_back(word);
        }
    }

    // Поиск вертикальных слов
    for (int col = 0; col < 5; ++col) {
        std::string word;
        for (int row = 0; row < 5; ++row) {
            if (board[row][col] != ' ') {
                word += board[row][col];
            } else {
                if (!word.empty() && isValidWord(word) && std::find(usedWords.begin(), usedWords.end(), word) == usedWords.end()) {
                    foundWords.push_back(word);
                }
                word.clear();
            }
        }
        if (!word.empty() && isValidWord(word) && std::find(usedWords.begin(), usedWords.end(), word) == usedWords.end()) {
            foundWords.push_back(word);
        }
    }

    return foundWords;
}

void Game::loadDictionary(const std::string& dictionaryFile) {
    std::ifstream file(dictionaryFile);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open dictionary file.");
    }
    std::string word;
    while (file >> word) {
        dictionary.insert(word);
    }
}
