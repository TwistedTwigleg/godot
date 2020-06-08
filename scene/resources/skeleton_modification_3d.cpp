/*************************************************************************/
/*  skeleton_modification.cpp                                            */
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

void SkeletonModification3D::execute() {
	if (!enabled)
		return;
}

void SkeletonModification3D::setup_modification() {
	is_setup = true;
}

void SkeletonModification3D::set_enabled(bool p_enabled) {
	enabled = p_enabled;
}

bool SkeletonModification3D::get_enabled() {
	return enabled;
}

void SkeletonModification3D::set_skeleton(Skeleton3D *p_skeleton) {
	skeleton = p_skeleton;
	_change_notify();
}

Skeleton3D *SkeletonModification3D::get_skeleton() {
	return skeleton;
}

void SkeletonModification3D::_bind_methods() {
	BIND_VMETHOD(MethodInfo("execute"));
	BIND_VMETHOD(MethodInfo("setup_modification"));

	ClassDB::bind_method(D_METHOD("set_enabled", "enabled"), &SkeletonModification3D::set_enabled);
	ClassDB::bind_method(D_METHOD("get_enabled"), &SkeletonModification3D::get_enabled);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "enabled"), "set_enabled", "get_enabled");
}

SkeletonModification3D::SkeletonModification3D() {
	is_setup = false;
}

///////////////////////////////////////

void SkeletonModification3D_LookAt::execute() {
	if (!enabled)
		return;

	if (!skeleton) {
		return;
	}

	if (!skeleton->is_inside_tree()) {
		return;
	}

	if (target_node_cache.is_null()) {
		update_cache();
		return;
	}

	Node3D *n = Object::cast_to<Node3D>(ObjectDB::get_instance(target_node_cache));
	if (!n) {
		return;
	}

	if (!n->is_inside_tree()) {
		return;
	}

	if (bone_name != "") {
		int bone_idx = skeleton->find_bone(bone_name);
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

        // Apply additional rotation
        new_bone_trans.basis.rotate_local(Vector3(1, 0, 0), Math::deg2rad(additional_rotation.x));
        new_bone_trans.basis.rotate_local(Vector3(0, 1, 0), Math::deg2rad(additional_rotation.y));
        new_bone_trans.basis.rotate_local(Vector3(0, 0, 1), Math::deg2rad(additional_rotation.z));
        
        // Look rotation if needed
        if (lock_rotation_x || lock_rotation_y || lock_rotation_z) {
            Transform rest_transform = skeleton->get_bone_rest(bone_idx);
            rest_transform = skeleton->local_bone_transform_to_bone_transform(bone_idx, rest_transform);

            Vector3 new_rotation = new_bone_trans.basis.get_rotation();
            Vector3 old_rotation = rest_transform.basis.get_rotation();
            if (lock_rotation_x) {
                new_rotation.x = old_rotation.x;
            }
            if (lock_rotation_y) {
                new_rotation.y = old_rotation.y;
            }
            if (lock_rotation_z) {
                new_rotation.z = old_rotation.z;
            }
            new_bone_trans.basis.set_euler(new_rotation);
        }


		// Convert to a local bone transform, so it retains rotation from parent bones, etc. Then apply to the bone.
		new_bone_trans = skeleton->bone_transform_to_local_bone_transform(bone_idx, new_bone_trans);
		skeleton->set_bone_local_pose_override(bone_idx, new_bone_trans, skeleton->get_skeleton_modification_strength(), true);

		// TODO: make this configurable.
		skeleton->force_update_bone_children_transforms(bone_idx);
	}
}

void SkeletonModification3D_LookAt::setup_modification() {
	is_setup = true;
	update_cache();
}

void SkeletonModification3D_LookAt::_validate_property(PropertyInfo &property) const {
	if (!is_setup) {
		return;
	}
    
    if (property.name == "bone_name") {
		if (skeleton) {
            String names;
            for (int i = 0; i < skeleton->get_bone_count(); i++) {
                if (i > 0) {
                    names += ",";
                }
                names += skeleton->get_bone_name(i);
            }

            property.hint = PROPERTY_HINT_ENUM;
            property.hint_string = names;
		} else {
			property.hint = PROPERTY_HINT_NONE;
			property.hint_string = "";
		}
	}
}

void SkeletonModification3D_LookAt::set_bone_name(String p_name) {
	bone_name = p_name;
	update_cache();
}

String SkeletonModification3D_LookAt::get_bone_name() {
	return bone_name;
}

void SkeletonModification3D_LookAt::update_cache() {
	if (!is_setup) {
		return;
	}

	target_node_cache = ObjectID();
	if (skeleton) {
		if (skeleton->is_inside_tree()) {
			if (skeleton->has_node(target_node)) {
				Node *node = skeleton->get_node(target_node);
				if (!node || skeleton == node) {
					return;
				}
				target_node_cache = node->get_instance_id();
			}
		}
	}
}

void SkeletonModification3D_LookAt::set_target_node(const NodePath &p_target_node) {
	target_node = p_target_node;
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

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "bone_name"), "set_bone_name", "get_bone_name");
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "target_nodepath", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "Node3D"), "set_target_node", "get_target_node");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "lookat_axis", PROPERTY_HINT_ENUM, "axis x, axis y, axis z"), "set_lookat_axis", "get_lookat_axis");
    ADD_GROUP("Additional Settings", "");
    ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "rotation_offset"), "set_rotation_offset", "get_rotation_offset");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "lock_rotation_x"), "set_lock_rotation_x", "get_lock_rotation_x");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "lock_rotation_y"), "set_lock_rotation_y", "get_lock_rotation_y");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "lock_rotation_z"), "set_lock_rotation_z", "get_lock_rotation_z");
    
}

SkeletonModification3D_LookAt::SkeletonModification3D_LookAt() {
	skeleton = nullptr;
    is_setup = false;
    lookat_axis = 1;
    additional_rotation = Vector3();
    lock_rotation_x = false;
    lock_rotation_y = false;
    lock_rotation_z = false;
}

SkeletonModification3D_LookAt::~SkeletonModification3D_LookAt() {
}

///////////////////////////////////////