/*
 *  Format class
 *
 *  Please READ /License.txt FIRST! 
 *
 *  Copyright (C)2005 Sijawusz Pur Rahnama
 *  Copyright (C)2005 Winthux
 *
 *  $Id$
 */

#pragma once

namespace kAway2 {
  typedef std::string (*tFunc)();

  enum enType {
    FUNCTION,
    STRING
  };

  struct helpVar {
    const std::string name; // nazwa zmiennej
    const std::string desc; // opis zmiennej

    // konstruktor
    helpVar(std::string _name, std::string _desc) : name(_name), desc(_desc) { }
  };

  struct sVar {
    std::string name; // nazwa zmiennej
    enType type; // typ zmiennej
    std::string value; // wartosc zmiennej (TYPE == STRING)
    tFunc function; // funkcja, kt�ra zwraca string (TYPE == FUNCTION)

    // defaultowy konstruktor, �eby nie trzeba by�o si� bawi� w deklarowanie zmiennych
    sVar(std::string _name, enType _type, tFunc function, std::string value) : 
      name(_name), type(_type) {
      switch(_type) {
        case FUNCTION:
          this->function = function;
          break;
        case STRING:
          this->value = value;
          break;
      }
    }
  };

  typedef std::list<helpVar> tHelpVars;
  typedef std::list<sVar> tVars;

  class Format {
    public:
      Format(bool format = true, std::string pattern = "/\{([^a-z0-9]*)([a-z0-9]+)([^a-z0-9]*)\}/i");
      ~Format();

    public:
      std::string parse(std::string txt);

      std::string buildHtmlHelp();
      std::string buildHtmlHelp(tHelpVars vars);

      inline void clearVars() {
        this->vars.clear();
      }

      std::string getVar(std::string name);
      bool getVar(std::string name, std::string &val);

      void addVar(std::string name, tFunc function);
      void addVar(std::string name, std::string value);

      void removeVar(std::string name);
      bool varExists(std::string name);

      bool format;

    protected:
      std::string pattern;
      tVars vars;
  };
}