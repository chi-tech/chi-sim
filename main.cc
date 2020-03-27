#include <iostream>
#include <stdexcept>

#include "ChiSim/chi_sim.h"

ChiSim ChiSim::m_instance;

int main()
{
    auto& chi_sim = ChiSim::GetSystemScope();

    try 
    {
      chi_sim.Initialize();
      chi_sim.ExecuteRuntime();
      chi_sim.Finalize();
    }
    catch (const std::exception& e) 
    {
      std::cerr << e.what() << std::endl;
      exit(EXIT_FAILURE);
    }
    

    return 0;
}