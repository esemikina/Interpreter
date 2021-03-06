/*main.cpp*/

//Author: Yelizaveta Semikina, UIC, Spring 2021
//Assignment: Project 03 Part 02

#include <iostream>
#include <string>
#include <math.h>    

#include "token.h"

using namespace std;

struct Variable {
    string name;
    string value;
    int type;
};

enum Types {
  STRING = 0,
  INT,
  BOOLEAN,
  UNDEFINED
};

// token functions
Variable convertTokenToVariable(Token* token_arr, int& curr_token_idx, Token token, Variable* mem_arr, int mem_size);
bool tokenIsSign(Token token);
bool tokenIsMathSign(Token token);
bool tokenIsLogicalSign(Token token);

// variables functions 
Variable doVariableMath(int sign, Variable first, Variable second);
Variable doVariableLogic(int sign, Variable first_var, Variable second_var);

// execute functions
void executeToken(Token* token_arr, int& curr_token_idx, Variable* mem_arr, int mem_size, int& mem_used);
void executeStatement(Token* token_arr, int& curr_token_idx, Variable* mem_arr, int mem_size, int& mem_used);
void executeIfStatement(Token* token_arr, int& curr_token_idx, Variable* mem_arr, int mem_size, int& mem_used);
Variable execFuncCall(string func_name, Variable param);
Variable execFuncCall(string func_name);
void print(Token* token_arr, int& curr_token_idx, Variable* mem_arr, int mem_size);

// search through memory function
int searchVariable(string name, Variable* mem_arr, int mem_size);


//
// main filename memorysize
//
int main(int argc, char* argv[]) 
{
  // 
  // 1. process command-line args:
  //
  // argv[0]: ./main
  // argv[1]: filename.py
  // argv[2]: memorysize
  //
  if (argc != 3)
  {
    cout << "usage: ./main filename.py memorysize" << endl;
    return 0;
  }

  // this is your SimplePy program file
  string filename(argv[1]);  
  // this is size for Memory array
  int MemorySize = stoi(argv[2]);  

  //
  // TODO: call inputSimplePy to input the SimplePy program, and
  // you'll get back an array of Tokens.  Allocate another array
  // for your memory, and then start executing!
  //

  Variable* mem_arr = new Variable[MemorySize];
  int mem_used = 0, tokensCount;

  // take all the tokens from the file
  Token* token_arr = inputSimplePy(filename, tokensCount); 
 
  int curr_token_idx = 0; 
  // go through each token
  while (token_arr[curr_token_idx].ID != TokenID::EOS) {
  
    executeToken(token_arr, curr_token_idx, mem_arr, MemorySize, mem_used);
   
    curr_token_idx++; 
  }
  // free allocated arrays
  delete[] mem_arr;
  delete[] token_arr;

  return 0;
}


void executeToken(
  Token* token_arr, 
  int& curr_token_idx, 
  Variable* mem_arr, 
  int mem_size,
  int& mem_used) 
{
  // If a token is Identifier
  if(token_arr[curr_token_idx].ID == TokenID::IDENTIFIER) {
    // print function
    if(token_arr[curr_token_idx].Value == "print") {
      print(token_arr, curr_token_idx, mem_arr, mem_size);
    } else {
    // Just some statement 
      executeStatement(token_arr, curr_token_idx, mem_arr, mem_size, mem_used);
    }
  // If a token is an IF
  } else if(token_arr[curr_token_idx].ID == TokenID::IF_KEYW) {
    executeIfStatement(token_arr, curr_token_idx, mem_arr, mem_size, mem_used);
  } else {

  }
}

