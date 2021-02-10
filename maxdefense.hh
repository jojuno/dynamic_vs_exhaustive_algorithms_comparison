///////////////////////////////////////////////////////////////////////////////
// maxdefense.hh
//
// Compute the set of armos that maximizes defense, within a gold budget,
// with the dynamic method or exhaustive search.
//
///////////////////////////////////////////////////////////////////////////////


#pragma once


#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <memory>
#include <queue>
#include <sstream>
#include <string>
#include <vector>


// One armor item available for purchase.
class ArmorItem
{
	//
	public:
		
		//
		ArmorItem
		(
			const std::string& description,
			size_t cost_gold,
			double defense_points
		)
			:
			_description(description),
			_cost_gold(cost_gold),
			_defense_points(defense_points)
		{
			assert(!description.empty());
			assert(cost_gold > 0);
		}
		
		//
		const std::string& description() const { return _description; }
		int cost() const { return _cost_gold; }
		double defense() const { return _defense_points; }
	
	//
	private:
		
		// Human-readable description of the armor, e.g. "new enchanted helmet". Must be non-empty.
		std::string _description;
		
		// Cost, in units of gold; Must be positive
		int _cost_gold;
		
		// Defense points; most be non-negative.
		double _defense_points;
};


// Alias for a vector of shared pointers to ArmorItem objects.
typedef std::vector<std::shared_ptr<ArmorItem>> ArmorVector;


// Load all the valid armor items from the CSV database
// Armor items that are missing fields, or have invalid values, are skipped.
// Returns nullptr on I/O error.
std::unique_ptr<ArmorVector> load_armor_database(const std::string& path)
{
	std::unique_ptr<ArmorVector> failure(nullptr);
	
	std::ifstream f(path);
	if (!f)
	{
		std::cout << "Failed to load armor database; Cannot open file: " << path << std::endl;
		return failure;
	}
	
	std::unique_ptr<ArmorVector> result(new ArmorVector);
	
	size_t line_number = 0;
	for (std::string line; std::getline(f, line); )
	{
		line_number++;
		
		// First line is a header row
		if ( line_number == 1 )
		{
			continue;
		}
		
		std::vector<std::string> fields;
		std::stringstream ss(line);
		
		for (std::string field; std::getline(ss, field, '^'); )
		{
			fields.push_back(field);
		}
		
		if (fields.size() != 3)
		{
			std::cout
				<< "Failed to load armor database: Invalid field count at line " << line_number << "; Want 3 but got " << fields.size() << std::endl
				<< "Line: " << line << std::endl
				;
			return failure;
		}
		
		std::string
			descr_field = fields[0],
			cost_gold_field = fields[1],
			defense_points_field = fields[2]
			;
		
		auto parse_dbl = [](const std::string& field, double& output)
		{
			std::stringstream ss(field);
			if ( ! ss )
			{
				return false;
			}
			
			ss >> output;
			
			return true;
		};
		
		std::string description(descr_field);
		double cost_gold, defense_points;
		if (
			parse_dbl(cost_gold_field, cost_gold)
			&& parse_dbl(defense_points_field, defense_points)
		)
		{
			result->push_back(
				std::shared_ptr<ArmorItem>(
					new ArmorItem(
						description,
						cost_gold,
						defense_points
					)
				)
			);
		}
	}

	f.close();
	
	return result;
}


// Convenience function to compute the total cost and defense in an ArmorVector.
// Provide the ArmorVector as the first argument
// The next two arguments will return the cost and defense back to the caller.
void sum_armor_vector
(
	const ArmorVector& armors,
	int& total_cost,
	double& total_defense
)
{
	total_cost = total_defense = 0;
	for (auto& armor : armors)
	{
		total_cost += armor->cost();
		total_defense += armor->defense();
	}
}


