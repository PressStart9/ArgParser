#include "ArgParser.h"

namespace ArgumentParser {

template<typename T>
bool ParserArgument<T>::SetValue(const std::string_view& value) {
  if (is_multi_value_ || values_.empty()) {
    values_.emplace_back(value.begin(), value.end());
  } else if (is_default_) {
    values_[0] = {value.begin(), value.end()};
  } else {
    return false;
  }

  return true;
}

template<typename T>
ParserArgument<T>::ParserArgument(const std::string& l_form) {
  long_form_ = l_form;
  short_form_ = '\0';
  description_ = '\0';

  minimal_count_ = 1;
  is_multi_value_ = false;
  is_positional_ = false;
  is_default_ = false;

  variable_ = nullptr;
  variables_ = nullptr;
}

template<typename T>
ParserArgument<T>& ParserArgument<T>::Default(const T& value) {
  std::string ret;
  std::ostringstream ss;
  ss << value;
  ret = ss.str();
  if (values_.empty()) {
    values_.push_back(ret);
  }
  return *this;
}

template<typename T>
ParserArgument<T>& ParserArgument<T>::MultiValue() {
  return MultiValue(kDefaultValuesCount);
}

template<typename T>
ParserArgument<T>& ParserArgument<T>::MultiValue(int value_count) {
  is_multi_value_ = true;
  minimal_count_ = value_count;
  return *this;
}

template<typename T>
ParserArgument<T>& ParserArgument<T>::Positional() {
  is_positional_ = true;
  return *this;
}

template<typename T>
ParserArgument<T>& ParserArgument<T>::StoreValue(T& value) {
  variable_ = &value;
  return *this;
}

template<typename T>
ParserArgument<T>& ParserArgument<T>::StoreValues(std::vector<T>& values) {
  variables_ = &values;
  return *this;
}


ArgParser::ArgParser(const std::string& name) {
  short_help_ = '\0';
  long_help_ = '\0';
  help_text_ = '\0';

  name_ = name;
}

ArgParser::~ArgParser() {
  std::string key;
  for (auto& s : string_argument_) {
    auto sc = s.second;
    if (!sc) {
      continue;
    }
    key = sc->long_form_.insert(0, "--");
    if (string_argument_.contains(key)) {
      string_argument_[key] = nullptr;
    }
    key = std::string("--").append(1, sc->short_form_);
    if (string_argument_.contains(key)) {
      string_argument_[key] = nullptr;
    }
    delete sc;
  }
  for (auto& s : int_argument_) {
    auto sc = s.second;
    if (!sc) {
      continue;
    }
    key = sc->long_form_.insert(0, "--");
    if (int_argument_.contains(key)) {
      int_argument_[key] = nullptr;
    }
    key = std::string("--").append(1, sc->short_form_);
    if (int_argument_.contains(key)) {
      int_argument_[key] = nullptr;
    }
    delete sc;
  }
  for (auto& s : boolean_argument_) {
    auto sc = s.second;
    if (!sc) {
      continue;
    }
    key = sc->long_form_.insert(0, "--");
    if (boolean_argument_.contains(key)) {
      boolean_argument_[key] = nullptr;
    }
    key = std::string("--").append(1, sc->short_form_);
    if (boolean_argument_.contains(key)) {
      boolean_argument_[key] = nullptr;
    }
    delete sc;
  }
}

void ArgParser::AddHelp(char short_form, const std::string& long_form, const std::string& description) {
  short_help_ = "-";
  short_help_ += short_form;
  long_help_ = "--" + long_form;
  help_text_ = description;
}

bool ArgParser::Help() const {
  return !short_help_.empty();
}

std::string ArgParser::HelpDescription() {
  std::string help = name_ + '\n' + help_text_ + "\n\n";
  for (const auto& arg : string_argument_) {
    if (arg.second->short_form_ != '\0') {
      help += '-';
      help += arg.second->short_form_;
      help += ",  ";
    }
    help += "--" + arg.second->long_form_ + "=<string>,  " + arg.second->description_ + '\n';
  }

  for (const auto& arg : int_argument_) {
    if (arg.second->short_form_ != '\0') {
      help += '-';
      help += arg.second->short_form_;
      help += ",  ";
    }
    help += "--" + arg.second->long_form_ + "=<int>,  " + arg.second->description_ + '\n';
  }

  for (const auto& arg : boolean_argument_) {
    if (arg.second->short_form_ != '\0') {
      help += '-';
      help += arg.second->short_form_;
      help += ",  ";
    }
    help += "--" + arg.second->long_form_ + ",  " + arg.second->description_ + '\n';
  }
  if (!long_help_.empty()) {
    if (!short_help_.empty()) {
      help += '-';
      help += short_help_;
      help += ",  ";
    }
    help += "--" + long_help_ + "\n";
  }
  return help;
}

bool ArgParser::SetPositional(const std::vector<std::string>& pos_args) {
  for (const auto& s : string_argument_) {
    if (s.second->is_positional_) {
      for (const auto& val : pos_args) {
        s.second->values_.push_back(val);
      }
      return true;
    }
  }
  for (const auto& s : int_argument_) {
    if (s.second->is_positional_) {
      for (const auto& val : pos_args) {
        s.second->values_.push_back(val);
      }
      return true;
    }
  }
  for (const auto& s : boolean_argument_) {
    if (s.second->is_positional_) {
      for (const auto& val : pos_args) {
        s.second->values_.push_back(val);
      }
      return true;
    }
  }

  return false;
}

bool ArgParser::PassAllArguments() {
  for (auto& arg : string_argument_) {
    if (arg.second->values_.size() < arg.second->minimal_count_ || arg.second->values_.empty()) {
      return false;
    }
    if (arg.second->variable_ != nullptr) {
      *arg.second->variable_ = arg.second->values_[0];
    } else if (arg.second->variables_ != nullptr) {
      *arg.second->variables_ = arg.second->values_;
    }
  }
  for (auto& arg : int_argument_) {
    if (arg.second->values_.size() < arg.second->minimal_count_ || arg.second->values_.empty()) {
      return false;
    }
    if (arg.second->variable_ != nullptr) {
      *arg.second->variable_ = std::stoi(arg.second->values_[0]);
    } else if (arg.second->variables_ != nullptr) {
      std::vector<int> new_ints;
      new_ints.reserve(arg.second->values_.size());
      for (const auto& str : arg.second->values_) {
        new_ints.push_back(std::stoi(str));
      }
      *arg.second->variables_ = new_ints;
    }
  }
  for (auto& arg : boolean_argument_) {
    if (arg.second->values_.size() < arg.second->minimal_count_ || arg.second->values_.empty()) {
      return false;
    }
    if (arg.second->variable_ != nullptr) {
      *arg.second->variable_ = arg.second->values_[0][0] == '1';
    } else if (arg.second->variables_ != nullptr) {
      std::vector<bool> new_ints;
      new_ints.reserve(arg.second->values_.size());
      for (const auto& str : arg.second->values_) {
        new_ints.push_back(str[0] == '1');
      }
      *arg.second->variables_ = new_ints;
    }
  }
  return true;
}

bool ArgParser::Parse(const std::vector<std::string>& command_arguments) {
  if (command_arguments.size() == 2 && (short_help_ == command_arguments[1] || long_help_ == command_arguments[1])) {
    return true;
  }

  for (int i = 1; i < command_arguments.size(); ++i) {
    std::string_view arg_name = command_arguments[i];
    size_t equality_sign = arg_name.find('=');
    std::string_view arg_value;
    if (equality_sign != std::string_view::npos) {
      arg_value = arg_name.substr(equality_sign + 1, arg_name.size() - equality_sign);
      arg_name = arg_name.substr(0, equality_sign);
    } else if (arg_name[0] != '-') {
      std::vector<std::string> pos_args;
      while (i < command_arguments.size() && command_arguments[i][0] != '-') {
        pos_args.push_back(command_arguments[i]);
        ++i;
      }
      --i;

      if (!SetPositional(pos_args)) {
        return false;
      }
      continue;
    } else if (i + 1 < command_arguments.size()) {
      ++i;
      arg_value = command_arguments[i];
    }

    std::string name = {arg_name.begin(), arg_name.end()};
    if (arg_name[1] != '-') {
      name.insert(0, "-");
    }

    if ((string_argument_.contains(name) && string_argument_[name]->SetValue(arg_value)) ||
        (int_argument_.contains(name) && int_argument_[name]->SetValue(arg_value)) ||
        (boolean_argument_.contains(name) && boolean_argument_[name]->SetValue("1"))) {
      continue;
    }

    name = std::string("-- ");
    for (const auto& c : arg_name) {
      if (c == '-') {
        continue;
      }
      name[2] = c;
      if (!(boolean_argument_.contains(name) && boolean_argument_.at(name)->SetValue("1"))) {
        return false;
      }
    }
  }

  return PassAllArguments();
}

bool ArgParser::Parse(int arg_count, char** command_arguments) {
  std::vector<std::string> v(command_arguments, command_arguments + arg_count);
  return Parse(v);
}

ParserArgument<std::string>& ArgParser::AddStringArgument(const std::string& long_form, const std::string& description) {
  return AddStringArgument('\0', long_form, description);
}

ParserArgument<std::string>& ArgParser::AddStringArgument(char short_form, const std::string& long_form, const std::string& description) {
  auto* arg = new ParserArgument<std::string>(long_form);
  arg->description_ = description;
  arg->short_form_ = short_form;

  std::string key = "--" + long_form;
  string_argument_[key] = arg;
  if (short_form != '\0') {
    key = std::string("-- ");
    key[2] = short_form;
    string_argument_[key] = arg;
  }

  return *arg;
}

std::string ArgParser::GetStringValue(const std::string& long_form) {
  std::string key = "--" + long_form;
  if (string_argument_.contains(key)) {
    return string_argument_[key]->values_[0];
  }
  return "";
}

std::string ArgParser::GetStringValue(const std::string& long_form, int index) {
  std::string key = "--" + long_form;
  if (string_argument_.contains(key)) {
    return string_argument_[key]->values_[index];
  }
  return "";
}

ParserArgument<int>& ArgParser::AddIntArgument(const std::string& long_form, const std::string& description) {
  return AddIntArgument('\0', long_form, description);
}

ParserArgument<int>& ArgParser::AddIntArgument(char short_form, const std::string& long_form, const std::string& description) {
  auto* arg = new ParserArgument<int>(long_form);
  arg->description_ = description;
  arg->short_form_ = short_form;

  std::string key = "--" + long_form;
  int_argument_[key] = arg;
  if (short_form != '\0') {
    key = std::string("-- ");
    key[2] = short_form;
    int_argument_[key] = arg;
  }

  return *arg;
}

int ArgParser::GetIntValue(const std::string& long_form) {
  std::string key = "--" + long_form;
  if (int_argument_.contains(key)) {
    return std::stoi(int_argument_[key]->values_[0]);
  }
  return 0;
}

int ArgParser::GetIntValue(const std::string& long_form, int index) {
  std::string key = "--" + long_form;
  if (int_argument_.contains(key)) {
    return std::stoi(int_argument_[key]->values_[index]);
  }
  return 0;
}

ParserArgument<bool>& ArgParser::AddFlag(const std::string& long_form, const std::string& description) {
  return AddFlag('\0', long_form, description);
}

ParserArgument<bool>& ArgParser::AddFlag(char short_form, const std::string& long_form, const std::string& description) {
  auto* arg = new ParserArgument<bool>(long_form);
  arg->short_form_ = short_form;
  arg->description_ = description;

  std::string key = "--" + long_form;
  boolean_argument_[key] = arg;
  if (short_form != '\0') {
    key = std::string("-- ");
    key[2] = short_form;
    boolean_argument_[key] = arg;
  }

  return *arg;
}

bool ArgParser::GetFlag(const std::string& long_form) {
  std::string key = "--" + long_form;
  if (boolean_argument_.contains(key)) {
    return boolean_argument_[key]->values_[0][0] == '1';
  }
  return false;
}

bool ArgParser::GetFlag(const std::string& long_form, int index) {
  std::string key = "--" + long_form;
  if (boolean_argument_.contains(key)) {
    return boolean_argument_[key]->values_[index][0] == '1';
  }
  return false;
}

template class ParserArgument<int>;
template class ParserArgument<bool>;
template class ParserArgument<std::string>;

}  // namespace ArgumentParser
