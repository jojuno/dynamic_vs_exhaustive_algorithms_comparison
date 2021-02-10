#include "maxdefense.hh"
#include "timer.hh"

int main()
{
	//n = 5
	auto all_armors = load_armor_database("armor.csv");
	assert( all_armors );
	Timer timer;
	auto filtered_armors_1 = filter_armor_vector(*all_armors, 1, 2500, 5);
	std::unique_ptr<ArmorVector> soln_dynamic_1 = dynamic_max_defense(*filtered_armors_1, 2500);
	std::cout << "time elapsed for dynamic solution 1: " << timer.elapsed() << std::endl;
	timer.reset();
	std::unique_ptr<ArmorVector> soln_exhaustive_1 = exhaustive_max_defense(*filtered_armors_1, 2500);
	std::cout << "time elapsed for exhaustive solution 1: " << timer.elapsed() << std::endl;
	timer.reset();

	//n = 10
	auto filtered_armors_2 = filter_armor_vector(*all_armors, 1, 2500, 10);
	std::unique_ptr<ArmorVector> soln_dynamic_2 = dynamic_max_defense(*filtered_armors_2, 5000);
	std::cout << "time elapsed for dynamic solution: " << timer.elapsed() << std::endl;
	timer.reset();
	std::unique_ptr<ArmorVector> soln_exhaustive_2 = exhaustive_max_defense(*filtered_armors_2, 5000);
	std::cout << "time elapsed for exhaustive solution 2: " << timer.elapsed() << std::endl;
	timer.reset();

	//n = 15
	auto filtered_armors_3 = filter_armor_vector(*all_armors, 1, 2500, 15);
	std::unique_ptr<ArmorVector> soln_dynamic_3 = dynamic_max_defense(*filtered_armors_3, 7500);
	std::cout << "time elapsed for dynamic solution 3: " << timer.elapsed() << std::endl;
	timer.reset();
	std::unique_ptr<ArmorVector> soln_exhaustive_3 = exhaustive_max_defense(*filtered_armors_3, 7500);
	std::cout << "time elapsed for exhaustive solution 3: " << timer.elapsed() << std::endl;
	timer.reset();

	//n = 20
	//fixed from 24 because the process would get killed
	auto filtered_armors_4 = filter_armor_vector(*all_armors, 1, 2500, 20);
	std::unique_ptr<ArmorVector> soln_dynamic_4 = dynamic_max_defense(*filtered_armors_4, 10000);
	std::cout << "time elapsed for dynamic solution 4: " << timer.elapsed() << std::endl;
	timer.reset();
	std::unique_ptr<ArmorVector> soln_exhaustive_4 = exhaustive_max_defense(*filtered_armors_4, 10000);
	std::cout << "time elapsed for exhaustive solution 4: " << timer.elapsed() << std::endl;
	timer.reset();
	return 0;
}











































