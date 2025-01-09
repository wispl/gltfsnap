#include "input.h"

#include <cassert>
#include <cstdint>
#include <vector>

namespace input {

static MappedData mapped_data;

// TODO: might want this to be a vector as well, use a reference_wrapper in that case
static ActionSet* enabled;
static std::vector<ActionSet> actionsets;
static std::vector<InputCallback> callbacks;


// TODO: handle duplicate ids
void add_actionset(ActionSet& actionset)
{
	actionsets.push_back(actionset);
}

void add_callback(InputCallback callback)
{
	callbacks.push_back(callback);
}

void enable_actionset(uint32_t id)
{
	assert(actionsets.size() > 0);

	for (auto& actionset : actionsets) {
		if (actionset.id == id) {
			enabled = &actionset;
			return;
		}
	}
	// TODO: handle no actionset found
}

// TODO: the `held` key is not used when `pressed` is false, which is weird,
// split into two functions?
void process_button(uint32_t button, bool pressed, bool held)
{
	assert(enabled != nullptr);

	if (auto find = enabled->action_map.find(button); find != enabled->action_map.end()) {
		auto code = find->second.code;
		if (pressed) {
			mapped_data.pressed.insert(code);
			if (held) {
				mapped_data.held.insert(code);
			}
		} else {
			mapped_data.pressed.erase(code);
			mapped_data.held.erase(code);
		}
	}
}

void process_axis(uint32_t axis, float value)
{
	assert(enabled != nullptr);

	if (auto find = enabled->range_map.find(axis); find != enabled->range_map.end()) {
		auto range = find->second;
		mapped_data.ranges[axis] = RangeData {
			.value = value,
			.sensitivity = range.sensitivity,
			.min = range.min,
			.max = range.max,
		};
	}
}

void update(float dt)
{
	mapped_data.delta_time = dt;
	for (const auto& callback : callbacks) {
		callback(mapped_data);
	}
	// Clear pressed actions. Held actions do not get cleared since they
	// should persist in the next frame.
	mapped_data.pressed.clear();
}

} // namespace input
