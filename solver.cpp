#include "solver.h"

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <algorithm>
#include <deque>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

using Wordle = std::vector<Letter>;

bool resultContains(Wordle result, char r, int number) {
  int count = 0;
  for (Letter letter : result) {
    if (letter.character == r && letter.color != Color::grey) {
      count++;
    }
  }

  return number == count;
}

int countChar(std::string str, char c) {
  int number = 0;
  for (char c1 : str) {
    if (c1 == c) number++;
  }

  return number;
}

Wordle wordle(std::string answer, std::string test) {
  Wordle result;

  for (int i = 0; i < test.length(); i++) {
    result.push_back(Letter{test[i], Color::grey});
  }

  for (int i = 0; i < test.length(); i++) {
    if (answer[i] == test[i]) result[i] = Letter{test[i], Color::green};
  }

  for (int i = 0; i < test.length(); i++) {
    if (answer.find(test[i]) != std::string::npos && test[i] != answer[i] &&
        !resultContains(result, test[i], countChar(answer, test[i])))
      result[i] = Letter{test[i], Color::yellow};
  }

  return result;
}

int getPosition(std::string str, char character,
                std::vector<int> alreadyCheckedPosition) {
  /*std::cout << "checked: ";
  for (int v : alreadyCheckedPosition) std::cout << v << " ";
  std::cout << std::endl;*/

  for (int i = 0; i < str.size(); i++) {
    if (str[i] == character &&
        std::find(alreadyCheckedPosition.begin(), alreadyCheckedPosition.end(),
                  i) == alreadyCheckedPosition.end())
      return i;
  }

  return -1;
}
bool canBeValid(ColorWordle wordle, std::string origin, std::string test) {
  std::vector<int> alreadyChecked;
  for (int i = 0; i < wordle.size(); i++) {
    if (wordle[i] == Color::green) {
      if (origin[i] == test[i])
        alreadyChecked.push_back(i);
      else
        return false;
    }
  }

  for (int i = 0; i < wordle.size(); i++) {
    if (wordle[i] == Color::yellow) {
      int position = getPosition(test, origin[i], alreadyChecked);
      if (position != -1 && origin[i] != test[i])
        alreadyChecked.push_back(position);
      else
        return false;
    }
  }

  for (int i = 0; i < wordle.size(); i++) {
    if (wordle[i] == Color::grey &&
        getPosition(test, origin[i], alreadyChecked) != -1)
      return false;
  }

  return true;
}

bool canBeValid(Wordle wordle, std::string word) {
  for (int i = 0; i < wordle.size(); i++) {
    Letter letter = wordle[i];
    if (letter.color == Color::green && wordle[i].character != word[i])
      return false;
    else if (letter.color == Color::yellow && wordle[i].character == word[i] &&
             word.find(letter.character) == std::string::npos)
      return false;
    else if (letter.color == Color::grey &&
             word.find(letter.character) != std::string::npos)
      return false;
  }
  return true;
}

double safeLog2(double x) { return x > 0 ? log2(x) : 0; }

std::vector<ColorWordle> getAllPossibleWordle() {
  std::vector<ColorWordle> all;

  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      for (int k = 0; k < 3; k++) {
        for (int l = 0; l < 3; l++) {
          for (int m = 0; m < 3; m++) {
            for (int n = 0; n < 3; n++) {
              ColorWordle wordle = {
                  static_cast<Color>(i), static_cast<Color>(j),
                  static_cast<Color>(k), static_cast<Color>(l),
                  static_cast<Color>(m), static_cast<Color>(n)};
              all.push_back(wordle);
            }
          }
        }
      }
    }
  }
  return all;
}
std::vector<std::string> filterValues(std::vector<std::string> v,
                                      std::string test, ColorWordle wordle) {
  std::vector<std::string> possibleValues;

  for (std::string str : v) {
    if (canBeValid(wordle, test, str)) {
      possibleValues.push_back(str);
    }
  }
  return possibleValues;
}

double getEntropy(std::vector<std::string> v, std::string test,
                  ColorWordle wordle) {
  return -safeLog2(filterValues(v, test, wordle).size() / (double)v.size());
}

std::vector<ColorWordle> allWordle = getAllPossibleWordle();
double treatAnswer(std::vector<std::string> v, std::string test) {
  double probaSums = 0;

  for (ColorWordle wordle : allWordle) {
    probaSums += getEntropy(v, test, wordle);
  }

  return probaSums / (double)allWordle.size();
}

std::mutex mtx;
void treatQueue(std::vector<std::string> v, std::deque<std::string> *q,
                std::map<std::string, double> *averages,
                std::function<void()> lamda) {
  while (!q->empty()) {
    auto val = q->back();
    mtx.lock();
    q->pop_back();
    mtx.unlock();
    averages->operator[](val) = treatAnswer(v, val);
    lamda();
  }
}

std::map<std::string, double> calculateAllProbas(std::vector<std::string> v) {
  return calculateAllProbas(v, []() -> void {});
}

std::map<std::string, double> calculateAllProbas(std::vector<std::string> v,
                                                 std::function<void()> lamda) {
  std::deque<std::string> q;
  for (std::string str : v) {
    q.push_back(str);
  }

  std::vector<std::thread> threads;
  std::map<std::string, double> averages;

  for (int i = 0; i <= std::thread::hardware_concurrency(); i++) {
    threads.push_back(std::thread(treatQueue, v, &q, &averages, lamda));
  }

  for (int i = 0; i < threads.size(); i++) {
    threads[i].join();
  }
  return averages;
}

ColorWordle getWordleFromColors(std::string str) {
  ColorWordle wordle;
  for (char c : str) {
    switch (c) {
      case 'g':
        wordle.push_back(Color::green);
        break;
      case 'y':
        wordle.push_back(Color::yellow);
        break;
      case 'b':
        wordle.push_back(Color::grey);
        break;
    }
  }
  return wordle;
}

std::string getBestValue(std::map<std::string, double> averages) {
  std::map<std::string, double>::iterator it = averages.begin();

  std::string name;
  double max = 0;

  while (it != averages.end()) {
    if (max < it->second) {
      name = it->first;
      max = it->second;
    }
    it++;
  }

  return name;
}

