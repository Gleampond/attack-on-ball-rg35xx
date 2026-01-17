#pragma once

#include <string>

class ScoreStorage {
public:
    explicit ScoreStorage(std::string path);

    int LoadBestScore() const;
    void SaveBestScore(int score) const;

private:
    std::string path_;
};