// If statement execute function
void executeIfStatement(
  Token* token_arr, 
  int& curr_token_idx, 
  Variable* mem_arr, 
  int mem_size, 
  int& mem_used) 
{
// Evaluating the condition
  Variable result_variable, first_var, second_var;
  int sign = -1;

  curr_token_idx++;

  first_var = convertTokenToVariable(token_arr, curr_token_idx, token_arr[curr_token_idx], mem_arr, mem_size);
 
  curr_token_idx++;

  sign = token_arr[curr_token_idx].ID;
  curr_token_idx++;

  second_var = convertTokenToVariable(token_arr, curr_token_idx, token_arr[curr_token_idx], mem_arr, mem_size);
  // Save Logic result
  result_variable = doVariableLogic(sign, first_var, second_var);
  // Skip the colon token
  curr_token_idx+=2;
  // if the result is true
  if( stoi(result_variable.value) == 1 ) {
    // check for indent present
    if ( token_arr[curr_token_idx].ID == TokenID::INDENT ) {
      // Do executes from indent to unindent
      while(token_arr[curr_token_idx].ID != UNDENT) {
        executeToken(token_arr, curr_token_idx, mem_arr, mem_size, mem_used);
        curr_token_idx++;
      }
     
      // Check the next token to be anything but not elif or else
      curr_token_idx++;
      if(token_arr[curr_token_idx].ID != TokenID::ELIF_KEYW && token_arr[curr_token_idx].ID != TokenID::ELSE_KEYW) {
        // If the token is not elif or else, then just continue execution
      } else {
        bool skipped = false;
        // if the token is indeed an elif or else, skip to the next UNINDENT
        while( !skipped ) {
        
          curr_token_idx++;
      \
          if(token_arr[curr_token_idx].ID == TokenID::UNDENT) {
            // check next token 
            curr_token_idx++;
            // check elif and else
            if(token_arr[curr_token_idx].ID == TokenID::ELIF_KEYW 
            || token_arr[curr_token_idx].ID == TokenID::ELSE_KEYW) {
              continue;
            } else {
              skipped = true;
            }
          }
        }
        // Skipped 
        curr_token_idx--;
      }

    } else {
      // Skip if no Indent is present
    }
  } else {
  // if the result is false
    // skip to indent part
    if(token_arr[curr_token_idx].ID == TokenID::INDENT) {
      // if there is an indent 
      // skip to unindent
      while(token_arr[curr_token_idx].ID != TokenID::UNDENT) {
        curr_token_idx++;
      }
      // we are on undent
      // skip undent token 
      curr_token_idx++;
      // check if we have ifelse
      if(token_arr[curr_token_idx].ID == TokenID::ELIF_KEYW) {
        // execute that elif statement
        executeIfStatement(token_arr, curr_token_idx, mem_arr, mem_size, mem_used);
      // if we do not have elif statement but just else
      } else if(token_arr[curr_token_idx].ID == TokenID::ELSE_KEYW) {
        // execute everything untill undent
        while(token_arr[curr_token_idx].ID == TokenID::UNDENT) {
          executeToken(token_arr, curr_token_idx, mem_arr, mem_size, mem_used);
          curr_token_idx++;
        }
        // ended on undent
      } else {
        // continue executing
      }
    }
  }
}

void print(
  Token* token_arr, 
  int& curr_token_idx, 
  Variable* mem_arr, 
  int mem_size) 
{
  // skip "("" 
  curr_token_idx+=2;
  // read next tokens untill ")"
  while( token_arr[curr_token_idx].ID != TokenID::RIGHT_PAREN ) {
    // check if the token is either: literal int or string or Identifier 
    Token curr_token = token_arr[curr_token_idx];

    // if string or int literal 
    if( curr_token.ID == TokenID::STR_LITERAL 
    || curr_token.ID == TokenID::INT_LITERAL ) {
      cout << curr_token.Value;
    // if its a variable
    } else if( curr_token.ID == TokenID::IDENTIFIER ) {
      int foundIdx = searchVariable(curr_token.Value, mem_arr, mem_size);
      cout << mem_arr[foundIdx].value;
    // if "," output space
    } else if( curr_token.ID == TokenID::COMMA ) {
      cout << " ";
    } 
    curr_token_idx++;
  }
  cout << endl;
}

void executeStatement(
  Token* token_arr, 
  int& curr_token_idx, 
  Variable* mem_arr, 
  int mem_size, 
  int& mem_used) 
{

  Variable main_var;
  string var_name = token_arr[curr_token_idx].Value;
  // skip "="
  curr_token_idx+=2;
  // Read the first Value and set the main_var to be the same type and value
  main_var = convertTokenToVariable(token_arr, curr_token_idx, token_arr[curr_token_idx], mem_arr, mem_size);
  curr_token_idx++;

  bool hasSign = false;
  // check if the current token is a sign
  if(tokenIsSign(token_arr[curr_token_idx])) 
    hasSign = true;

  // process all vals and do math or logic untill there is no signs left
  while(hasSign) {
    // check if its a math sign or logical one 
    // if math sign 
    if(tokenIsMathSign(token_arr[curr_token_idx])) {
      int sign = token_arr[curr_token_idx].ID;
      curr_token_idx++;
      // convert to Variable
      Variable captured_variable = convertTokenToVariable(token_arr, curr_token_idx, token_arr[curr_token_idx], mem_arr, mem_size);
      // do math 
      main_var = doVariableMath(sign, main_var, captured_variable);

    } // If its a logical operator
    else if(tokenIsLogicalSign(token_arr[curr_token_idx])) {
      int sign = token_arr[curr_token_idx].ID;
      curr_token_idx++;
      // convert to var
      Variable captured_variable = convertTokenToVariable(token_arr, curr_token_idx, token_arr[curr_token_idx], mem_arr, mem_size);
      // do logic 
      main_var = doVariableLogic(sign, main_var, captured_variable);
    }
    curr_token_idx++;
    // check if its a sign
    hasSign = tokenIsSign(token_arr[curr_token_idx]);
  }
  // decrease idx to go one token back 
  curr_token_idx--;
  // set the name of the main_var
  main_var.name = var_name;
  // search if the variable is already present in the memory
  int found_var_idx = searchVariable(var_name, mem_arr, mem_size);
  if( found_var_idx != -1 ) {
    mem_arr[found_var_idx] = main_var;
  } else {
    // add main_var to mem_array
    mem_arr[mem_used] = main_var;
    if(mem_used < mem_size) {
      // increase mem_used
      mem_used++;
    }
    
  }
  
}

