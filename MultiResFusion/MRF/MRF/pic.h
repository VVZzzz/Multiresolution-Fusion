#pragma once
#include <QImage>
#include <vector>
#include <string>
using namespace std;
class Thumbnail: public vector<string> {
  //�õ��ļ���
  string getFileName(int i) const {
    string res;
    if (this->empty()) return res;
    auto pos = (*this)[i].find_last_of('/');
    res = (*this)[i].substr(pos + 1);
    return res;
  }

};
