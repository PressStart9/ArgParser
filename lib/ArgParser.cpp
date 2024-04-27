#include "ArgParser.h"

namespace argument_parser {

Argument::Argument(const std::string& long_form, char short_form) : long_form_(long_form), short_form_(short_form) {}

template<typename T>
argument_parser::ParserArgument<T>::ParserArgument(const std::string& long_form, T& variable, char short_form, bool has_default)
    : Argument(long_form, short_form), variable_(&variable), is_init_(has_default) {}

template<typename T>
void argument_parser::ParserArgument<T>::PushValue(const std::string& value) {
  is_init_ = true;
  std::stringstream convert(value);
  convert >> *variable_;
}

template<typename T>
bool ParserArgument<T>::Check() {
  return is_init_;
}

template<typename T>
argument_parser::ParserVector<T>::ParserVector(const std::string& long_form, std::vector<T>& variable, int minimal_count, char short_form)
    : Argument(long_form, short_form), variables_(&variable), minimal_count_(minimal_count) {}

template<typename T>
void argument_parser::ParserVector<T>::PushValue(const std::string& value) {
  std::stringstream convert(value);
  T temp;
  convert >> temp;
  variables_->push_back(temp);
}

template<typename T>
bool ParserVector<T>::Check() {
  return variables_->size() >= minimal_count_;
}

ArgParser::ArgParser() {
  positional_ = nullptr;
}

ArgParser::~ArgParser() {
  delete positional_;
  for (auto& s : arguments_) {
    auto sc = s.second;
    if (sc == nullptr) {
      continue;
    }
    if (arguments_.contains(sc->long_form_)) {
      arguments_[sc->long_form_] = nullptr;
    }
    if (arguments_.contains(std::string(1, sc->short_form_))) {
      arguments_[std::string(1, sc->short_form_)] = nullptr;
    }
    delete sc;
  }
}

bool ArgParser::Parse(const std::vector<std::string>& command_arguments) {
  for (int i = 1; i < command_arguments.size(); ++i) {
    std::string_view arg_name = command_arguments[i];

    if (arg_name[0] == '-' && !std::isdigit(arg_name[1])) {
      arg_name = arg_name.substr(1);
      if (arg_name[0] == '-') {
        arg_name = arg_name.substr(1);
      }
    } else {
      if (positional_ == nullptr) {
        return false;
      }
      positional_->PushValue({arg_name.begin(), arg_name.end()});
      continue;
    }

    size_t equality_sign = arg_name.find('=');
    std::string name;
    name = arg_name.substr(0, equality_sign);
    if (!arguments_.contains(name)) {
      return false;
    }

    std::string_view arg_value;
    if (equality_sign != std::string_view::npos) {
      arg_value = arg_name.substr(equality_sign + 1, arg_name.size() - equality_sign);
    } else if (typeid(*arguments_[name]) == typeid(ParserArgument<bool>)) {
      arg_value = "1";
    } else {
      ++i;
      arg_value = command_arguments[i];
    }

    arguments_[name]->PushValue({arg_value.begin(), arg_value.end()});
  }

  for (auto pair : arguments_) {
    if (!pair.second->Check()) {
      return false;
    }
  }

  return true;
}

bool ArgParser::Parse(int arg_count, char** command_arguments) {
  return Parse(std::vector<std::string>(command_arguments, command_arguments + arg_count));
}

template<typename T>
void ArgParser::AddArgument(const std::string& long_form, T& variable, bool has_default) {
  AddArgument(long_form, '\0', variable, has_default);
}

template<typename T>
void ArgParser::AddArgument(const std::string& long_form, char short_form, T& variable, bool has_default) {
  auto arg = new ParserArgument(long_form, variable, short_form, has_default);
  arguments_[long_form] = arg;
  if (short_form != '\0') {
    arguments_[std::string(1, short_form)] = arg;
  }
}

template<typename T>
void ArgParser::AddVector(const std::string& long_form, std::vector<T>& variable, int minimal_count) {
  AddVector(long_form, '\0', variable, minimal_count);
}

template<typename T>
void ArgParser::AddVector(const std::string& long_form, char short_form, std::vector<T>& variable, int minimal_count) {
  auto arg = new ParserVector(long_form, variable, minimal_count, short_form);
  arguments_[long_form] = arg;
  if (short_form != '\0') {
    arguments_[std::string(1, short_form)] = arg;
  }
}

template<typename T>
void ArgParser::SetPositional(std::vector<T>& variable, int minimal_count) {
  positional_ = new ParserVector<T>(kReservedName_, variable, minimal_count);
}

template class ParserArgument<int>;
template class ParserArgument<bool>;
template class ParserArgument<std::string>;

template class ParserVector<int>;
template class ParserVector<bool>;
template class ParserVector<std::string>;

template void ArgParser::AddArgument<int>(const std::string& long_form, int& variable, bool has_default);
template void ArgParser::AddArgument<bool>(const std::string& long_form, bool& variable, bool has_default);
template void ArgParser::AddArgument<std::string>(const std::string& long_form, std::string& variable, bool has_default);
template void ArgParser::AddArgument<int>(const std::string& long_form, char short_form, int& variable, bool has_default);
template void ArgParser::AddArgument<bool>(const std::string& long_form, char short_form, bool& variable, bool has_default);
template void ArgParser::AddArgument<std::string>(const std::string& long_form, char short_form, std::string& variable, bool has_default);

template void ArgParser::AddVector<int>(const std::string& long_form, std::vector<int>& variable, int minimal_count);
template void ArgParser::AddVector<bool>(const std::string& long_form, std::vector<bool>& variable, int minimal_count);
template void ArgParser::AddVector<std::string>(const std::string& long_form, std::vector<std::string>& variable, int minimal_count);
template void ArgParser::AddVector<int>(const std::string& long_form, char short_form, std::vector<int>& variable, int minimal_count);
template void ArgParser::AddVector<bool>(const std::string& long_form, char short_form, std::vector<bool>& variable, int minimal_count);
template void ArgParser::AddVector<std::string>(const std::string& long_form, char short_form, std::vector<std::string>& variable, int minimal_count);

template void ArgParser::SetPositional<int>(std::vector<int>& variable, int minimal_count);
template void ArgParser::SetPositional<bool>(std::vector<bool>& variable, int minimal_count);
template void ArgParser::SetPositional<std::string>(std::vector<std::string>& variable, int minimal_count);

}  // namespace argument_parser
