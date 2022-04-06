#include "searcher.h"
#include <algorithm>
#define PROCESS_BAR_LENGTH 102
namespace searcher{

    const char* const DICT_PATH = "../../cppjieba/dict/jieba.dict.utf8";
    const char* const HMM_PATH = "../../cppjieba//dict/hmm_model.utf8";
    const char* const USER_DICT_PATH = "../../cppjieba/dict/user.dict.utf8";
    const char* const IDF_PATH = "../../cppjieba/dict/idf.utf8";
    const char* const STOP_WORD_PATH = "../../cppjieba/dict/stop_words.utf8";

    Index::Index() // Index类的构造函数
    : jieba(DICT_PATH, HMM_PATH, USER_DICT_PATH, IDF_PATH, STOP_WORD_PATH){}

    void ProcessBar(int64_t line_amount, int64_t doc_id) { // 进度条
        char pb[PROCESS_BAR_LENGTH] = {0};
        char state[4] = {'-', '\\', '|', '/'};
        pb[0] = '[';
        pb[PROCESS_BAR_LENGTH - 1] = ']';
        int64_t mark_amount = (doc_id + 2) * 100 / line_amount;
        for(int i = 1; i <= mark_amount; i++) {
            pb[i] = '#';
        }
        for(int i = 0; i < PROCESS_BAR_LENGTH; i++) {
            if(pb[i] != 0) {
                cout << pb[i];
            }
            else {
                cout << " ";
            }
        }
        cout << " [%" << mark_amount << "] [" << state[mark_amount % 4] << "]";
        if(mark_amount == 100) {
            sleep(1);
        }
        std::fflush(stdout);
        cout << '\r';
    }

    //////////////////////提供对外调用的API/////////////////////////////////
    const DocInfo* Index::GetDocInfo(int64_t doc_id) { // 传入doc_id返回DocInfo

        if(doc_id < 0 || doc_id > forward_index.size()) {
            return nullptr;
        }
        return &forward_index[doc_id];

    }
    const InvertedList* Index::GetInvertedList(const string& key) { // 传入关键字返回倒排拉链

        unordered_map<string, InvertedList>::iterator it = inverted_index.find(key);
        if(it == inverted_index.end()) {
            return nullptr;
        }
        return &it->second;

    }
    //////////////////////提供对外调用的API/////////////////////////////////

    /////////////////////构建正排、倒排索引/////////////////////////////////
    bool Index::Build(const string& input_path) {

        // 1. 读取raw_input文件内容
        std::cout << "Start Build Index..." << std::endl;
        std::ifstream file(input_path.c_str());
        if(!file.is_open()) {
            std::cout << "Read raw_input Failing..." << std::endl;
            return false;
        }

        // 2. 按行读取raw_input，每一行对应一个html文件
        //////////////////////统计行数///////////////////////////
        std::ifstream tmpfile(input_path.c_str());
        string tmpline;
        int64_t line_amount = 0;
        while(std::getline(tmpfile, tmpline)) {
            line_amount++;
        }
        //////////////////////统计行数///////////////////////////
        string line;
        while(std::getline(file, line)) {

            // 3. 将每一行转换成一个DocInfo结构体，构建正排索引
            DocInfo* doc_info = BuildForward(line);
            if(!doc_info) {
                std::cout << "Construct Failing..." << std::endl;
                continue;
            }
            // 4. 使用doc_info同时构建倒排索引
            BuildInverted(*doc_info);
            // print process bar
            ProcessBar(line_amount, (*doc_info).doc_id);

        }

        std::cout << "Finish Build Index..." << std::endl;
        file.close();
        return true;

    }
    DocInfo* Index::BuildForward(const string& line) { // 构建 文档ID->DocInfo的正排索引
        vector<string> tokens;
        // 1.将raw_input文件用 "\3" 切成 title, url, content 三部分，存入名为tokens的string数组中
        common::Util::Split(line, "\3", &tokens);
        if(tokens.size() != 3) {
            // 如果没有切成3份，则返回空指针表示结果出错
            return nullptr;
        }
        // 2. 把token中的string一一赋值给doc_info的成员
        DocInfo doc_info;
        doc_info.doc_id = forward_index.size();
        doc_info.title = tokens[0];
        doc_info.url = tokens[1];
        doc_info.content = tokens[2];
        forward_index.push_back(std::move(doc_info)); // 调用push_back(T&& val)移动构造，转移数据，减少拷贝，提高效率
        return &forward_index.back();
    }
    void Index::BuildInverted(const DocInfo& doc_info) { // 构建 关键字->倒排拉链 的倒排索引
        struct WordCut{ // 分别对于标题和正文进行分词，并且统计关键词的出现次数
            int title_cnt;
            int content_cnt;
            WordCut(): title_cnt(0), content_cnt(0) {}
        };
        unordered_map<string, WordCut> word_cut_map; // hash表存储关键字和综合权重(10*title出现 + 1*content出现)之间的关系
        // 1.针对标题进行分词
        vector<string> title_token;
        CutWord(doc_info.title, &title_token);
        // 2.遍历分词结果，统计每个词出现的次数
        for(string word: title_token) {
            boost::to_lower(word); // convert all character to lower
            word_cut_map[word].title_cnt++;
        }
        // 3.针对正文进行分词
        vector<string> content_token;
        CutWord(doc_info.content, &content_token);
        // 4.遍历分词结果，统计每个词出现的次数
        for(string word: content_token) {
            boost::to_lower(word);
            word_cut_map[word].content_cnt++;
        }
        //5.根据统计结果，构建出Weight对象，并整合到倒排拉链中
        for(const auto& word_pair: word_cut_map) {
            Weight weight;
            weight.doc_id = doc_info.doc_id;
            // weight = title_cnt * 10 + content_cnt * 1
            weight.weight = 10 * word_pair.second.title_cnt + word_pair.second.content_cnt;
            weight.word = word_pair.first;
            // 把整合出来Weight插入到对应关键字的倒排拉链中
            InvertedList& invert_list = inverted_index[word_pair.first]; // 关键字找到倒排拉链
            invert_list.push_back(weight); // 向倒排拉链尾插Weight结构体
        }
    }
    void Index::CutWord(const string& input, vector<string>* output) { // 分词
        jieba.CutForSearch(input, *output);
    }
    /////////////////////构建正排、倒排索引/////////////////////////////////

