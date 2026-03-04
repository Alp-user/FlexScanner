#include "my_lexer.h"
#include <sstream>

auto *lexer = new MyLexer();
std::string c_line;
auto rubix_state = RubixState{};
auto chemical_state = ChemicalState{};

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
      if (isValidElement(data->prev_char)) {
        data->prev_char = next_char;
        data->tag = next_tag;
        data->length += 1;
      } else {
        lexer->errorHandler();
      }
      break;
    }
      // Start can't happen in the middle
    case ChemicalTag::Start: {
      lexer->errorHandler();
      break;
    }
      // This is a two letter elements so we can check
    case ChemicalTag::Lower: {
      if (isValidElement(data->prev_char, next_char)) {
        data->prev_char = next_char;
        data->tag = next_tag;
        data->length += 1;
      } else {
        lexer->errorHandler();
      }

      break;
    }
      // Must check the validity of last upper case character
    case ChemicalTag::End: {
      if (isValidElement(data->prev_char)) {
        data->prev_char = next_char;
        data->tag = next_tag;
        // Don't increase length at slash
      } else {
        lexer->errorHandler();
      }
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
bool isValidElement(char x, char y) {
  switch (x) {
  case 'A': {
    switch (y) {
    case 'c':
    case 'g':
    case 'l':
    case 'm':
    case 'r':
    case 's':
    case 't':
    case 'u':
      return true;
    default:
      return false;
    }
  }
  case 'B': {
    switch (y) {
    case 'a':
    case 'e':
    case 'h':
    case 'i':
    case 'k':
    case 'r':
      return true;
    default:
      return y == '0';
    }
  }
  case 'C': {
    switch (y) {
    case 'a':
    case 'd':
    case 'e':
    case 'f':
    case 'l':
    case 'm':
    case 'n':
    case 'o':
    case 'r':
    case 's':
    case 'u':
      return true;
    default:
      return y == '0';
    }
  }
  case 'D': {
    switch (y) {
    case 'b':
    case 's':
    case 'y':
      return true;
    default:
      return false;
    }
  }
  case 'E': {
    switch (y) {
    case 'r':
    case 's':
    case 'u':
      return true;
    default:
      return false;
    }
  }
  case 'F': {
    switch (y) {
    case 'e':
    case 'l':
    case 'm':
    case 'r':
      return true;
    default:
      return y == '0';
    }
  }
  case 'G': {
    switch (y) {
    case 'a':
    case 'd':
    case 'e':
      return true;
    default:
      return false;
    }
  }
  case 'H': {
    switch (y) {
    case 'e':
    case 'f':
    case 'g':
    case 'o':
    case 's':
      return true;
    default:
      return y == '0';
    }
  }
  case 'I': {
    switch (y) {
    case 'n':
    case 'r':
      return true;
    default:
      return y == '0';
    }
  }
  case 'J': {
    return false;
  }
  case 'K': {
    switch (y) {
    case 'r':
      return true;
    default:
      return y == '0';
    }
  }
  case 'L': {
    switch (y) {
    case 'a':
    case 'i':
    case 'r':
    case 'u':
    case 'v':
      return true;
    default:
      return false;
    }
  }
  case 'M': {
    switch (y) {
    case 'c':
    case 'd':
    case 'g':
    case 'n':
    case 'o':
    case 't':
      return true;
    default:
      return false;
    }
  }
  case 'N': {
    switch (y) {
    case 'a':
    case 'b':
    case 'd':
    case 'e':
    case 'h':
    case 'i':
    case 'o':
    case 'p':
      return true;
    default:
      return y == '0';
    }
  }
  case 'O': {
    switch (y) {
    case 'g':
    case 's':
      return true;
    default:
      return y == '0';
    }
  }
  case 'P': {
    switch (y) {
    case 'a':
    case 'b':
    case 'd':
    case 'm':
    case 'o':
    case 'r':
    case 't':
    case 'u':
      return true;
    default:
      return y == '0';
    }
  }
  case 'Q': {
    return false;
  }
  case 'R': {
    switch (y) {
    case 'a':
    case 'b':
    case 'e':
    case 'f':
    case 'g':
    case 'h':
    case 'n':
    case 'u':
      return true;
    default:
      return false;
    }
  }
  case 'S': {
    switch (y) {
    case 'b':
    case 'c':
    case 'e':
    case 'g':
    case 'i':
    case 'm':
    case 'n':
    case 'r':
      return true;
    default:
      return y == '0';
    }
  }
  case 'T': {
    switch (y) {
    case 'a':
    case 'b':
    case 'c':
    case 'e':
    case 'h':
    case 'i':
    case 'l':
    case 'm':
    case 's':
      return true;
    default:
      return false;
    }
  }
  case 'U': {
    return y == '0';
  }
  case 'V': {
    return y == '0';
  }
  case 'W': {
    return y == '0';
  }
  case 'X': {
    switch (y) {
    case 'e':
      return true;
    default:
      return false;
    }
  }
  case 'Y': {
    switch (y) {
    case 'b':
      return true;
    default:
      return y == '0';
    }
  }
  case 'Z': {
    switch (y) {
    case 'n':
    case 'r':
      return true;
    default:
      return false;
    }
  }
  default:
    return false;
  }
}

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
