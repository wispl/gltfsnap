#pragma once

#include <cstdint>
#include <functional>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

// TODO: switch to strings instead of enums? Convenient, but typos...

/// Input
///
/// Configuration:
///
/// This abstracts raw inputs into some game or user defined term. Oftentimes,
/// the input consumer does not need or want to know that 'w' was pressed,
/// rather, they want to know if a button to MOVE_FORWARD was pressed. Besides
/// being easier to reason about, it also allows for sane remapping.
///
/// An `Action` represents this idea, representing a mapping between a raw
/// input to some user-defined code, likely a enum.
///
/// The `Range` is similar to an `Action` but is meant to handle inputs which
/// can take a range instead of a simple boolean. Examples are controller axis
/// or the cursor. These values are frequently normalized or require additional
/// context such as the sensitivity or minimum and maximum input for mapping.
/// In other words, a `Range` is an `Action` with extra context.
///
/// An `ActionSet` is a set of `Action`s and `Range`s. By grouping them sets of
/// actions and ranges together, it is possible at runtime to enable different
/// mapping configurations, such as different mappings in a menu vs during gameplay.
/// The enabled `ActionSet`, then, is responsible for matching the raw input to
/// an `Action` and an axis to a `Range`.
///
/// Dispatch:
///
/// Each frame, raw input is gathered by calling the relevant `process_button`
/// and `process_axis` functions. The data is then process and added to
/// `MappedData`, which holds the triggered actions and ranges for a given
/// frame. As stated before, the lookup is performed by the enabled `ActionSet`.
///
/// The `MappedData` is then passed to a list of `InputCallback`s which can run
/// the relevant checks and perform what they need to base off that. This is done
/// when calling `update`, which requires delta_time as its argument.
///
/// The callbacks can check the `MappedData` for pressed or held actions. The
/// definition of a held action is an action which is pressed currently but
/// also pressed in the frame before. Since the extra information from ranges
/// is useful, a `RangeData` is returned from `get_range` which includes not
/// only the value but also sensitivity, minimum and maximum input.
///
/// In summary,
/// 	Define and create an actionset
/// 	Define and add actions and ranges to the actionset
/// 	Enable the actionset
/// 	Call `process_button` and `process_axis` where your code gathers input
/// 	Call `update` in the update loop
///
/// Any number of actionsets can be added to the global store, but only one can
/// be activated at any point in time.

namespace input {

struct Action {
	uint32_t code;		// user defined code
	uint32_t raw;		// raw input
	std::string desc;	// name or description of the action
};

struct Range {
	uint32_t code;		// user defined code
	uint32_t axis;		// raw axis input
	float sensitivity;	// factor to multiply the final mapped value by
	float min;		// min input value the axis supports
	float max;		// max input value the axis supports
	std::string desc;	// name or description of the range
};

struct ActionSet {
	uint32_t id;
	std::unordered_map<uint32_t, Action> action_map; // mapping of raw key/button to action
	std::unordered_map<uint32_t, Range> range_map;   // mapping of raw axis to action

	// TODO: warn on duplicate keys?
	ActionSet(uint32_t id, const std::vector<Action>& actions, const std::vector<Range>& ranges) : id(id) {
		for (const auto& action : actions) {
			action_map[action.raw] = action;
		}

		for (const auto& range : ranges) {
			range_map[range.axis] = range;
		}
	}
};

struct RangeData {
	float value;
	float sensitivity;
	float min;
	float max;
};

struct MappedData {
	float delta_time;
	std::set<uint32_t> pressed;
	std::set<uint32_t> held;

	// TODO: investigate using a single array
	std::unordered_map<uint32_t, RangeData> ranges;

	bool is_pressed(uint32_t action) const {
		return pressed.find(action) != pressed.end();
	}

	bool is_held(uint32_t action) const {
		return held.find(action) != held.end();
	}

	std::optional<RangeData> get_range(uint32_t axis) {
		if (auto range = ranges.find(axis); range != ranges.end()) {
			return range->second;
		}
		return {};
	}
};

using InputCallback = std::function<void(MappedData)>;

void add_actionset(ActionSet& actionset);
void add_callback(InputCallback callback);
void enable_actionset(uint32_t id);

void process_button(uint32_t button, bool pressed, bool held);
void process_axis(uint32_t axis, float value);
void update(float dt);

} // end namespace input

// Helper namespace for range conversion
namespace input::range {

float normalize(float x, float in_min, float in_max, float out_mix, float out_min);

} // end namespace input::range
