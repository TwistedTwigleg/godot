/*************************************************************************/
/*  skeleton_modification_3d.cpp                                         */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2020 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2020 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include "skeleton_modification_3d.h"
#include "scene/3d/skeleton_3d.h"

///////////////////////////////////////

void SkeletonModificationStack3D::_get_property_list(List<PropertyInfo> *p_list) const {
	for (int i = 0; i < modifications.size(); i++) {
		p_list->push_back(
				PropertyInfo(Variant::OBJECT, "Modifications/" + itos(i),
						PROPERTY_HINT_RESOURCE_TYPE,
						"SkeletonModification3D",
						PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_DEFERRED_SET_RESOURCE));
	}
}

bool SkeletonModificationStack3D::_set(const StringName &p_path, const Variant &p_value) {
	String path = p_path;

	if (path.begins_with("Modifications/")) {
		int mod_idx = path.get_slicec('/', 1).to_int();
		set_modification(mod_idx, p_value);
		return true;
	}
	return true;
}

bool SkeletonModificationStack3D::_get(const StringName &p_path, Variant &r_ret) const {
	String path = p_path;

	if (path.begins_with("Modifications/")) {
		int mod_idx = path.get_slicec('/', 1).to_int();
		r_ret = get_modification(mod_idx);
		return true;
	}
	return true;
}

void SkeletonModificationStack3D::setup() {
	if (is_setup) {
		return;
	}

	if (skeleton != nullptr) {
		is_setup = true;
		for (int i = 0; i < modifications.size(); i++) {
			if (!modifications[i].is_valid()) {
				continue;
			}
			modifications.get(i)->setup_modification(this);
		}
	} else {
		WARN_PRINT("Cannot setup SkeletonModificationStack3D: no skeleton set!");
	}
}

void SkeletonModificationStack3D::execute() {
	if (!is_setup || skeleton == nullptr || !enabled || is_queued_for_deletion()) {
		return;
	}
	if (!skeleton->is_inside_tree() || !skeleton->is_inside_world()) {
		return;
	}

	// TODO: not sure if this is needed
	skeleton->clear_bones_local_pose_override();

	for (int i = 0; i < modifications.size(); i++) {
		if (!modifications[i].is_valid()) {
			continue;
		}
		modifications.get(i)->execute();
	}
}

void SkeletonModificationStack3D::enable_all_modifications(bool p_enabled) {
	for (int i = 0; i < modifications.size(); i++) {
		if (!modifications[i].is_valid()) {
			continue;
		}
		modifications.get(i)->set_enabled(p_enabled);
	}
}

Ref<SkeletonModification3D> SkeletonModificationStack3D::get_modification(int p_mod_idx) const {
	ERR_FAIL_INDEX_V(p_mod_idx, modifications.size(), nullptr);
	return modifications[p_mod_idx];
}

void SkeletonModificationStack3D::add_modification(Ref<SkeletonModification3D> p_mod) {
	p_mod->setup_modification(this);
	modifications.push_back(p_mod);
}

void SkeletonModificationStack3D::delete_modification(int p_mod_idx) {
	ERR_FAIL_INDEX(p_mod_idx, modifications.size());
	modifications.remove(p_mod_idx);
}

void SkeletonModificationStack3D::set_modification(int p_mod_idx, Ref<SkeletonModification3D> p_mod) {
	ERR_FAIL_INDEX(p_mod_idx, modifications.size());

	if (p_mod == nullptr) {
		modifications.set(p_mod_idx, nullptr);
	} else {
		p_mod->setup_modification(this);
		modifications.set(p_mod_idx, p_mod);
	}
}

void SkeletonModificationStack3D::set_modification_count(int p_count) {
	modifications.resize(p_count);
	_change_notify();
}
int SkeletonModificationStack3D::get_modification_count() const {
	return modifications.size();
}

void SkeletonModificationStack3D::set_skeleton(Skeleton3D *p_skeleton) {
	skeleton = p_skeleton;
}
Skeleton3D *SkeletonModificationStack3D::get_skeleton() const {
	return skeleton;
}

bool SkeletonModificationStack3D::get_is_setup() const {
	return is_setup;
}

void SkeletonModificationStack3D::set_enabled(bool p_enabled) {
	enabled = p_enabled;
}
bool SkeletonModificationStack3D::get_enabled() const {
	return enabled;
}

void SkeletonModificationStack3D::set_strength(float p_strength) {
	ERR_FAIL_COND_MSG(p_strength < 0, "Strength cannot be less than zero!");
	ERR_FAIL_COND_MSG(p_strength > 1, "Strength cannot be more than one!");
	strength = p_strength;
}
float SkeletonModificationStack3D::get_strength() const {
	return strength;
}

