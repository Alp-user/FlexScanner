%option noyywrap
%option yylineno

%{
#include <iostream>
#include "my_lexer.h"
extern MyLexer *lexer;
extern RubixState rubix_state;
extern ChemicalState chemical_state;
extern std::string c_line;
%}

rubixCharacter [UDLRFBudlrfbMESXYZxyz]
cardsCharacter [23456789AJKQ]
%x RUBIX CHEMICAL CARDS
%%

<RUBIX>"\n" {
  lexer->successHandler();
  BEGIN(INITIAL);
}
<CHEMICAL>"\n" {
  if (chemical_state.tag == ChemicalTag::End){
    lexer->successHandler();
  }else{
    std::cerr << "The line causing the error: " << c_line << std::endl;
    c_line.clear();
  }
  BEGIN(INITIAL);
}
<CARDS>"\n" {
  lexer->successHandler();
  BEGIN(INITIAL);
}

<INITIAL>"/" {
  resetChemicalState(&chemical_state);
  // We don't need it for chemical, but we need to add to the line
  // unput(lexer->yytext[0]);
  c_line.append(lexer->yytext);
  BEGIN(CHEMICAL);
}
<INITIAL>{cardsCharacter} {
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
  lexer->errorHandler();
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
  lexer->errorHandler();
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
  lexer->errorHandler();
}

<CARDS>{cardsCharacter} {
  c_line.append(lexer->yytext);
}
<CARDS>"." {
  c_line.append(lexer->yytext);
}
<CARDS>. {
  c_line.append(lexer->yytext);
  lexer->errorHandler();
} 

%%
