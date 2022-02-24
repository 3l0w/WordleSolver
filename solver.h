#ifndef solver_h
#define solver_h
#include <functional>
#include <map>
#include <string>
#include <vector>

enum Color { green, yellow, grey };

struct Letter {
  char character;
  Color color;
};

using ColorWordle = std::vector<Color>;

std::map<std::string, double> calculateAllProbas(std::vector<std::string> words,
                                                 std::function<void()> update);

std::map<std::string, double> calculateAllProbas(
    std::vector<std::string> words);

ColorWordle getWordleFromColors(std::string colors);

std::vector<std::string> filterValues(std::vector<std::string> values,
                                      std::string word, ColorWordle wordle);

std::string getBestValue(std::map<std::string, double> averages);

#endif