void SkeletonModificationStack3D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("setup"), &SkeletonModificationStack3D::setup);
	ClassDB::bind_method(D_METHOD("execute"), &SkeletonModificationStack3D::execute);

	ClassDB::bind_method(D_METHOD("enable_all_modifications", "enabled"), &SkeletonModificationStack3D::enable_all_modifications);
	ClassDB::bind_method(D_METHOD("get_modification", "mod_idx"), &SkeletonModificationStack3D::get_modification);
	ClassDB::bind_method(D_METHOD("add_modification", "modification"), &SkeletonModificationStack3D::add_modification);
	ClassDB::bind_method(D_METHOD("delete_modification", "mod_idx"), &SkeletonModificationStack3D::delete_modification);
	ClassDB::bind_method(D_METHOD("set_modification", "mod_idx", "modification"), &SkeletonModificationStack3D::set_modification);

	ClassDB::bind_method(D_METHOD("set_modification_count"), &SkeletonModificationStack3D::set_modification_count);
	ClassDB::bind_method(D_METHOD("get_modification_count"), &SkeletonModificationStack3D::get_modification_count);

	//ClassDB::bind_method(D_METHOD("set_skeleton", "skeleton"), &SkeletonModificationStack3D::set_skeleton);
	//ClassDB::bind_method(D_METHOD("get_skeleton"), &SkeletonModificationStack3D::get_skeleton);

	ClassDB::bind_method(D_METHOD("get_is_setup"), &SkeletonModificationStack3D::get_is_setup);

	ClassDB::bind_method(D_METHOD("set_enabled", "enabled"), &SkeletonModificationStack3D::set_enabled);
	ClassDB::bind_method(D_METHOD("get_enabled"), &SkeletonModificationStack3D::get_enabled);

	ClassDB::bind_method(D_METHOD("set_strength", "strength"), &SkeletonModificationStack3D::set_strength);
	ClassDB::bind_method(D_METHOD("get_strength"), &SkeletonModificationStack3D::get_strength);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "enabled"), "set_enabled", "get_enabled");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "strength", PROPERTY_HINT_RANGE, "0, 1, 0.001"), "set_strength", "get_strength");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "modification_count", PROPERTY_HINT_RANGE, "0, 100, 1"), "set_modification_count", "get_modification_count");
}

SkeletonModificationStack3D::SkeletonModificationStack3D() {
	skeleton = nullptr;
	modifications = Vector<Ref<SkeletonModification3D>>();
	is_setup = false;
	enabled = false;
	modifications_count = 0;
	strength = 0;
}

///////////////////////////////////////

void SkeletonModification3D::execute() {
	if (!enabled)
		return;
}

void SkeletonModification3D::setup_modification(SkeletonModificationStack3D *p_stack) {
	stack = p_stack;

	if (stack != nullptr) {
		is_setup = true;
	}
}

void SkeletonModification3D::set_enabled(bool p_enabled) {
	enabled = p_enabled;
}

bool SkeletonModification3D::get_enabled() {
	return enabled;
}

void SkeletonModification3D::_bind_methods() {
	BIND_VMETHOD(MethodInfo("execute"));
	BIND_VMETHOD(MethodInfo("setup_modification"));

	ClassDB::bind_method(D_METHOD("set_enabled", "enabled"), &SkeletonModification3D::set_enabled);
	ClassDB::bind_method(D_METHOD("get_enabled"), &SkeletonModification3D::get_enabled);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "enabled"), "set_enabled", "get_enabled");
}

SkeletonModification3D::SkeletonModification3D() {
	stack = nullptr;
	is_setup = false;
}

///////////////////////////////////////

