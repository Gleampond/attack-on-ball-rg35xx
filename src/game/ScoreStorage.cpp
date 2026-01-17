#include "ScoreStorage.h"

#include <fstream>

ScoreStorage::ScoreStorage(std::string path) : path_(std::move(path)) {}

int ScoreStorage::LoadBestScore() const {
    std::ifstream file(path_);
    if (!file.is_open()) {
        return 0;
    }
    int score = 0;
    file >> score;
    return score;
}

void ScoreStorage::SaveBestScore(int score) const {
    std::ofstream file(path_, std::ios::trunc);
    if (!file.is_open()) {
        return;
    }
    file << score;
}
