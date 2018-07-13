#ifndef CUSTOM_OBJECT_H
#define CUSTOM_OBJECT_H

#include <string>
#include <iostream>

class CustomObject
{
public:
  CustomObject(const std::string& name, int value) : name_(name), value_(value) {}
  ~CustomObject() { std:: cout << "[~CustomObject] (" << name_ << ")\n"; }
  std::string getName() const {return name_;}
  void setName(const std::string& name) { name_ = name; }
  int getValue() const { return value_; }
  void setValue(int value) { value_ = value;}
private:
  std::string name_;
  int value_;
};

#endif // CUSTOM_OBJECT_H
