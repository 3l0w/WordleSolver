#include <replxx.h>
#include <stdio.h>
#include <string.h>

#include <fstream>
#include <functional>
#include <indicators/block_progress_bar.hpp>
#include <indicators/cursor_control.hpp>
#include <termcolor/termcolor.hpp>

#include "solver.h"

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

std::map<std::string, double> runCalculateAllProbas(
    std::vector<std::string> v) {
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
  std::function<void()> update = [&bar]() -> void { bar.tick(); };

  auto averages = calculateAllProbas(v, update);
  bar.mark_as_completed();

  return averages;
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

int main() {
  std::vector<std::string> v;
  std::string line;
  std::ifstream fin("words.txt");
  while (getline(fin, line)) {
    v.push_back(line);
  }

  std::map<std::string, double> averages;

  if (ReadFile("result.txt", &averages) == -errno) {
    averages = runCalculateAllProbas(v);

    WriteFile("result.txt", &averages);
  }

  std::cout << "You should start with: " << getBestValue(averages) << std::endl;

  std::vector<std::string> values = v;

  auto replxxInstance = replxx_init();
  while (!values.empty()) {
    std::string str = replxx_input(replxxInstance, "Enter the word you type: ");
    if (str == "") str = getBestValue(averages);
    std::string colors = replxx_input(replxxInstance, "Enter the color: ");
    ColorWordle wordle = getWordleFromColors(colors);
    int size = values.size();
    values = filterValues(values, str, wordle);
    if (values.size() == 1) {
      std::cout << "The solution is: " << values[0] << std::endl;
      exit(0);
    }
    averages = runCalculateAllProbas(values);

    std::cout << "Possible words: ";
    for (std::string str : values) {
      std::cout << str << " ";
    }
    std::cout << std::endl;

    std::cout << "You should use: " << getBestValue(averages) << std::endl;
  }
}
