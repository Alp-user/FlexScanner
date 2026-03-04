#pragma once
#undef yyFlexLexer
#include <FlexLexer.h>
#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>

enum class RubixTag {
  Start,
  // Last was a character
  Character,
  // Last was a Rotation
  Apostrophe,
  // Last was a number
  Twice
};
enum class ChemicalTag {
  // Start slash
  Start,
  Upper,
  Lower,
  Digit,
  // End slash
  End,
};

class MyLexer : public yyFlexLexer {
public:
  void consumeLine();
  void errorHandler();
  void successHandler();
};

bool isValidElement(char x, char y = '0');

struct RubixState {
  RubixTag tag = RubixTag::Start;
};
struct ChemicalState {
  char prev_char = '0';
  size_t length = 0;
  ChemicalTag tag = ChemicalTag::Start;
};

void resetRubixState(RubixState *data);
void transitionRubix(RubixState *data, RubixTag next_tag);
void resetChemicalState(ChemicalState *data);
void transitionChemical(ChemicalState *data, ChemicalTag next_tag,
                        char next_char);

void printChemicalTag(ChemicalTag tag);
