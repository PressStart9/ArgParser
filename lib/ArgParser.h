#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <sstream>

namespace ArgumentParser {

template <typename T>
class ParserArgument {
 private:
  T* variable_;
  std::vector<T>* variables_;
  int minimal_count_;
  bool is_multi_value_;
  bool is_positional_;
  bool is_default_;

  std::vector<std::string> values_;

  std::string description_;
  char short_form_;
  std::string long_form_;

  bool SetValue(const std::string_view& value);

  static const int kDefaultValuesCount = 1;

 public:
  explicit ParserArgument(const std::string& l_form);

  ParserArgument<T>& Default(const T& value);

  ParserArgument<T>& MultiValue();
  ParserArgument<T>& MultiValue(int value_count);

  ParserArgument<T>& Positional();

  ParserArgument<T>& StoreValue(T& value);
  ParserArgument<T>& StoreValues(std::vector<T>& values);

  friend class ArgParser;
};


class ArgParser {
 private:
  std::string name_;

  std::string short_help_;
  std::string long_help_;
  std::string help_text_;

  std::unordered_map<std::string, ParserArgument<std::string>*> string_argument_;
  std::unordered_map<std::string, ParserArgument<int>*> int_argument_;
  std::unordered_map<std::string, ParserArgument<bool>*> boolean_argument_;

  bool SetPositional(const std::vector<std::string>& pos_args);
  bool PassAllArguments();

 public:
  explicit ArgParser(const std::string& name);
  ~ArgParser();

  void AddHelp(char short_form, const std::string& long_form, const std::string& description);

  bool Help() const;
  std::string HelpDescription();

  bool Parse(const std::vector<std::string>& command_arguments);
  bool Parse(int arg_count, char** command_arguments);

  ParserArgument<std::string>& AddStringArgument(const std::string& long_form, const std::string& description = "");
  ParserArgument<std::string>& AddStringArgument(char short_form, const std::string& long_form = "", const std::string& description = "");

  std::string GetStringValue(const std::string& long_form);
  std::string GetStringValue(const std::string& long_form, int index);

  ParserArgument<int>& AddIntArgument(const std::string& long_form, const std::string& description = "");
  ParserArgument<int>& AddIntArgument(char short_form, const std::string& long_form = "", const std::string& description = "");

  int GetIntValue(const std::string& long_form);
  int GetIntValue(const std::string& long_form, int index);

  ParserArgument<bool>& AddFlag(const std::string& long_form, const std::string& description = "");
  ParserArgument<bool>& AddFlag(char short_form, const std::string& long_form = "", const std::string& description = "");

  bool GetFlag(const std::string& long_form);
  bool GetFlag(const std::string& long_form, int index);
};

}  // namespace ArgumentParser
