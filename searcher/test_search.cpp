#include "searcher.h"

int main() {
  searcher::Searcher searcher;
  bool ret = searcher.Init("../data/tmp/raw_input");

  if(!ret) {
    std::cout << "Search fail..." <<std::endl;
    return 1;
  }
  while(true) {
    std::cout << "searcher>" << std::flush;
    string query;
    std::cin >> query;
    if(!std::cin.good()) {
      std::cout << "goodbye" << std::endl;
      break;
    }
    string results;
    searcher.Search(query, &results);
    std::cout << results <<std::endl;
  }
}
