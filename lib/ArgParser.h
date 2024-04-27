#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <sstream>
#include <iostream>

namespace argument_parser {

class Argument {
 protected:
  Argument(const std::string& long_form, char short_form);
  virtual ~Argument() = default;

  char short_form_;
  std::string long_form_;;

  virtual void PushValue(const std::string& value) = 0;
  virtual bool Check() = 0;

  friend class ArgParser;
};

template<typename T>
class ParserArgument : public Argument {
 public:
  ParserArgument(const std::string& long_form, T& variable, char short_form = '\0', bool has_default = false);

 private:
  T* variable_;
  bool is_init_;

  void PushValue(const std::string& value) override;
  bool Check() override;

  friend class ArgParser;
};

template<typename T>
class ParserVector : public Argument {
 public:
  ParserVector(const std::string& long_form, std::vector<T>& variable, int minimal_count, char short_form = '\0');

 private:
  std::vector<T>* variables_;
  int minimal_count_;

  void PushValue(const std::string& value) override;
  bool Check() override;

  friend class ArgParser;
};

class ArgParser {
 public:
  ArgParser();
  ~ArgParser();

  bool Parse(const std::vector<std::string>& command_arguments);
  bool Parse(int arg_count, char** command_arguments);

  template<typename T>
  void AddArgument(const std::string& long_form, T& variable, bool has_default = false);
  template<typename T>
  void AddArgument(const std::string& long_form, char short_form, T& variable, bool has_default = false);

  template<typename T>
  void AddVector(const std::string& long_form, std::vector<T>& variable, int minimal_count = 1);
  template<typename T>
  void AddVector(const std::string& long_form, char short_form, std::vector<T>& variable, int minimal_count = 1);

  template<typename T>
  void SetPositional(std::vector<T>& variable, int minimal_count = 1);

 private:
  std::unordered_map<std::string, Argument*> arguments_;
  Argument* positional_;

  const char kReservedName_[4] = "$$$";
};

}  // namespace argument_parser