// Search Variables function 
int searchVariable(string name, Variable* mem_arr, int mem_size) {
  for(int i = 0; i < mem_size; i++) {
    if(mem_arr[i].name == name) {
      return i;
    }
  }

  return -1;
}

// math function
Variable doVariableMath(int sign, Variable first_var, Variable second_var) {

  Variable result_var;
  // copy the type of var
  result_var.type = first_var.type;
  if(result_var.type == Types::INT) {
    // declare int result, first and second variables integer values
    int first_val = stoi(first_var.value), second_val = stoi(second_var.value);
    // do math based on the sign provided
    switch(sign) {
      
      case TokenID::PLUS:
        result_var.value = to_string(first_val + second_val);
      break;

      case TokenID::MINUS:
        result_var.value = to_string(first_val - second_val);
      break;
      
      case TokenID::MULT:
        result_var.value = to_string(first_val * second_val);
      break;

      case TokenID::POWER:
        result_var.value = to_string((int)pow(first_val, second_val));
      break;

      case TokenID::DIV:
        if(second_val == 0) {
          result_var.value = to_string(0);
        } else {
          result_var.value = to_string(first_val / second_val);
        }
        
      break;

      case TokenID::MOD:
      if(second_val == 0) {
          result_var.value = to_string(0);
        } else {
          result_var.value = to_string(first_val % second_val);
        }
        
      break;

      default:
      break;
    }
  } else if(result_var.type == Types::STRING) {
    string string_result = "", first_str = first_var.value, second_str = second_var.value;
    // perform string operations
    if(sign == TokenID::PLUS) {
      string_result = (first_str).append(second_str);
    } else {}
    result_var.value = string_result;
  }
  return result_var;
}

// logic function 
Variable doVariableLogic(int sign, Variable first_var, Variable second_var) {
  Variable result_var;
  result_var.type = Types::BOOLEAN;
  if(first_var.type == Types::STRING) {
    string first_str = first_var.value, second_str = second_var.value;
    // do the logic based on the provided sign
    switch(sign) {
      case(TokenID::EQUAL_EQUAL):
        result_var.value = to_string(first_str == second_str);
      break;

      case(TokenID::LT):
        result_var.value = to_string(first_str < second_str);
      break;

      case(TokenID::LTE):
        result_var.value = to_string(first_str <= second_str);
      break;

      case(TokenID::GT):
        result_var.value = to_string(first_str > second_str);
      break;

      case(TokenID::GTE):
        result_var.value = to_string(first_str >= second_str);
      break;

      case(TokenID::NOT_EQUAL):
        result_var.value = to_string(first_str != second_str);
      break;

      default:

      break;
    }
  } else {
    // get the integer values
    int first_val = stoi(first_var.value), second_val = stoi(second_var.value);
    // do the logic based on the provided sign
    switch(sign) {
      case(TokenID::EQUAL_EQUAL):
        result_var.value = to_string(first_val == second_val);
      break;

      case(TokenID::LT):
        result_var.value = to_string(first_val < second_val);
      break;

      case(TokenID::LTE):
        result_var.value = to_string(first_val <= second_val);
      break;

      case(TokenID::GT):
        result_var.value = to_string(first_val > second_val);
      break;

      case(TokenID::GTE):
        result_var.value = to_string(first_val >= second_val);
      break;

      case(TokenID::NOT_EQUAL):
        result_var.value = to_string(first_val != second_val);
      break;

      default:

      break;
    }
  }
  

  return result_var;
}

