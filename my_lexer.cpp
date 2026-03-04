#include "my_lexer.h"
#include <sstream>

auto *lexer = new MyLexer();
std::string c_line;
auto rubix_state = RubixState{};
auto chemical_state = ChemicalState{};
auto card_state = CardState{};

// MyLexer class functions
void MyLexer::consumeLine() {
  int c;
  while ((c = this->yyinput()) != '\n' && c != EOF)
    ;
}
void MyLexer::errorHandler() {
  int c;
  std::cerr << "An error occurred!";
  while ((c = this->yyinput()) != '\n' && c != EOF) {
    c_line.push_back((char)c);
  }
  std::cerr << "The line causing the error: " << c_line << std::endl;
  // TODO: add error to output
  // Back to initial state
  this->yy_push_state(0);
  c_line.clear();
}
void MyLexer::successHandler() {
  std::cout << "You did a great job!" << std::endl;
  c_line.clear();
}

// Transition and reset functions
void resetRubixState(RubixState *data) { *data = RubixState{}; }

void resetChemicalState(ChemicalState *data) { *data = ChemicalState{}; }

void resetCardState(CardState *data) { *data = CardState{}; }

void transitionRubix(RubixState *data, RubixTag next_tag) {
  switch (data->tag) {
  // Anything can follow
  case RubixTag::Start: {
    data->tag = next_tag;
    break;
  }
  // Anything may come after a character
  case RubixTag::Character: {
    data->tag = next_tag;
    break;
  }
  // Only a character can follow and nothing else
  case RubixTag::Twice: {
    if (next_tag != RubixTag::Character) {
      lexer->errorHandler();
    } else {
      data->tag = next_tag;
    }
    break;
  }
  // Only another apostrophe can't follow
  case RubixTag::Apostrophe: {
  }
    if (next_tag == RubixTag::Apostrophe) {
      lexer->errorHandler();
    } else {
      data->tag = next_tag;
    }
    break;
  }
}

void transitionChemical(ChemicalState *data, ChemicalTag next_tag,
                        char next_char) {
  switch (data->tag) {
  // This is not the first slash, it is the first character
  case ChemicalTag::Start: {
    data->length += 1;
    data->tag = next_tag;
    data->prev_char = next_char;
    break;
  }
  case ChemicalTag::Upper: {
    switch (next_tag) {
      // Another upper, need to check the validity of last element
    case ChemicalTag::Upper: {
      data->prev_char = next_char;
      data->tag = next_tag;
      data->length += 1;
      break;
    }
      // Start can't happen in the middle
    case ChemicalTag::Start: {
      lexer->errorHandler();
      break;
    }
      // This is a two letter elements so we can check
    case ChemicalTag::Lower: {
      data->prev_char = next_char;
      data->tag = next_tag;
      data->length += 1;

      break;
    }
      // Must check the validity of last upper case character
    case ChemicalTag::End: {
      data->prev_char = next_char;
      data->tag = next_tag;
      break;
    }
      // This is not a problem at all
    case ChemicalTag::Digit: {
      data->prev_char = next_char;
      data->tag = next_tag;
      data->length += 1;
      break;
    }
    }
    break;
  }
  // All validity checks are done in Upper case, none needed here
  case ChemicalTag::Lower: {
    switch (next_tag) {
    case ChemicalTag::Start:
    case ChemicalTag::Lower: {
      lexer->errorHandler();
      break;
    }
    // Increase length by 1 for Digit and Upper but not for End
    case ChemicalTag::Digit:
    case ChemicalTag::Upper: {
      data->length += 1;
    }
    case ChemicalTag::End: {
      data->tag = next_tag;
      data->prev_char = next_char;
      break;
    }
    }
    break;
  }
  // Next can be either an upper case or end
  case ChemicalTag::Digit: {
    switch (next_tag) {
    case ChemicalTag::Start:
    case ChemicalTag::Digit:
    case ChemicalTag::Lower: {
      lexer->errorHandler();
      break;
    }
    // Increase length by 1 for Upper but not for End
    case ChemicalTag::Upper: {
      data->length += 1;
    }
    case ChemicalTag::End: {
      data->tag = next_tag;
      data->prev_char = next_char;
      break;
    }
    }
    break;
  }
  case ChemicalTag::End: {
    // Nothing should come after the end
    lexer->errorHandler();
    break;
  }
  }
  if (data->length > 24) {
    lexer->errorHandler();
  }
}

void transitionCard(CardState *data, CardTag next_tag, char next_char) {
  switch (next_tag) {
  case CardTag::Start: {
    // It is impossible to have Start later
    lexer->errorHandler();
    return;
  }
  case CardTag::Card: {
    int value = equivalentValue(next_char);
    if (value <= data->largest) {
      lexer->errorHandler();
    } else {
      data->largest = value;
      data->tag = CardTag::Card;
      data->n_cards += 1;
    }
    break;
  }
  case CardTag::Dot: {
    resetCardLargest(data);
    data->tag = CardTag::Dot;
    data->n_decks += 1;
    break;
  }
  }
}

int main(int argc, char **argv) {
  // Skip the program name
  argv++;
  argc--;
  std::ifstream file(*argv);
  if (!file.is_open()) {
    std::cerr << "File not found!" << std::endl;
    return 1;
  }

  lexer->switch_streams(&file);
  while (lexer->yylex() != 0)
    ;

  // Cleanup
  file.close();
  delete lexer;
  return 0;
}

// Helper function to check the validity of the elements

void printChemicalTag(ChemicalTag tag) {
  switch (tag) {
  case ChemicalTag::Start:
    std::cout << "Start";
    break;
  case ChemicalTag::Upper:
    std::cout << "Upper";
    break;
  case ChemicalTag::Lower:
    std::cout << "Lower";
    break;
  case ChemicalTag::Digit:
    std::cout << "Digit";
    break;
  case ChemicalTag::End:
    std::cout << "End";
    break;
  default:
    std::cout << "Unknown";
    break;
  }
}

int equivalentValue(char c) {
  switch (c) {
  case 'A': {
    return 13;
  }
  case 'K': {
    return 12;
  }
  case 'Q': {
    return 11;
  }
  case 'J': {
    return 10;
  }
  case '9': {
    return 8;
  }
  case '8': {
    return 7;
  }
  case '7': {
    return 6;
  }
  case '6': {
    return 5;
  }
  case '5': {
    return 4;
  }
  case '4': {
    return 3;
  }
  case '3': {
    return 2;
  }
  case '2': {
    return 1;
  }
  }
  return -1;
}

void resetCardLargest(CardState *data) { data->largest = 0; }
