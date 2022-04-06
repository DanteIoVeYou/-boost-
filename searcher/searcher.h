#include <iostream>                                           
#include <stdint.h>
#include <unistd.h>
#include <string>
#include <vector>                                  
#include <unordered_map>      
#include <algorithm>
#include <boost/filesystem/path.hpp>                        
#include <boost/filesystem/operations.hpp>
#include <jsoncpp/json/json.h>
#include "../../cppjieba/include/cppjieba/Jieba.hpp"                                                                       
#include "../common/util.hpp"
                                                     
using std::cout;
using std::cin;
using std::endl;                                                   
using std::string;           
using std::vector;
using std::unordered_map; 


namespace searcher{

////////////////////////////////////////////////////////////////////////////////////////// 
// Index model
////////////////////////////////////////////////////////////////////////////////////////// 
// 正派索引的核心结构体
  struct DocInfo {
    int64_t doc_id;
    string title;
    string url;
    string content;
  };
// 倒排索引的核心结构体
  struct Weight {
   int64_t doc_id; // 文档id
   int weight; // 权重
   string word; // 关键词
  };

  typedef vector<Weight> InvertedList;
  // Index 类用来进行索引的构建
  class Index {

    private:
      vector<DocInfo> forward_index; // 正排索引->DocInfo的数组
      unordered_map<string, InvertedList> inverted_index; // 倒排索引->用关键字映射倒排拉链的hash表
    public:
      Index();
      // 1. 查正排索引的方法，返回一个DocInfo指针，用NULL表示无效结果
      const DocInfo* GetDocInfo(int64_t doc_id);
      // 2. 查倒排索引的方法
      const InvertedList* GetInvertedList(const string& key);
      // 3. 构建索引
      bool Build(const string& input_path);
      // 4. 分词
      void CutWord(const string& input, vector<string>* output);
    private:
      DocInfo* BuildForward(const string& line);
      void BuildInverted(const DocInfo& doc_info);
      cppjieba::Jieba jieba;
  };







////////////////////////////////////////////////////////////////////////////////////////// 
// Searcher model
////////////////////////////////////////////////////////////////////////////////////////// 
  class Searcher {
    private:
      Index* index;

    public:
      Searcher() : index(new Index()){}
      bool Init(const string& input_path);
      bool Search(const string& query, string* output);
    private:
      string GenerateDescription(const string& content, const string& word);
  };
} // namesapce end