void SkeletonModification3D_LookAt::execute() {
	if (!enabled || !stack || !is_setup || stack->skeleton == nullptr) {
		return;
	}

	if (target_node_cache.is_null()) {
		update_cache();
		return;
	}

	if (bone_idx <= -2) {
		bone_idx = stack->skeleton->find_bone(bone_name);
	}

	Node3D *n = Object::cast_to<Node3D>(ObjectDB::get_instance(target_node_cache));
	if (!n) {
		return;
	}

	if (!n->is_inside_tree()) {
		return;
	}

	if (bone_name != "") {
		if (bone_idx <= -1) {
			return;
		}

		Skeleton3D *skeleton = stack->skeleton;
		Transform new_bone_trans = skeleton->get_bone_local_pose_override(bone_idx);

		// Convert to a global bone transform
		new_bone_trans = skeleton->local_bone_transform_to_bone_transform(bone_idx, new_bone_trans);

		new_bone_trans = new_bone_trans.looking_at(
				skeleton->world_transform_to_bone_transform(n->get_global_transform()).origin,
				skeleton->world_transform_to_bone_transform(skeleton->get_global_transform()).basis[lookat_axis].normalized());

		// NOTE: The looking_at function is Z+ forward, but the bones in the skeleton may not.
		// Because of this, we need to rotate the transform accordingly if the bone mode is not Z+.
		if (skeleton->get_bone_axis_forward(bone_idx) != Vector3(0, 0, 1)) {
			new_bone_trans.basis.rotate_local(skeleton->get_bone_axis_perpendicular(bone_idx), -M_PI / 2.0);
		}

		// Lock rotation if needed
		if (lock_rotation_x || lock_rotation_y || lock_rotation_z) {
			Transform rest_transform = skeleton->get_bone_rest(bone_idx);
			rest_transform = skeleton->local_bone_transform_to_bone_transform(bone_idx, rest_transform);

			Quat new_rotation_quat = new_bone_trans.basis.get_rotation_quat();
			Quat old_rotation_quat = rest_transform.basis.get_rotation_euler();
			if (lock_rotation_x) {
				Vector3 axis = Vector3(1, 0, 0);
				new_rotation_quat = (new_rotation_quat.get_swing_quat(axis)) * (old_rotation_quat.get_twist_quat(axis));
				new_rotation_quat = new_rotation_quat.get_swing_quat(axis);
			}
			if (lock_rotation_y) {
				Vector3 axis = Vector3(0, 1, 0);
				new_rotation_quat = (new_rotation_quat.get_swing_quat(axis)) * (old_rotation_quat.get_twist_quat(axis));
				new_rotation_quat = new_rotation_quat.get_swing_quat(axis);
			}
			if (lock_rotation_z) {
				Vector3 axis = Vector3(0, 0, 1);
				new_rotation_quat = (new_rotation_quat.get_swing_quat(axis)) * (old_rotation_quat.get_twist_quat(axis));
				new_rotation_quat = new_rotation_quat.get_swing_quat(axis);
			}
			new_bone_trans.basis = Basis(new_rotation_quat).scaled(new_bone_trans.basis.get_scale());

		}

		// Apply additional rotation
		new_bone_trans.basis.rotate_local(Vector3(1, 0, 0), Math::deg2rad(additional_rotation.x));
		new_bone_trans.basis.rotate_local(Vector3(0, 1, 0), Math::deg2rad(additional_rotation.y));
		new_bone_trans.basis.rotate_local(Vector3(0, 0, 1), Math::deg2rad(additional_rotation.z));

		// Convert to a local bone transform, so it retains rotation from parent bones, etc. Then apply to the bone.
		new_bone_trans = skeleton->bone_transform_to_local_bone_transform(bone_idx, new_bone_trans);
		skeleton->set_bone_local_pose_override(bone_idx, new_bone_trans, stack->strength, true);

		if (instantly_apply_modification) {
			skeleton->force_update_bone_children_transforms(bone_idx);
		}
	}
}

void SkeletonModification3D_LookAt::setup_modification(SkeletonModificationStack3D *p_stack) {
	stack = p_stack;

	if (stack != nullptr) {
		is_setup = true;
		update_cache();
	}
}

void SkeletonModification3D_LookAt::set_bone_name(String p_name) {
	bone_name = p_name;
	if (stack && stack->skeleton) {
		bone_idx = stack->skeleton->find_bone(bone_name);
	}
}

String SkeletonModification3D_LookAt::get_bone_name() const {
	return bone_name;
}

void SkeletonModification3D_LookAt::update_cache() {
	if (!is_setup || !stack) {
		return;
	}

	target_node_cache = ObjectID();
	if (stack->skeleton) {
		if (stack->skeleton->is_inside_tree()) {
			if (stack->skeleton->has_node(target_node)) {
				Node *node = stack->skeleton->get_node(target_node);
				if (!node || stack->skeleton == node) {
					return;
				}
				target_node_cache = node->get_instance_id();
			}
		}
	}
}

void SkeletonModification3D_LookAt::set_target_node(const NodePath &p_target_node) {
	target_node = p_target_node;
	update_cache();
}
NodePath SkeletonModification3D_LookAt::get_target_node() const {
	return target_node;
}

void SkeletonModification3D_LookAt::set_lookat_axis(int p_axis) {
	ERR_FAIL_COND_MSG(p_axis > 2, "Unknown axis! lookat_axis cannot have be more than 2!");
	ERR_FAIL_COND_MSG(p_axis < 0, "Unkown axis! lookat_axis cannot have be negative!");
	lookat_axis = p_axis;
}
int SkeletonModification3D_LookAt::get_lookat_axis() {
	return lookat_axis;
}

Vector3 SkeletonModification3D_LookAt::get_rotation_offset() const {
	return additional_rotation;
}
void SkeletonModification3D_LookAt::set_rotation_offset(Vector3 p_offset) {
	additional_rotation = p_offset;
}