// param
Variable execFuncCall(string func_name, Variable param) {
  Variable result_var;
  if(func_name == "input") {
    // exec input function with params
    string line = "";
    cout << param.value; 
    getline(cin, line);
    result_var.value = line;
    result_var.type = Types::STRING;
  } else if(func_name == "int") {
    if(param.type == Types::INT) {
      result_var.type = Types::INT;
      result_var.value = param.value;  
    // if string
    } else if(param.type == Types::STRING) {
      bool allDigits = true;
      // check each char
      for(int i =0; i < (int)param.value.length(); i++) {
        if(!isdigit(param.value[i])) {
          allDigits = false;
        }
      }

      if(!allDigits) {
        //cout << "Cannot convert!" << endl;
      }

      result_var.type = Types::INT;
      result_var.value = param.value;
    }

  } else if(func_name == "str") {
    // if an int 
    if(param.type == Types::INT) {
      result_var.type = Types::STRING;
      result_var.value = to_string(stoi(param.value));  
    // if string
    } else if(param.type == Types::STRING) {
      result_var.type = Types::STRING;
      result_var.value = param.value;
    }
  } else if(func_name == "type") {
    // if an int 
    if(param.type == Types::INT) {
      result_var.type = Types::STRING;
      result_var.value = "int";  
    // if string
    } else if(param.type == Types::STRING) {
      result_var.type = Types::STRING;
      result_var.value = "str";
    }
  }  else {

  }

  return result_var;
  
}

// no param
Variable execFuncCall(string func_name) {
  Variable result_var;
  
  if(func_name == "input") {
    result_var.type = Types::STRING;
    // exec input function with params
    string line = "";
    getline(cin, line);
    // assing that line from cin to value of result variable
    result_var.value = line;
  }

  return result_var;
}


// Get variable from token
Variable convertTokenToVariable(
  Token* token_arr, 
  int& curr_token_idx, 
  Token token, 
  Variable* mem_arr, 
  int mem_size) 
{
  // create the resulting Variable
  Variable res;
  if(token.ID == TokenID::IDENTIFIER) {
    string var_name = token.Value;

    curr_token_idx++;
    if(token_arr[curr_token_idx].ID == TokenID::LEFT_PAREN) {
      Variable param_var;
      param_var.type = Types::UNDEFINED;
      // check for param
      curr_token_idx++;
      if(token_arr[curr_token_idx].ID == TokenID::IDENTIFIER
      || token_arr[curr_token_idx].ID == TokenID::STR_LITERAL
      || token_arr[curr_token_idx].ID == TokenID::INT_LITERAL) {
        param_var = convertTokenToVariable(token_arr, curr_token_idx, token_arr[curr_token_idx], mem_arr, mem_size);
        res = execFuncCall(var_name, param_var);
      } else {
      // function call without params
        res = execFuncCall(var_name);
      }
    } else {
      curr_token_idx--;
      // find its index
      int idx = searchVariable(var_name, mem_arr, mem_size);
      res.type = mem_arr[idx].type;
      res.value = mem_arr[idx].value;
    }
  } else if(token.ID == TokenID::INT_LITERAL) {
    // its an int
    res.type = Types::INT;
    res.value = token.Value;
  } else if(token.ID == TokenID::STR_LITERAL) {
    // its a string
    res.type = Types::STRING;
    res.value = token.Value;
  } else {

  }

  return res;
}

bool tokenIsSign(Token token) {
  if(token.ID == TokenID::PLUS 
  || token.ID == TokenID::MINUS 
  || token.ID == TokenID::MULT 
  || token.ID == TokenID::POWER 
  || token.ID == TokenID::DIV 
  || token.ID == TokenID::MOD 
  || token.ID == TokenID::LT 
  || token.ID == TokenID::LTE
  || token.ID == TokenID::GT 
  || token.ID == TokenID::GTE 
  || token.ID == TokenID::EQUAL 
  || token.ID == TokenID::EQUAL_EQUAL 
  || token.ID == TokenID::NOT_EQUAL ) {
    return true;
  }
  return false;
}

bool tokenIsMathSign(Token token) {
  if(token.ID == TokenID::PLUS 
  || token.ID == TokenID::MINUS 
  || token.ID == TokenID::MULT 
  || token.ID == TokenID::POWER 
  || token.ID == TokenID::DIV 
  || token.ID == TokenID::MOD ) {
    return true;
  }
  return false;
}

bool tokenIsLogicalSign(Token token) {
  if( token.ID == TokenID::LT 
  || token.ID == TokenID::LTE
  || token.ID == TokenID::GT 
  || token.ID == TokenID::GTE 
  || token.ID == TokenID::EQUAL 
  || token.ID == TokenID::EQUAL_EQUAL 
  || token.ID == TokenID::NOT_EQUAL ) {
    return true;
  }
  return false;
}


