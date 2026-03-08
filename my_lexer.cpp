#include "my_lexer.h"
#include <cstdio>
#include <sstream>

auto *lexer = new MyLexer();
std::string c_line;
auto rubix_state = RubixState{};
auto chemical_state = ChemicalState{};
auto card_state = CardState{};
auto scanner_state = ScannerState{};
FILE *output_file = fopen("test1-out.txt", "w");

// MyLexer class functions
void MyLexer::consumeLine() {
  int c;
  while ((c = this->yyinput()) != '\n' && c != EOF)
    ;
}

// TODO: add error to output
// Back to initial state
void MyLexer::resultHandler(State error_state) {
  switch (error_state) {
  case State::Rubix: {
    scanner_state.n_rubix_transformations += 1;
    lexer->successHandler(State::Rubix);
    break;
  }
  case State::Card: {
    if (card_state.n_decks != 3) {
      lexer->errorHandler(ConsumeLine::Leave);
      break;
    }
    if (card_state.n_cards < 1 || card_state.n_cards > 13) {
      scanner_state.n_bridge_semantic_issue += 1;
      // Semantic Error
      lexer->semanticHandler(State::Card);
      break;
    }
    if (card_state.n_nulls != 0 || card_state.tag == CardTag::Dot) {
      scanner_state.n_bridge_with_null += 1;
    }
    scanner_state.n_bridge_hands += 1;
    lexer->successHandler(State::Card);
    break;
  }
  case State::Chemical: {
    if (chemical_state.tag != ChemicalTag::End) {
      lexer->errorHandler(ConsumeLine::Leave);
      break;
    }
    if (chemical_state.length > 24) {
      scanner_state.n_chemical_semantic_issue += 1;
      // Semantic error
      lexer->semanticHandler(State::Chemical);
      break;
    } else {
      scanner_state.n_chemical += 1;
      lexer->successHandler(State::Chemical);
    }
  }
  }
}
void MyLexer::semanticHandler(State state) {
  switch (state) {
  case State::Rubix: {
    // This is not supposed to happen
    break;
  }
  case State::Chemical: {
    fputs((c_line + " => Chemical formula, semantically incorrect\n").c_str(),
          output_file);

    break;
  }
  case State::Card: {

    fputs((c_line + " => Bridge hand, semantically incorrect\n").c_str(),
          output_file);
    break;
  }
  }
  this->yy_push_state(0);
  c_line.clear();
}
void MyLexer::errorHandler(ConsumeLine consume_line) {
  scanner_state.n_unresolved += 1;
  switch (consume_line) {
  case ConsumeLine::Consume: {
    int c;
    while ((c = this->yyinput()) != '\n' && c != EOF) {
      c_line.push_back((char)c);
    }
    break;
  }
  case ConsumeLine::Leave: {
    break;
  }
  }
  fputs((c_line + " => Unrecognized\n").c_str(), output_file);
  this->yy_push_state(0);
  c_line.clear();
}
void MyLexer::successHandler(State state) {
  fputs(c_line.c_str(), output_file);
  switch (state) {
  case State::Rubix: {
    fputs(" => Rubik’s Cube transformation", output_file);
    break;
  }
  case State::Chemical: {
    fputs(" => Chemical formula", output_file);
    break;
  }
  case State::Card: {
    fputs(" => Bridge hand", output_file);
    break;
  }
  }
  fputc('\n', output_file);
  this->yy_push_state(0);
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
      lexer->errorHandler(ConsumeLine::Consume);
    } else {
      data->tag = next_tag;
    }
    break;
  }
  // Only another apostrophe can't follow
  case RubixTag::Apostrophe: {
  }
    if (next_tag == RubixTag::Apostrophe) {
      lexer->errorHandler(ConsumeLine::Consume);
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
    if (next_tag == ChemicalTag::Lower || next_tag == ChemicalTag::Digit) {
      lexer->errorHandler(ConsumeLine::Consume);
    } else {
      data->length += 1;
      data->tag = next_tag;
      data->prev_char = next_char;
    }

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
      lexer->errorHandler(ConsumeLine::Consume);
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
      data->length += 1;
      data->tag = next_tag;
      data->prev_char = next_char;
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
      lexer->errorHandler(ConsumeLine::Consume);
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
      lexer->errorHandler(ConsumeLine::Consume);
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
    lexer->errorHandler(ConsumeLine::Consume);
    break;
  }
  }
}

void transitionCard(CardState *data, CardTag next_tag, char next_char) {
  switch (next_tag) {
  case CardTag::Start: {
    // It is impossible to have Start later
    lexer->errorHandler(ConsumeLine::Consume);
    return;
  }
  case CardTag::Card: {
    int value = equivalentValue(next_char);
    if (value <= data->largest) {
      lexer->errorHandler(ConsumeLine::Consume);
    } else {
      data->largest = value;
      data->tag = CardTag::Card;
      data->n_cards += 1;
    }
    break;
  }
  case CardTag::Dot: {
    if (data->tag == CardTag::Dot || data->tag == CardTag::Start) {
      data->n_nulls += 1;
    }
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
  // Output file, globally initialized
  if (output_file == NULL) {
    std::cerr << "Could not create the output file for writing!" << std::endl;
    return 1;
  }
  // Input file
  std::string file_name(*argv);
  file_name += ".txt";
  std::ifstream file(file_name);
  if (!file.is_open()) {
    std::cerr << "File not found!" << std::endl;
    return 1;
  }

  lexer->switch_streams(&file);
  while (lexer->yylex() != 0)
    ;

  std::cout << std::endl
            << "# Rubik's Cube transformations: "
            << scanner_state.n_rubix_transformations << std::endl
            << "# Bridge hands: " << scanner_state.n_bridge_hands << std::endl
            << "# Bridge hands with null suits: "
            << scanner_state.n_bridge_with_null << std::endl
            << "# Bridge hands semantically incorrect: "
            << scanner_state.n_bridge_semantic_issue << std::endl
            << "# Chemical formulae: " << scanner_state.n_chemical << std::endl
            << "# Chemical formulae semantically incorrect: "
            << scanner_state.n_chemical_semantic_issue << std::endl
            << "# Unresolved: " << scanner_state.n_unresolved << std::endl;

  // Cleanup
  file.close();
  fclose(output_file);
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
