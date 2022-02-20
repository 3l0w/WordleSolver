#include <math.h>
#include <replxx.h>
#include <stdio.h>
#include <string.h>

#include <deque>
#include <fstream>
#include <indicators/block_progress_bar.hpp>
#include <indicators/cursor_control.hpp>
#include <iostream>
#include <map>
#include <string>
#include <termcolor/termcolor.hpp>
#include <vector>

enum Color { green, yellow, grey };

struct Letter {
  char character;
  Color color;
};

using Wordle = std::vector<Letter>;
using ColorWordle = std::vector<Color>;

bool resultContains(Wordle result, char r) { return true; }

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

void displayWordle(ColorWordle wordle, std::string str) {
  for (int i = 0; i < str.size(); i++) {
    if (wordle[i] == Color::green)
      std::cout << termcolor::green << termcolor::bold << str[i];

    if (wordle[i] == Color::yellow)
      std::cout << termcolor::yellow << termcolor::bold << str[i];

    if (wordle[i] == Color::grey)
      std::cout << termcolor::grey << termcolor::bold << str[i];
  }
  std::cout << termcolor::reset << std::endl;
}

void displayWordle(Wordle result) {
  for (Letter letter : result) {
    if (letter.color == Color::green)
      std::cout << termcolor::green << termcolor::bold << letter.character;

    if (letter.color == Color::yellow)
      std::cout << termcolor::yellow << termcolor::bold << letter.character;

    if (letter.color == Color::grey)
      std::cout << termcolor::grey << termcolor::bold << letter.character;
  }
  std::cout << termcolor::reset << std::endl;
}

bool canBeValid(ColorWordle wordle, std::string origin, std::string test) {
  for (int i = 0; i < wordle.size(); i++) {
    char letter = origin[i];
    Color color = wordle[i];
    if (color == Color::green && letter != test[i])
      return false;
    else if (color == Color::yellow &&
             (origin[i] == test[i] || test.find(letter) == std::string::npos))
      return false;
    else if (color == Color::grey && test.find(letter) != std::string::npos)
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

int WriteFile(std::string fname, std::map<std::string, double> *m) {
  int count = 0;
  if (m->empty()) return 0;

  FILE *fp = fopen(fname.c_str(), "w");
  if (!fp) return -errno;

  for (std::map<std::string, double>::iterator it = m->begin(); it != m->end();
       it++) {
    fprintf(fp, "%s=%f\n", it->first.c_str(), it->second);
    count++;
  }

  fclose(fp);
  return count;
}

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

std::map<std::string, double> averages;
std::mutex mtx;
void treatQueue(std::vector<std::string> v, std::deque<std::string> *q,
                indicators::BlockProgressBar *bar) {
  while (!q->empty()) {
    auto val = q->back();
    mtx.lock();
    q->pop_back();
    mtx.unlock();
    averages[val] = treatAnswer(v, val);
    bar->tick();
  }
}

int ReadFile(std::string fname, std::map<std::string, double> *m) {
  int count = 0;
  if (access(fname.c_str(), R_OK) < 0) return -errno;

  FILE *fp = fopen(fname.c_str(), "r");
  if (!fp) return -errno;

  m->clear();

  char *buf = 0;
  size_t buflen = 0;

  while (getline(&buf, &buflen, fp) > 0) {
    char *nl = strchr(buf, '\n');
    if (nl == NULL) continue;
    *nl = 0;

    char *sep = strchr(buf, '=');
    if (sep == NULL) continue;
    *sep = 0;
    sep++;

    std::string s1 = buf;
    std::string s2 = sep;

    (*m)[s1] = std::stod(sep);

    count++;
  }

  if (buf) free(buf);

  fclose(fp);
  return count;
}

void calculateAllProbas(std::vector<std::string> v, bool write) {
  averages.clear();
  std::deque<std::string> q;
  for (std::string str : v) {
    q.push_back(str);
  }

  indicators::BlockProgressBar bar{
      indicators::option::ForegroundColor{indicators::Color::white},
      indicators::option::ShowPercentage{true},
      indicators::option::ShowRemainingTime{true},
      indicators::option::ShowElapsedTime{true},
      indicators::option::BarWidth{75},
      indicators::option::FontStyles{
          std::vector<indicators::FontStyle>{indicators::FontStyle::bold}},
      indicators::option::MaxProgress{v.size()},
  };
  std::vector<std::thread> threads;

  for (int i = 0; i <= std::thread::hardware_concurrency(); i++) {
    threads.push_back(std::thread(treatQueue, v, &q, &bar));
  }

  for (int i = 0; i < threads.size(); i++) {
    threads[i].join();
  }
  bar.mark_as_completed();
  if (write) WriteFile("result.txt", &averages);
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

std::string getBestValue() {
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

int main() {
  std::vector<std::string> v;
  std::string line;
  std::ifstream fin("words.txt");
  while (getline(fin, line)) {
    v.push_back(line);
  }

  if (ReadFile("result.txt", &averages) == -errno) calculateAllProbas(v, true);

  std::cout << "You should start with: " << getBestValue() << std::endl;

  std::vector<std::string> values = v;

  auto replxxInstance = replxx_init();
  while (!values.empty()) {
    std::string str = replxx_input(replxxInstance, "Enter the word you type: ");
    if (str == "") str = getBestValue();
    std::string colors = replxx_input(replxxInstance, "Enter the color: ");
    ColorWordle wordle = getWordleFromColors(colors);
    int size = values.size();
    values = filterValues(values, str, wordle);
    if (values.size() == 1) {
      std::cout << "The solution is: " << values[0] << std::endl;
      exit(0);
    }
    calculateAllProbas(values, false);

    std::cout << "Possible words: ";
    for (std::string str : values) {
      std::cout << str << " ";
    }
    std::cout << std::endl;

    std::cout << "You should use: " << getBestValue() << std::endl;
  }
  return 0;
}