bool SkeletonModification3D_LookAt::get_lock_rotation_x() const {
	return lock_rotation_x;
}
bool SkeletonModification3D_LookAt::get_lock_rotation_y() const {
	return lock_rotation_y;
}
bool SkeletonModification3D_LookAt::get_lock_rotation_z() const {
	return lock_rotation_z;
}
void SkeletonModification3D_LookAt::set_lock_rotation_x(bool p_lock) {
	lock_rotation_x = p_lock;
}
void SkeletonModification3D_LookAt::set_lock_rotation_y(bool p_lock) {
	lock_rotation_y = p_lock;
}
void SkeletonModification3D_LookAt::set_lock_rotation_z(bool p_lock) {
	lock_rotation_z = p_lock;
}

void SkeletonModification3D_LookAt::set_instantly_apply_modification(bool p_apply) {
	instantly_apply_modification = p_apply;
}
bool SkeletonModification3D_LookAt::get_instantly_apply_modification() const {
	return instantly_apply_modification;
}

void SkeletonModification3D_LookAt::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_bone_name", "name"), &SkeletonModification3D_LookAt::set_bone_name);
	ClassDB::bind_method(D_METHOD("get_bone_name"), &SkeletonModification3D_LookAt::get_bone_name);

	ClassDB::bind_method(D_METHOD("set_target_node", "target_nodepath"), &SkeletonModification3D_LookAt::set_target_node);
	ClassDB::bind_method(D_METHOD("get_target_node"), &SkeletonModification3D_LookAt::get_target_node);

	ClassDB::bind_method(D_METHOD("set_lookat_axis", "lookat_axis"), &SkeletonModification3D_LookAt::set_lookat_axis);
	ClassDB::bind_method(D_METHOD("get_lookat_axis"), &SkeletonModification3D_LookAt::get_lookat_axis);

	ClassDB::bind_method(D_METHOD("set_rotation_offset", "offset"), &SkeletonModification3D_LookAt::set_rotation_offset);
	ClassDB::bind_method(D_METHOD("get_rotation_offset"), &SkeletonModification3D_LookAt::get_rotation_offset);

	ClassDB::bind_method(D_METHOD("set_lock_rotation_x", "lock"), &SkeletonModification3D_LookAt::set_lock_rotation_x);
	ClassDB::bind_method(D_METHOD("get_lock_rotation_x"), &SkeletonModification3D_LookAt::get_lock_rotation_x);
	ClassDB::bind_method(D_METHOD("set_lock_rotation_y", "lock"), &SkeletonModification3D_LookAt::set_lock_rotation_y);
	ClassDB::bind_method(D_METHOD("get_lock_rotation_y"), &SkeletonModification3D_LookAt::get_lock_rotation_y);
	ClassDB::bind_method(D_METHOD("set_lock_rotation_z", "lock"), &SkeletonModification3D_LookAt::set_lock_rotation_z);
	ClassDB::bind_method(D_METHOD("get_lock_rotation_z"), &SkeletonModification3D_LookAt::get_lock_rotation_z);

	ClassDB::bind_method(D_METHOD("set_instantly_apply_modification", "apply_modification"), &SkeletonModification3D_LookAt::set_instantly_apply_modification);
	ClassDB::bind_method(D_METHOD("get_instantly_apply_modification"), &SkeletonModification3D_LookAt::get_instantly_apply_modification);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "bone_name"), "set_bone_name", "get_bone_name");
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "target_nodepath", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "Node3D"), "set_target_node", "get_target_node");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "lookat_axis", PROPERTY_HINT_ENUM, "axis x, axis y, axis z"), "set_lookat_axis", "get_lookat_axis");
	ADD_GROUP("Additional Settings", "");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "lock_rotation_x"), "set_lock_rotation_x", "get_lock_rotation_x");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "lock_rotation_y"), "set_lock_rotation_y", "get_lock_rotation_y");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "lock_rotation_z"), "set_lock_rotation_z", "get_lock_rotation_z");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "rotation_offset"), "set_rotation_offset", "get_rotation_offset");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "instantly_apply_modification"), "set_instantly_apply_modification", "get_instantly_apply_modification");
}

SkeletonModification3D_LookAt::SkeletonModification3D_LookAt() {
	stack = nullptr;
	is_setup = false;
	bone_name = "";
	bone_idx = -2;
	lookat_axis = 1;
	additional_rotation = Vector3();
	lock_rotation_x = false;
	lock_rotation_y = false;
	lock_rotation_z = false;
	instantly_apply_modification = true;
}

SkeletonModification3D_LookAt::~SkeletonModification3D_LookAt() {
}

///////////////////////////////////////