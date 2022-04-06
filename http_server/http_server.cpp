#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

#include "../cpp-httplib/httplib.h"
#include "../searcher/searcher.h"
using std::cout;
using std::cin;
using std::endl;
using std::string;
using std::vector;
using std::unordered_map;

int main() {
  
  using namespace httplib;
  // 1.Create Searcher Object
  
  searcher::Searcher searcher;
  bool ret = searcher.Init("../data/tmp/raw_input");
  if(!ret) {
    std::cout << "searcher init fail" << std::endl;
    return 1;
  }


  // 2.Create Server Object
  Server server;
  server.Get("/searcher", [&searcher](const Request& req, Response& resp){

      (void)req;
      if(!req.has_param("query")) {
        resp.set_content("Invalid Paramenter", "text/plain; charset=utf-8");
      }
      string query = req.get_param_value("query");
      string results;
      searcher.Search(query, &results);
      resp.set_content(results, "application/json; charset=utf-8");
  });
  // set static resources path
  server.set_base_dir("./wwwroot");
  // 3.Start Server 
  server.listen("0.0.0.0", 10002);
  return 0;
}