// Convenience function to print out each ArmorItem in an ArmorVector,
// followed by the total kilocalories and protein in it.
void print_armor_vector(const ArmorVector& armors)
{
	std::cout << "*** Armor Vector ***" << std::endl;
	
	if ( armors.size() == 0 )
	{
		std::cout << "[empty armor list]" << std::endl;
	}
	else
	{
		for (auto& armor : armors)
		{
			std::cout
				<< "Ye olde " << armor->description()
				<< " ==> "
				<< "Cost of " << armor->cost() << " gold"
				<< "; Defense points = " << armor->defense()
				<< std::endl
				;
		}
		
		int total_cost;
		double total_defense;
		sum_armor_vector(armors, total_cost, total_defense);
		std::cout
			<< "> Grand total cost: " << total_cost << " gold" << std::endl
			<< "> Grand total defense: " << total_defense
			<< std::endl
			;
	}
}


// Convenience function to print out a 2D cache, composed of an std::vector<std::vector<double>>
// For sanity, will refuse to print a cache that is too large.
// Hint: When running this program, you can redirect stdout to a file,
//	which may be easier to view and inspect than a terminal
void print_2d_cache(const std::vector<std::vector<double>>& cache)
{
	std::cout << "*** 2D Cache ***" << std::endl;
	
	if ( cache.size() == 0 )
	{
		std::cout << "[empty]" << std::endl;
	}
	else if ( cache.size() > 250 || cache[1].size() > 250 )
	{
		std::cout << "[too large]" << std::endl;
	}
	else
	{
		for ( const std::vector<double> row : cache)
		{
			for ( double value : row )
			{
				std::cout << std::setw(5) << value;
			}
			std::cout << std::endl;
		}
	}
}

// Filter the vector source, i.e. create and return a new ArmorVector
// containing the subset of the armor items in source that match given
// criteria.
// This is intended to:
//	1) filter out armor with zero or negative defense that are irrelevant to our optimization
//	2) limit the size of inputs to the exhaustive search algorithm since it will probably be slow.
//
// Each armor item that is included must have at minimum min_defense and at most max_defense.
//	(i.e., each included armor item's defense must be between min_defense and max_defense (inclusive).
//
// In addition, the the vector includes only the first total_size armor items that match these criteria.
std::unique_ptr<ArmorVector> filter_armor_vector
(
	const ArmorVector& source,
	double min_defense,
	double max_defense,
	int total_size
)
{
	std::unique_ptr<ArmorVector> filtered_source(new ArmorVector);
	for (auto& armor : source)
	{
		if (max_defense > armor->defense() && armor->defense() > min_defense)
			filtered_source->push_back(armor);
		if (filtered_source->size() == unsigned(total_size))
			break;
	}
	return filtered_source;
}


