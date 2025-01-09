#pragma once

// These are used as ids and uints so the auto decay is useful, but so is the
// namespace issue so adding them to structs solves both isses.

struct ActionSets {
	enum ActionSetId { DEFAULT };
};

struct DefaultActions {
	enum Actions { FORWARD, RIGHT, LEFT, BACKWARD, QUIT };
};
