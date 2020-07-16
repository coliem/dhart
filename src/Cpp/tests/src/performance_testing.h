/*!
	\brief Some inline functions to make performance testing easier
*/

#include <string>
#include <chrono>

using namespace std::chrono;
using std::string;

using this_clock = std::chrono::high_resolution_clock; //< Type of clock to use
using timepoint = this_clock::time_point;
using hf_time_units = std::chrono::milliseconds; //< Units to use
static const string hf_time_unit_postfix = "ms"; //< shorthand for units I.E. milliseconds = ms

/*
	\brief Utility class to simplify recording execution time.
*/
struct StopWatch {

	timepoint start{};
	timepoint end{};

	/*! \brief Create a new clock and start it if auto_start is true.*/
	inline StopWatch::StopWatch(bool auto_start = true) {
		if (auto_start)
			StartClock();
	}

	/*! \brief Sets the start time of this trial to the current time.*/
	inline void StartClock() {
		start = this_clock::now();
	}

	/*! \brief Get the time from the start to now in milliseconds.*/
	inline long long StartToNow() const {
		return duration_cast<std::chrono::milliseconds>(this_clock::now() - start).count();
	}

	/*! \brief Set the end time of this stopwatch. */
	inline void StopClock() {
		end = this_clock::now();
	}

	/*! \brief Get the duration from start to now. */
	inline long long GetDuration() const {
		return duration_cast<hf_time_units>(end - start).count();
	}
};


/*! 
	\brief A function to uniformly the results of multiple trials to std::cerr.

	\param watches StopWatches containing ths start/end times to print.
	\param num_units number of units were processed by each trial in order.
	\param unit_name Name of units being processed. I.E. the graph generator would put Nodes.
*/
inline void PrintTrials(
	const std::vector<StopWatch> & watches,
	const std::vector<int> num_units,
	std::string unit_name = "Elements"
) {
	// Iterate through the results of each trial.
	for (int i = 0; i < watches.size(); i++) {
		auto num_elements = num_units[i];
		auto& this_trial = watches[i];
		auto duration = this_trial.GetDuration();

		float units_per_ms = static_cast<float>(num_elements) / static_cast<float>(duration);

		// Print results.
		std::cerr << "Trial " << i << " | "
			<< "Count of " << unit_name << ": " << num_elements << ", "
			<< "Time: " << duration << hf_time_unit_postfix << ", "
			<< unit_name << " / ms: " << units_per_ms
			<< std::endl;
	}
}