    //////////////////////////////Searcher模块//////////////////////////////////////////////////////////
    bool Searcher::Init(const string& input_path) {
        return index->Build(input_path);
    }
    bool Searcher::Search(const string& query, string* output) { // 搜索查询词，得到搜索结果
        // 1. [分词]针对查询词分词
        vector<string> tokens;
        index->CutWord(query, &tokens);
        // 2. [触发]根据分词结果，查倒排，把相关文档都获取到
        vector<Weight> all_token_result;
        for(string word : tokens) {
            boost::to_lower(word); // 把查询词统一转到小写
            auto* inverted_list = index->GetInvertedList(word); // 尝试获取查询词的倒排拉链
            if(!inverted_list) { // 无法获取查询词的倒排拉链，说明该查询词不在倒排索引中
                continue;
            }
            // tokens包含多个结果，我们要把它们合并到一起，才能进行统一的排序
            all_token_result.insert(all_token_result.end(), inverted_list->begin(), inverted_list->end());
        }
        // 3. [排序]把刚才查到的这些文档的倒排拉链合并到一起并按照权重进行降序排序
        std::sort(all_token_result.begin(), all_token_result.end(), [](const Weight& w1, const Weight& w2){return w1.weight > w2.weight;});
        // 4. [包装结果] 把得到的倒排拉链中的文档ID获取到，去查正排，再把doc_info中的内容构造成最终预期的格式
        Json::Value results; // 这个results包含了若干个搜索结果，每个搜索结果是一个JSON对象
        for(const auto& weight: all_token_result) {
            const DocInfo* doc_info = index->GetDocInfo(weight.doc_id);
            Json::Value result;
            result["title"] = doc_info->title;
            result["url"] = doc_info->url;
            result["description"] = GenerateDescription(doc_info->content, weight.word);
            results.append(result);
        }
        // 把JSON对象序列化成字符串，写入output中
        Json::FastWriter writer;
        *output = writer.write(results);
        return true;
    }
    string Searcher::GenerateDescription(const string& content, const string& word) {
        // 根据正文，找到word出现的位置
        // 以该位置为中心，往前找60个字节，作为描述的起始位置
        // 再从起始位置往后找160个字节，作为描述的终止位置
        // 如果前面不够60字节，就从0开始
        // 总共不足160个字节，就到末尾结束
        // 如果后面内容表示不下，用...表示

        size_t first_pos = content.find(word);
        size_t begin = 0;
        if(first_pos == string::npos) {
            // 1. 该关键字只在标题中出现，没有在正文中出现，以开头为起始位置
            if(content.size() < 160) {
                return content;
            }
            string description =  content.substr(0, 160);
            description[description.size() - 1] = '.';
            description[description.size() - 2] = '.';
            description[description.size() - 3] = '.';
            return content.substr(0, 160);
        }
        // 2. 正文中找到了关键词，以这个位置为基准，向前找一些字节
        begin = first_pos < 60 ? 0 : first_pos - 60;
        if(begin + 160 > content.size()) {
            return content.substr(begin);
        }
        else {
            string description =  content.substr(begin, 160);
            description[description.size() - 1] = '.';
            description[description.size() - 2] = '.';
            description[description.size() - 3] = '.';
            return description;
        }

    }
    //////////////////////////////////////////////////////////////////////////////////////////////////


} // namespace end
