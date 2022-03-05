#include "graph.h"
#include "graph.hpp"
#include "grower.h"

constexpr int K = 2;
constexpr int N = 7;

std::string get_log_file_name(const char* exe_path) {
  std::string path(exe_path);
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
  std::cout << "Log file path: " << log << "\n";

  std::ofstream log_stream(log);
  Counters::initialize(&log_stream);

  Grower<K, N> s(&log_stream);
  s.grow();
  s.print(false);

  Counters::print_counters();
  log_stream.flush();
  log_stream.close();
  return 0;
}
