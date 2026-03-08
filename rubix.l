%option noyywrap
%option yylineno

%{
#include <iostream>
#include "my_lexer.h"
extern MyLexer *lexer;
extern RubixState rubix_state;
extern ChemicalState chemical_state;
extern CardState card_state;
extern ScannerState scanner_state;
extern std::string c_line;
%}

rubixCharacter [UDLRFBudlrfbMESXYZ]
cardsCharacter [23456789AJKQ]
%x RUBIX CHEMICAL CARDS
%%

<INITIAL>"\n" {}
<RUBIX>"\n" {
  lexer->resultHandler(State::Rubix);
}
<CHEMICAL>"\n" {
  lexer->resultHandler(State::Chemical);
}
<CARDS>"\n" {
  lexer->resultHandler(State::Card);
}

<INITIAL>"/" {
  resetChemicalState(&chemical_state);
  // We don't need it for chemical, but we need to add to the line
  // unput(lexer->yytext[0]);
  c_line.append(lexer->yytext);
  BEGIN(CHEMICAL);
}
<INITIAL>{cardsCharacter}|"." {
  resetCardState(&card_state);
  unput(lexer->yytext[0]);
  BEGIN(CARDS);
}
<INITIAL>{rubixCharacter} {
  resetRubixState(&rubix_state);
  unput(lexer->yytext[0]);
  BEGIN(RUBIX);
}
<INITIAL>. {
  c_line.append(lexer->yytext);
  lexer->errorHandler(ConsumeLine::Consume);
}

<RUBIX>{rubixCharacter} {
  c_line.append(lexer->yytext);
  transitionRubix(&rubix_state, RubixTag::Character);
}
<RUBIX>"'" {
  c_line.append(lexer->yytext);
  transitionRubix(&rubix_state, RubixTag::Apostrophe);
}
<RUBIX>"2" {
  c_line.append(lexer->yytext);
  transitionRubix(&rubix_state, RubixTag::Twice);
}
<RUBIX>. {
  c_line.append(lexer->yytext);
  lexer->errorHandler(ConsumeLine::Consume);
}

<CHEMICAL>[[:upper:]] {
  c_line.append(lexer->yytext);
  transitionChemical(&chemical_state, ChemicalTag::Upper, lexer->yytext[0]);
}
<CHEMICAL>[[:lower:]] {
  c_line.append(lexer->yytext);
  transitionChemical(&chemical_state, ChemicalTag::Lower, lexer->yytext[0]);
}
<CHEMICAL>[[:digit:]]{-}[01] {
  c_line.append(lexer->yytext);
  transitionChemical(&chemical_state, ChemicalTag::Digit, lexer->yytext[0]);
}
<CHEMICAL>"/" {
  c_line.append(lexer->yytext);
  transitionChemical(&chemical_state, ChemicalTag::End, lexer->yytext[0]);
}
<CHEMICAL>. {
  c_line.append(lexer->yytext);
  lexer->errorHandler(ConsumeLine::Consume);
}

<CARDS>{cardsCharacter} {
  c_line.append(lexer->yytext);
  transitionCard(&card_state, CardTag::Card, lexer->yytext[0]);
}
<CARDS>"." {
  c_line.append(lexer->yytext);
  transitionCard(&card_state, CardTag::Dot, lexer->yytext[0]);
}
<CARDS>. {
  c_line.append(lexer->yytext);
  lexer->errorHandler(ConsumeLine::Consume);
} 

%%