// Compute the optimal set of armor items with a dynamic algorithm.
// Specifically, among the armor items that fit within a total_cost gold budget,
// choose the selection of armors whose defense is greatest.
// Repeat until no more armor items can be chosen, either because we've run out of armor items,
// or run out of gold.
std::unique_ptr<ArmorVector> dynamic_max_defense
(
	const ArmorVector& armors,
	int total_cost
)
{
	//2 + (n+1)((W+1)(4 + 3 + 13))
	//= 20*n*W + 20*n + 20*W + 22
	//make copy of armors
	std::unique_ptr<ArmorVector> todo(new ArmorVector(armors));
	//armor vector with resulting items
	std::unique_ptr<ArmorVector> result(new ArmorVector);
	//build 2d cache
	using matrix = std::vector<std::vector<double>>;
	//squares go from 0 to number of armors
	//move up index of armors by 1 to make space for 0 row on top
	matrix cache(todo->size()+1, std::vector<double>(total_cost+1)); //2
	//rows = specific items 
	//columns = budget
	//values = defense
	
	for (unsigned armorIndex = 0; armorIndex <= todo->size(); armorIndex++) { //n + 1
		for (double gold = 0; gold <= total_cost; gold++) { //W + 1
			//zero row and column
			if (armorIndex == 0 or gold == 0) { //3
				cache[armorIndex][gold] = 0; //1
				continue;
			}
			//if neither, because armor index starts at 0, pull it back by 1
			auto armor = todo->at(armorIndex-1);	//3
			//one square up
			//vs. one square up, (weight) squares to the left
			//take their values and compare them
			//take the greater one
			double value_one_above = cache[armorIndex-1][gold]; //1
			double value_one_above_and_weight_to_the_left = 0; //1
			double value_one_above_and_weight_to_the_left_plus_item_value = 0; //1
			if (gold-armor->cost() > -1) { //3
				value_one_above_and_weight_to_the_left = cache[armorIndex-1][gold-armor->cost()]; //4
				value_one_above_and_weight_to_the_left_plus_item_value = value_one_above_and_weight_to_the_left + armor->defense(); //3
			}
			if (value_one_above >= value_one_above_and_weight_to_the_left_plus_item_value) { //1
				cache[armorIndex][gold] = value_one_above; //1
			} else {
				cache[armorIndex][gold] = value_one_above_and_weight_to_the_left_plus_item_value; //1
			}
		}
	}

	//1 + (n)((W)(21)
	//= 21*n*W + 1
	//reconstruct armor vector from bottom right value
	int currentGold = total_cost; //1
	for (int cacheIndex = todo->size(); cacheIndex > 0; cacheIndex--) { //n
		for (int gold = currentGold; gold > 0; gold--) { //W
			if (cacheIndex-1 < 0) { //2
				continue;
			}
			auto armor = todo->at(cacheIndex-1); //3
			double value = cache[cacheIndex][gold]; //1
			double value_one_above = cache[cacheIndex-1][gold]; //2
			double value_one_above_and_weight_to_the_left = 0; //1
			if (gold - armor->cost() > -1) { //3
				value_one_above_and_weight_to_the_left = cache[cacheIndex-1][gold-armor->cost()]; //4
			}
			//if value is 0,
			//	continue
			if (value == 0) { //1
				continue;
			}
			//if value one above and value one above and weight to the left are the same,
			else if (value_one_above == value_one_above_and_weight_to_the_left) { //1
			//	pick the value one above and weight to the left
			//	that means set the gold to the left by armor's gold,
				currentGold = gold - armor->cost(); //3
			//	add the current armor to the solution,
				result->push_back(armor); //1
			//	and break
				break;
			//elseif value equals value one above,
			} else if (value == value_one_above) { //1
			//	pick the value one above
			//	that means break
				break;
			//else
			} else {
			//	pick the value one above and weight to the left
			//	that means set the gold to the left by armor's gold,
				currentGold = gold - armor->cost(); //3
			//	add the current armor to the solution,
				result->push_back(armor); //1
			//	and break
				break;
			}
		}
	}
	return result;
}


// Compute the optimal set of armor items with an exhaustive search algorithm.
// Specifically, among all subsets of armor items,
// return the subset whose gold cost fits within the total_cost budget,
// and whose total defense is greatest.
// To avoid overflow, the size of the armor items vector must be less than 64.
std::unique_ptr<ArmorVector> exhaustive_max_defense
(
	const ArmorVector& armors,
	double total_cost
)
{
	const int n = armors.size();
	assert(n < 64);

	std::unique_ptr<ArmorVector> best(new ArmorVector);
	for (size_t i = 0; i < pow(2, n); i++)
	{
		std::unique_ptr<ArmorVector> candidate(new ArmorVector);

		for (int j = 0; j < n; j++)
		{
			if (((i >> j) & 1) == 1)
			{
				candidate->push_back(armors.at(j));
			}
		}
		int* total_cost_candidate = new int();
		double* total_defense_candidate = new double();
		sum_armor_vector(*candidate, *total_cost_candidate, *total_defense_candidate);
		int* total_cost_best = new int();
		double* total_defense_best = new double();
		sum_armor_vector(*best, *total_cost_best, *total_defense_best);

		if (*total_cost_candidate <= total_cost)
		{
			if (best->empty() or *total_defense_candidate > *total_defense_best)
			{
				*best = *candidate;
			}
		}
	}

	return best;
}









