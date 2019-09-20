#pragma once
#include <QImage>
#include <vector>
#include <string>
using namespace std;
class Thumbnail: public vector<string> {
  //得到文件名
  string getFileName(int i) const {
    string res;
    if (this->empty()) return res;
    auto pos = (*this)[i].find_last_of('/');
    res = (*this)[i].substr(pos + 1);
    return res;
  }

};
