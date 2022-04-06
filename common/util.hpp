#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string.hpp>
#include <fstream>

using std::cout;
using std::endl;
using std::string;
using std::vector;   
using std::unordered_map;

namespace common {
class Util {
  public:
    // 根据文件名读取所有内容，写入输出型参数 output 中
    static bool Read(const string& input_path, string* output) {
        // 1.打开文件
        std::ifstream file(input_path.c_str());
        if(!file.is_open()) {
          std::cout << "Open file " << input_path << " failing" <<std::endl;
          return false;
        }
        // 2.从流中按行读取文件内容至line，追加line到output
        string line;
        while(std::getline(file, line)) {
          *output += (line + '\n');
        }
        // 3.关闭文件
        file.close();
        return true;
    }



    // 切分字符串，参数为：
    // input: 原字符串，是一个string
    // delimiter：按照什么来切分，是一个string
    // output：输出型参数，是一个string数组
    static void Split(const string& input, const string& delimiter, vector<string>* output) {
        boost::split(*output, input, boost::is_any_of(delimiter), boost::token_compress_off);
    }
};
} //namespace end
