#pragma once

#include <cstdint>
#include <functional>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

// TODO: handle cursors and controllers

/// Input
///
/// Configuration:
///
/// This abstracts raw inputs into some game or user defined term. Oftentimes,
/// the input consumer does not need or want to know that 'w' was pressed,
/// rather, they want to know if a button to MOVE_FORWARD was pressed. Besided
/// being easier to reason about, it also allows for sane remapping.
///
/// An `Action` represents this idea, representing a mapping between a raw
/// input to some user-defined code, likely a enum.
///
/// An `ActionSet` is a set of `Action`s. By grouping actions in some group, it
/// is possible at runtime to enable different mapping configurations. The
/// `ActionSet` is responsible for matching the raw input to an `Action`.
///
/// Dispatch:
///
///  Each frame, raw input is gathered by calling the relevant
/// `process_button` functions. The data is then process and added to
/// `MappedData`, which holds the activated actions for a given frame.
/// The lookup is performed by the enabled `ActionSet`.
///
/// The `MappedData` is then passed to a list of `InputCallback`s which can run
/// the relevant checks and perform what they need to base off that. This is done
/// when calling `update`, which requires delta_time as its argument.

namespace input {

struct Action {
	uint32_t code;		// user defined code
	uint32_t raw;		// raw input
	std::string desc;	// name or desc of the action
};

struct ActionSet {
	uint32_t id;
	std::unordered_map<uint32_t, Action> action_map; // mapping of raw input to action

	ActionSet(uint32_t id, std::vector<Action>& actions) : id(id) {
		// TODO: warn on duplicate keys?
		for (const auto& action : actions) {
			action_map[action.raw] = action;
		}
	}
};

struct MappedData {
	float delta_time;
	std::set<uint32_t> pressed;
	std::set<uint32_t> held;

	bool is_pressed(uint32_t action) const {
		return pressed.find(action) != pressed.end();
	}

	bool is_held(uint32_t action) const {
		return held.find(action) != held.end();
	}
};

using InputCallback = std::function<void(MappedData)>;

void add_actionset(ActionSet& actionset);
void add_callback(InputCallback callback);
void enable_actionset(uint32_t id);

void process_button(uint32_t button, bool pressed, bool held);
void update(float dt);

} // namespace input
