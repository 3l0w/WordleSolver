# WordleSolver
## Goal

The main goal of this project is to challenge me implement a algorithm who solves the [Wordle](https://www.nytimes.com/games/wordle/index.html) and also to start learning c++

The idea came when watching a video of [3Blue1Brown](https://www.youtube.com/c/3blue1brown) where he explain how this game was solvable with maths and algorithm [link](https://www.youtube.com/watch?v=v68zYyaEmEA)

## Prerequisite

You need to have gcc and CMake installed on your machine

Ubuntu and Debian:
```bash
sudo apt install build-essential cmake
```

ArchLinux:
```bash
sudo pacman -Sy gcc cmake
```

For other distro idk but it you know let me know by oppening a PR or a issue

## Build It

Create a build folder
```bash
mkdir buikd
```
Then call the CMake CLI from this folder pointing to the project folder
```bash
cd build
cmake ..
```

For lazy people, here are the one line cmd
```bash
mkdir build && cd build && cmake ..
```

## Usage
First you need to provide a file with the list of words that the algorithm will use, that file need to be named **words.txt** and place in the active directory
I provide in the project the words.txt of the classic Wordle

Then execute the binary
```
./WordleSolver
```
The first run it will generate a file named results.txt that file is used to compute the first value, same for words.txt i've provided in the root of the project a file that i've generate myself

Then the algo can be used

It will first prompt you the words you should use

Then it will ask the words you've used (or if blank the one he predicted) and after the color displayed by that word:

You need to type a series of letter (b, y or g) for representing the color: b for a letter that is not in the word, y for a letter that is misplaced and g for a letter that is correctly placed

And after that it will compute the possible words, propose you a word and it goes back to the beginning

## Contribution
If you know some improvement i can made let me know by opening a issue or a PR

## Screenshot

![Screenshot](./assets/screenshot.jpg?raw=true)
