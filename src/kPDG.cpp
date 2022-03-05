#include "graph.h"
#include "graph.hpp"
#include "grower.h"

string get_log_file_name(const char* exe_path) {
  string path(exe_path);
  auto pos = path.rfind('/');
  if (pos == path.npos) {
    pos = path.rfind('\\');
  }
  if (pos != path.npos) {
    path = path.substr(pos + 1);
  }
  return path + ".log";
}

int main(int argc, char* argv[]) {
  std::filesystem::path log = std::filesystem::current_path();
  log.append(get_log_file_name(argv[0]));
  cout << "Log file path: " << log << "\n";

  ofstream log_stream(log);
  Counters::initialize(&log_stream);

  Grower<6, 6> s(&log_stream);
  s.grow();
  s.print(false);

  Counters::print_counters();
  log_stream.flush();
  log_stream.close();
  return 0;
}
