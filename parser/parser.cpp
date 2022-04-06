// 这是一个预处理模块
// 1）读取html文档
// 2）去除html标签，得到 标题、url、内容
// 3）输出行文本


#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include "../common/util.hpp"


using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::unordered_map;

// boost文档html文件的输入路径
string g_input_path = "../data/input";

// 输出路径
string g_output_path = "../data/tmp/raw_input";

// 这是描述一个html文件信息的结构体
struct DocInfo {
  string title; // 标题
  string url; // url
  string content; // 内容（行文本）
};

//   core steps
// 1.枚举boost文件夹下所有html文件的路径
// 2.读取所有html文件的信息（标题、url、内容）
// 3.把结果写入到输出文件

// 枚举根目录下的所有文件名，写入vector
bool EnumFile(const string& g_input_path, vector<string>* file_list) {

    namespace fs = boost::filesystem; // 把boost::filesystem这个命名空间起个别名
    fs::path root_path(g_input_path);
    if(!fs::exists(root_path)) { // 判断目录是否存在，不存在就返回
        std::cout << "Input path is invalid" << std::endl;
        return false;
    }
    // recursive traversal
    fs::recursive_directory_iterator end_iter; // 把默认构造的迭代器作为哨兵，指向最后一个元素的下一个位置
    for(fs::recursive_directory_iterator iter(root_path); iter != end_iter; iter++) {
        if(!fs::is_regular_file(*iter)) { // 如果不是普通文件，跳过
            continue;
        }
        if(iter->path().extension() != ".html") { // 如果不是.html后缀的文件，跳过
            continue;
        }
        // 将文件名尾插到file_list中
        file_list->push_back(iter->path().string());
    }
    return true;

}

// Get html file infomation: title, url, content
bool ParseFile(const string& file_path, DocInfo* doc_info) {

    // 1. 读取文件内容
··
    if(!ret) { // 读取失败，返回
        std::cout << file_path << " :read file content failing..." << std::endl;
        return false;
    }
    // 2.extract title according to <title></title>
    ret = ParseTitle(html, &doc_info->title);
    if(!ret) {
        std::cout << file_path << " :title extracted failing..," << std::endl;
        return false;
    }
    // 3.construct online document url on the basis of file path
    ret = ParseUrl(file_path, &doc_info->url);
    if(!ret) {
        std::cout << file_path << " :extract url failing..." << std::endl;
        return false;
    }
    // 4.remove html label
    ret = ParseContent(html, &doc_info->content);
    if(!ret) {
        std::cout << file_path << " :remove html label failing..." << std::endl;
        return false;
    }
    return true;

}

// 1. Get title label
bool ParseTitle(const string& html, string* title) {

    // 寻找<title>
    size_t begin = html.find("<title>");
    if(begin == string::npos) {
        std::cout <<"title is not found" << std::endl;
        return false;
    }
    begin += string("<title>").size(); // 跳过"<title>"的长度，指向标题第一个字符位置

    // 寻找</title>
    size_t end = html.find("</title>");
    if(end == string::npos) {
        std::cout <<"/title is not found" << std::endl;
        return false;
    }

    // 用begin和end的关系判断标题的合法性
    if(begin >= end) {
        std::cout << "Title is invalid" << std::endl;
        return false;
    }

    *title = html.substr(begin, end - begin);

    return true;
}

// 2. Get url
// local path: ../data/input/html/xxx.html 
// online path: https://www.boost.org/doc/libs/1_53_0/doc/html/xxx.html
bool ParseUrl(const string& file_path, string* url) {

    string url_head = "https://www.boost.org/doc/libs/1_53_0/doc";
    string url_tail = file_path.substr(g_input_path.size());
    *url = url_head + url_tail;
    return true;

}

// 3. Get content
// remove html label in html string 
bool ParseContent(const string& html, string* content) {

    bool is_content = true;
    for(auto e: html) {
        if(is_content == true) {
            if(e == '<') { // 标签开始
                is_content = false;
            }
            else {
                // 处理'\n'
                if(e == '\n') {
                    e = ' ';
                }
                // 普通字符写入doc_info的content中
                content->push_back(e);
                }
        }
        else {
            if(e == '>') { // 普通字符开始
                is_content = true;
            }
        }
    }
    return true;

}


void WriteOutput(const DocInfo& doc_info, std::ofstream& ofstream) {
    ofstream << doc_info.title << '\3' << doc_info.url << '\3' << doc_info.content << std::endl;
}

int main() {
  
    // 1. 枚举出所有html文件的url，用vector<string>来保存
    vector<string> file_list;
    bool ret = EnumFile(g_input_path, &file_list);
    if(!ret) { // 枚举路径失败退出
        std::cout << "Enumerate failing..." << std::endl;
        return 1;
    }
  

  // 2. 遍历file_list数组中的每个目录，对于每个html文件进行处理
  int ans = 0;
  std::ofstream output_file(g_output_path.c_str());
  if(!output_file.is_open()) {
    std::cout << "open output_file failing..." << std::endl;
  }
  for(auto file_path = file_list.begin();  file_path != file_list.end(); file_path++) {
    std::cout << *file_path << std::endl;
    ans++;

    DocInfo doc_info; // 创建一个DocInfo结构体来保存html文件的信息
    ret = ParseFile(*file_path, &doc_info); / /ParseFile函数用来将文件名为file_path的html文件信息解析到doc_info中
    if(!ret) {
      std::cout << "Parse file failing:" << *file_path << std::endl;
      continue;
    }

    // wirte dec_info into file
    WriteOutput(doc_info, output_file);
  }
  std::cout << "There are " << ans << " html files" << std::endl;
}

