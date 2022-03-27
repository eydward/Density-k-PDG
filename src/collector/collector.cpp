// When running complex kPDG computations (e.g. `kPDG 4 7`), we can separate
// the computation to many smaller chunks. Each one will produce 3 log files.
// This program collects all log files in the same directory, checks for consistency,
// and produces the final result.

#include "../graph.h"

namespace fs = std::filesystem;
bool already_set_k_n = false;  // If true, the K and N values are already set.

// Data retrieved from summary log: (start, end, min_theta, cpu_seconds)
std::vector<std::tuple<int, int, Fraction, int>> summary_data;
// Data retrieved from result log: key=base graph id, value=(base_graph, min_theta, min_theta_graph)
std::map<int, std::tuple<std::string, Fraction, std::string>> results;
// The base graph list from all the detail logs, used for checking consistency.
std::vector<std::string> detail_base_graph_list;
int base_graph_count = 0;

void print_usage() {
  std::cout << "Usage: collector dir\n"
            << "  Where dir specifies the directory that contain all log files.\n";
}

void trim(std::string& s) {
  s.erase(s.begin(),
          std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
}
bool parse_name(const fs::directory_entry& item, int& threads, int& start, int& end) {
  int k, n;
  int params = std::sscanf(item.path().stem().string().c_str(), "kPDG_v9_%d_%d_%d_%d_%d_", &k, &n,
                           &threads, &start, &end);
  if (params < 5) {
    std::cout << "WARNING: unrecognized log file: " << item << "\n";
    return false;
  }
  if (k < 2 || k > 7 || k > n || n > 7) {
    std::cout << "ERROR: unrecognized K, N values: " << item << "\n";
    exit(-1);
  }
  if (!already_set_k_n) {
    Graph::set_global_graph_info(k, n);
  } else {
    if (k != Graph::K || n != Graph::N) {
      std::cout << "ERROR: different K, N values encountered: " << item << "\n";
      exit(-1);
    }
  }
  return true;
}
void read_detail_log(const fs::directory_entry& item) {
  std::cout << "Reading detail log " << item << "\n";
  int threads, start, end;
  if (!parse_name(item, threads, start, end)) return;

  std::ifstream is(item.path());
  int num_base_graphs = 0;
  while (!is.eof()) {
    std::string line;
    std::getline(is, line);

    int params = std::sscanf(line.c_str(), "-------- Accumulated canonicals[order=6] : %d",
                             &num_base_graphs);
    if (params > 0) {
      break;
    }
  }

  // Found the starting of base graph section, parse the section.
  std::vector<std::string> graphs;
  while (!is.eof()) {
    std::string line;
    std::getline(is, line);
    if (line.length() > 4 && line.substr(0, 4) == "----") {
      break;
    }

    int id = 0;
    int params = std::sscanf(line.c_str(), "  [%d]", &id);
    if (params != 1) {
      std::cout << "ERROR: unexpected content in detail log: " << line << "\n";
      exit(-1);
    }
    size_t pos = line.find(']');
    std::string graph = line.substr(pos + 1);
    trim(graph);
    if (graphs.size() != id) {
      std::cout << "ERROR: graph id mismatch: " << graphs.size() << " != " << line << "\n";
      exit(-1);
    }
    graphs.push_back(graph);
  }
  std::cout << "Done Reading detail log " << item << "\n";

  // Verify total count
  if (num_base_graphs != graphs.size()) {
    std::cout << "ERROR: graph count mismatch: " << num_base_graphs << " != " << graphs.size()
              << "\n";
    exit(-1);
  }
  // Verify consistent with other detail logs.
  if (base_graph_count == 0) {
    base_graph_count = num_base_graphs;
    detail_base_graph_list = graphs;
  } else {
    if (base_graph_count != num_base_graphs) {
      std::cout << "ERROR: graph count mismatch in detail logs: " << base_graph_count
                << " != " << num_base_graphs << "\n";
      exit(-1);
    }
    for (size_t i = 0; i < graphs.size(); i++) {
      if (detail_base_graph_list[i] != graphs[i]) {
        std::cout << "ERROR: graph content mismatch in detail logs [" << i
                  << "]: " << detail_base_graph_list[i] << " != " << graphs[i] << "\n";
        exit(-1);
      }
    }
  }

  is.close();
}

void assert_not_eof(const std::ifstream& is) {
  if (is.eof()) {
    std::cout << "ERROR: unexpected EOF\n";
    exit(-1);
  }
}

void read_result_log(const fs::directory_entry& item) {
  std::cout << "Reading result log " << item << "\n";
  int threads, start, end;
  if (!parse_name(item, threads, start, end)) return;

  std::ifstream is(item.path());
  while (!is.eof()) {
    std::string line;
    std::getline(is, line);
    int base_graph_id = 0, min_theta_n = 0, min_theta_d = 0;

    int params = std::sscanf(line.c_str(), "G[%d] min_theta=%d / %d", &base_graph_id, &min_theta_n,
                             &min_theta_d);
    if (params == 3) {
      Fraction min_theta(min_theta_n, min_theta_d);

      std::string base_graph, min_theta_graph;
      assert_not_eof(is);
      std::getline(is, base_graph);
      trim(base_graph);
      assert_not_eof(is);
      std::getline(is, min_theta_graph);
      trim(min_theta_graph);

      if (results.find(base_graph_id) != results.cend()) {
        const auto& existing = results.at(base_graph_id);
        if (std::get<0>(existing) != base_graph || std::get<1>(existing) != min_theta) {
          std::cout << "ERROR: results mismatch. Existing[" << base_graph_id
                    << "]: " << std::get<1>(existing).n << "/" << std::get<1>(existing).d << "\n"
                    << std::get<0>(existing) << "\n  " << std::get<2>(existing)
                    << "\nFound: " << min_theta.n << "/" << min_theta.d << "\n  " << base_graph
                    << "\n  " << min_theta_graph << "\n";
          exit(-1);
        }
      }
      results.emplace(base_graph_id, std::make_tuple(base_graph, min_theta, min_theta_graph));
    }
  }

  is.close();
}
void read_summary_log(const fs::directory_entry& item) {
  std::cout << "Reading summary log " << item << "\n";
  int threads, start, end;
  if (!parse_name(item, threads, start, end)) return;

  uint64 run_time = 0;
  Fraction theta(0, 1);

  std::ifstream is(item.path());
  while (!is.eof()) {
    std::string line;
    std::getline(is, line);
    uint64 time_high = 0, time_low = 0;

    // Parse time
    int params =
        std::sscanf(line.c_str(), "--------Wall clock time:  %d`%dms", &time_high, &time_low);
    if (params < 2) {
      time_high = 0;
      params = std::sscanf(line.c_str(), "--------Wall clock time:  %dms", &time_low);
    }
    if (params > 0) {
      uint64 time = time_high * 1000000 + time_low;
      if (run_time < time) {
        run_time = time;
      }
    }

    // Parse min_theta
    int theta_n = 0, theta_d = 0;
    params = std::sscanf(line.c_str(), "    Current minimum theta = %d / %d", &theta_n, &theta_d);
    if (params == 2) {
      theta.n = theta_n;
      theta.d = theta_d;
    }
  }
  is.close();
  // Add to summary.
  summary_data.push_back(std::make_tuple(start, end, theta, run_time * threads / 1000));
}

int main(int argc, char* argv[]) {
  if (argc != 2 && argc != 6) {
    print_usage();
    return -1;
  }
  fs::path dir(argv[1]);
  if (!fs::is_directory(dir)) {
    std::cout << "Error: the given log directory does not exist.\n";
    print_usage();
    return -1;
  }

  std::cout << "Reading log files...\n";
  for (const auto& item : fs::directory_iterator(dir)) {
    if (!item.is_regular_file()) continue;
    if (item.path().extension() != ".log") continue;
    std::string stem = item.path().stem().string();
    if (stem.length() > 7 && stem.substr(stem.length() - 6, 6) == "detail") {
      read_detail_log(item);
    } else if (stem.length() > 7 && stem.substr(stem.length() - 6, 6) == "result") {
      read_result_log(item);
    } else {
      read_summary_log(item);
    }
  }

  std::cout << "Finished reading log files.\n";
  std::sort(summary_data.begin(), summary_data.end());
  std::cout << "SUMMARY:\n"
            << "\tStart\tEnd\tmin_theta\tCPU seconds\n"
            << "\t------\t------\t------\t\t----------\n";

  Fraction min_theta(1E8, 1);
  uint64 total_cpu_seconds = 0;
  for (const auto& summary : summary_data) {
    std::cout << "\t" << std::get<0>(summary) << "\t" << std::get<1>(summary) << "\t"
              << std::get<2>(summary).n << "/" << std::get<2>(summary).d << "\t\t"
              << std::get<3>(summary) << "\n";
    if (min_theta > std::get<2>(summary)) {
      min_theta = std::get<2>(summary);
    }
    total_cpu_seconds += std::get<3>(summary);
  }
  std::cout << "\t------\t------\t------\t----------\n"
            << "Global min_theta = " << min_theta.n << "/" << min_theta.d << "\n"
            << "Total CPU seconds = " << total_cpu_seconds << "  (" << total_cpu_seconds / 3600
            << " CPU hours)\n";

  std::cout << "\n\nValidating detail log with result log...\n";
  std::vector<bool> covered(base_graph_count);
  Fraction result_min_theta(1E8, 1);
  for (const auto& kv : results) {
    int graph_id = kv.first;
    const std::string& base_graph = std::get<0>(kv.second);
    Fraction theta = std::get<1>(kv.second);
    if (base_graph != detail_base_graph_list[graph_id]) {
      std::cout << "ERROR: base graph mismatch between detail log and result log:\n  "
                << detail_base_graph_list[graph_id] << "\n  " << base_graph << "\n";
      exit(-1);
    }
    covered[graph_id] = true;
    if (result_min_theta > theta) {
      result_min_theta = theta;
    }
  }

  std::cout << "From result log min_theta = " << result_min_theta.n << "/" << result_min_theta.d
            << "\n";
  // Check if there are gaps
  int gap_start = -1, gap_end = -1;
  bool gap_exists = false;
  for (int i = 0; i < base_graph_count; i++) {
    if (!covered[i]) {
      gap_start = i;
      gap_end = base_graph_count;
      while (i < base_graph_count && !covered[i]) {
        gap_end = i;
        ++i;
      }
      std::cout << "WARNING: found gap between " << gap_start << " and " << gap_end << "\n";
      gap_exists = true;
    }
  }

  if (gap_exists) {
    std::cout << "\nPartial result due to gaps.\n";
  } else {
    std::cout << "\nAll results are verified for consistency. Success.\n";
  }
  return 0;
}