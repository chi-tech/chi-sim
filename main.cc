#include "ChiSim/chi_sim.h"
#include <stdexcept>

ChiSim ChiSim::m_instance;

int main() {
  auto& app = ChiSim::GetSystemScope();

  try {
    app.Execute();
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}