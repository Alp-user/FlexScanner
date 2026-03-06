#pragma once
#undef yyFlexLexer
#include <FlexLexer.h>
#include <cassert>
#include <cstdio>
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

enum class CardTag {
  Start,
  Dot,
  Card,
};

enum class State {
  Rubix,
  Chemical,
  Card,
};
enum class ConsumeLine {
  Consume,
  Leave,
};

class MyLexer : public yyFlexLexer {
public:
  void consumeLine();
  void errorHandler(ConsumeLine consume_line);
  void semanticHandler(State state);
  void successHandler(State state);
  void resultHandler(State error_state);
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
struct CardState {
  size_t n_cards = 0;
  size_t n_decks = 0;
  size_t n_nulls = 0;
  int largest = 0;
  CardTag tag = CardTag::Start;
};
struct ScannerState {
  size_t n_rubix_transformations = 0; //
  size_t n_bridge_hands = 0;          //
  size_t n_bridge_with_null = 0;      //
  size_t n_bridge_semantic_issue = 0;
  size_t n_chemical = 0;                //
  size_t n_chemical_semantic_issue = 0; //
  size_t n_unresolved = 0;
};

void resetRubixState(RubixState *data);
void transitionRubix(RubixState *data, RubixTag next_tag);
void resetChemicalState(ChemicalState *data);
void transitionChemical(ChemicalState *data, ChemicalTag next_tag,
                        char next_char);
void resetCardState(CardState *data);
void transitionCard(CardState *data, CardTag next_tag, char next_char);

void printChemicalTag(ChemicalTag tag);
int equivalentValue(char c);
void resetCardLargest(CardState *data);
