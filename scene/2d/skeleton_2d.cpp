/*************************************************************************/
/*  skeleton_2d.cpp                                                      */
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

#include "skeleton_2d.h"
#include "scene/resources/skeleton_modification_2d.h"

void Bone2D::_notification(int p_what) {
	if (p_what == NOTIFICATION_ENTER_TREE) {
		Node *parent = get_parent();
		parent_bone = Object::cast_to<Bone2D>(parent);
		skeleton = nullptr;
		while (parent) {
			skeleton = Object::cast_to<Skeleton2D>(parent);
			if (skeleton) {
				break;
			}
			if (!Object::cast_to<Bone2D>(parent)) {
				break; //skeletons must be chained to Bone2Ds.
			}

			parent = parent->get_parent();
		}

		if (skeleton) {
			Skeleton2D::Bone bone;
			bone.bone = this;
			skeleton->bones.push_back(bone);
			skeleton->_make_bone_setup_dirty();
		}
	}
	if (p_what == NOTIFICATION_LOCAL_TRANSFORM_CHANGED) {
		if (skeleton) {
			skeleton->_make_transform_dirty();
		}
	}
	if (p_what == NOTIFICATION_MOVED_IN_PARENT) {
		if (skeleton) {
			skeleton->_make_bone_setup_dirty();
		}
	}

	if (p_what == NOTIFICATION_EXIT_TREE) {
		if (skeleton) {
			for (int i = 0; i < skeleton->bones.size(); i++) {
				if (skeleton->bones[i].bone == this) {
					skeleton->bones.remove(i);
					break;
				}
			}
			skeleton->_make_bone_setup_dirty();
			skeleton = nullptr;
		}
		parent_bone = nullptr;
	}
}

void Bone2D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_rest", "rest"), &Bone2D::set_rest);
	ClassDB::bind_method(D_METHOD("get_rest"), &Bone2D::get_rest);
	ClassDB::bind_method(D_METHOD("apply_rest"), &Bone2D::apply_rest);
	ClassDB::bind_method(D_METHOD("get_skeleton_rest"), &Bone2D::get_skeleton_rest);
	ClassDB::bind_method(D_METHOD("get_index_in_skeleton"), &Bone2D::get_index_in_skeleton);

	ClassDB::bind_method(D_METHOD("set_default_length", "default_length"), &Bone2D::set_default_length);
	ClassDB::bind_method(D_METHOD("get_default_length"), &Bone2D::get_default_length);

	ADD_PROPERTY(PropertyInfo(Variant::TRANSFORM2D, "rest"), "set_rest", "get_rest");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "default_length", PROPERTY_HINT_RANGE, "1,1024,1"), "set_default_length", "get_default_length");
}

void Bone2D::set_rest(const Transform2D &p_rest) {
	rest = p_rest;
	if (skeleton) {
		skeleton->_make_bone_setup_dirty();
	}

	update_configuration_warning();
}

Transform2D Bone2D::get_rest() const {
	return rest;
}

Transform2D Bone2D::get_skeleton_rest() const {
	if (parent_bone) {
		return parent_bone->get_skeleton_rest() * rest;
	} else {
		return rest;
	}
}

void Bone2D::apply_rest() {
	set_transform(rest);
}

void Bone2D::set_default_length(float p_length) {
	default_length = p_length;
}

float Bone2D::get_default_length() const {
	return default_length;
}

int Bone2D::get_index_in_skeleton() const {
	ERR_FAIL_COND_V(!skeleton, -1);
	skeleton->_update_bone_setup();
	return skeleton_index;
}

String Bone2D::get_configuration_warning() const {
	String warning = Node2D::get_configuration_warning();
	if (!skeleton) {
		if (warning != String()) {
			warning += "\n\n";
		}
		if (parent_bone) {
			warning += TTR("This Bone2D chain should end at a Skeleton2D node.");
		} else {
			warning += TTR("A Bone2D only works with a Skeleton2D or another Bone2D as parent node.");
		}
	}

	if (rest == Transform2D(0, 0, 0, 0, 0, 0)) {
		if (warning != String()) {
			warning += "\n\n";
		}
		warning += TTR("This bone lacks a proper REST pose. Go to the Skeleton2D node and set one.");
	}

	return warning;
}

Bone2D::Bone2D() {
	skeleton = nullptr;
	parent_bone = nullptr;
	skeleton_index = -1;
	default_length = 16;
	set_notify_local_transform(true);
	//this is a clever hack so the bone knows no rest has been set yet, allowing to show an error.
	for (int i = 0; i < 3; i++) {
		rest[i] = Vector2(0, 0);
	}
}

//////////////////////////////////////

bool Skeleton2D::_set(const StringName &p_path, const Variant &p_value) {
	String path = p_path;

	if (path.begins_with("modification_stack")) {
		set_modification_stack(p_value);
		return true;
	}
	return true;
}

bool Skeleton2D::_get(const StringName &p_path, Variant &r_ret) const {
	String path = p_path;

	if (path.begins_with("modification_stack")) {
		r_ret = get_modification_stack();
		return true;
	}
	return true;
}

void Skeleton2D::_get_property_list(List<PropertyInfo> *p_list) const {
	p_list->push_back(
			PropertyInfo(Variant::OBJECT, "modification_stack",
					PROPERTY_HINT_RESOURCE_TYPE,
					"SkeletonModificationStack2D",
					PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_DEFERRED_SET_RESOURCE));
}

void Skeleton2D::_make_bone_setup_dirty() {
	if (bone_setup_dirty) {
		return;
	}
	bone_setup_dirty = true;
	if (is_inside_tree()) {
		call_deferred("_update_bone_setup");
	}
}

void Skeleton2D::_update_bone_setup() {
	if (!bone_setup_dirty) {
		return;
	}

	bone_setup_dirty = false;
	RS::get_singleton()->skeleton_allocate(skeleton, bones.size(), true);

	bones.sort(); //sort so bones are always in the same order/index

	for (int i = 0; i < bones.size(); i++) {
		bones.write[i].rest_inverse = bones[i].bone->get_skeleton_rest().affine_inverse(); //bind pose
		bones.write[i].bone->skeleton_index = i;
		Bone2D *parent_bone = Object::cast_to<Bone2D>(bones[i].bone->get_parent());
		if (parent_bone) {
			bones.write[i].parent_index = parent_bone->skeleton_index;
		} else {
			bones.write[i].parent_index = -1;
		}

		bones.write[i].local_pose_override = bones[i].bone->get_skeleton_rest();
	}

	transform_dirty = true;
	_update_transform();
	emit_signal("bone_setup_changed");
}

void Skeleton2D::_make_transform_dirty() {
	if (transform_dirty) {
		return;
	}
	transform_dirty = true;
	if (is_inside_tree()) {
		call_deferred("_update_transform");
	}
}

void Skeleton2D::_update_transform() {
	if (bone_setup_dirty) {
		_update_bone_setup();
		return; //above will update transform anyway
	}
	if (!transform_dirty) {
		return;
	}

	transform_dirty = false;

	for (int i = 0; i < bones.size(); i++) {
		ERR_CONTINUE(bones[i].parent_index >= i);
		if (bones[i].parent_index >= 0) {
			bones.write[i].accum_transform = bones[bones[i].parent_index].accum_transform * bones[i].bone->get_transform();
		} else {
			bones.write[i].accum_transform = bones[i].bone->get_transform();
		}
	}

	for (int i = 0; i < bones.size(); i++) {
		Transform2D final_xform = bones[i].accum_transform * bones[i].rest_inverse;
		RS::get_singleton()->skeleton_bone_set_transform_2d(skeleton, i, final_xform);
	}
}

int Skeleton2D::get_bone_count() const {
	ERR_FAIL_COND_V(!is_inside_tree(), 0);

	if (bone_setup_dirty) {
		const_cast<Skeleton2D *>(this)->_update_bone_setup();
	}

	return bones.size();
}

Bone2D *Skeleton2D::get_bone(int p_idx) {
	ERR_FAIL_COND_V(!is_inside_tree(), nullptr);
	ERR_FAIL_INDEX_V(p_idx, bones.size(), nullptr);

	return bones[p_idx].bone;
}

void Skeleton2D::_notification(int p_what) {
	if (p_what == NOTIFICATION_READY) {
		if (bone_setup_dirty) {
			_update_bone_setup();
		}
		if (transform_dirty) {
			_update_transform();
		}

		set_process_internal(true);
		request_ready();
	}

	if (p_what == NOTIFICATION_TRANSFORM_CHANGED) {
		RS::get_singleton()->skeleton_set_base_transform_2d(skeleton, get_global_transform());
	}

	if (p_what == NOTIFICATION_INTERNAL_PROCESS) {
		execute_modification(get_process_delta_time());
	}
}

RID Skeleton2D::get_skeleton() const {
	return skeleton;
}

void Skeleton2D::set_bone_local_pose_override(int bone_idx, Transform2D p_override, float amount, bool persistent) {
	ERR_FAIL_INDEX_MSG(bone_idx, bones.size(), "Bone index is out of range!");
	bones.write[bone_idx].local_pose_override = p_override;
	bones.write[bone_idx].local_pose_override_amount = amount;
	bones.write[bone_idx].local_pose_override_persistent = persistent;
}

Transform2D Skeleton2D::get_bone_local_pose_override(int bone_idx) {
	ERR_FAIL_INDEX_V_MSG(bone_idx, bones.size(), Transform2D(), "Bone index is out of range!");
	return bones[bone_idx].local_pose_override;
}

void Skeleton2D::set_modification_stack(Ref<SkeletonModificationStack2D> p_stack) {
	if (modification_stack.is_valid()) {
		modification_stack->is_setup = false;
		modification_stack->set_skeleton(nullptr);
	}
	modification_stack = p_stack;
	if (modification_stack.is_valid()) {
		modification_stack->set_skeleton(this);
		modification_stack->setup();
	}
}

Ref<SkeletonModificationStack2D> Skeleton2D::get_modification_stack() const {
	return modification_stack;
}

void Skeleton2D::execute_modification(float delta) {
	if (!modification_stack.is_valid()) {
		return;
	}

	// Cache the transform of the Bone2D before we apply any modifications to it.
	for (int i = 0; i < bones.size(); i++) {
		bones.write[i].local_pose_cache = bones[i].bone->get_transform();
		bones[i].bone->set_transform(bones.write[i].local_pose_cache);
	}

	modification_stack->execute(delta);

	// A hack: Override the CanvasItem transform using the RenderingServer so the local pose override is taken into account.
	for (int i = 0; i < bones.size(); i++) {
		if (bones[i].local_pose_override_amount > 0) {
			bones[i].bone->set_transform(bones.write[i].local_pose_cache);
			Transform2D final_trans = bones[i].local_pose_cache;
			final_trans = final_trans.interpolate_with(bones[i].local_pose_override, bones[i].local_pose_override_amount);

			RenderingServer::get_singleton()->canvas_item_set_transform(bones[i].bone->get_canvas_item(), final_trans);

			if (bones[i].local_pose_override_persistent) {
				bones.write[i].local_pose_override_amount = 0.0;
			}
		}
		else {
			// TODO: see if there is a way to undo the override without having to resort to setting every bone's transform.
			RenderingServer::get_singleton()->canvas_item_set_transform(bones[i].bone->get_canvas_item(), bones[i].local_pose_cache);
		}
	}
}

void Skeleton2D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_update_bone_setup"), &Skeleton2D::_update_bone_setup);
	ClassDB::bind_method(D_METHOD("_update_transform"), &Skeleton2D::_update_transform);

	ClassDB::bind_method(D_METHOD("get_bone_count"), &Skeleton2D::get_bone_count);
	ClassDB::bind_method(D_METHOD("get_bone", "idx"), &Skeleton2D::get_bone);

	ClassDB::bind_method(D_METHOD("get_skeleton"), &Skeleton2D::get_skeleton);

	ClassDB::bind_method(D_METHOD("set_modification_stack", "modification_stack"), &Skeleton2D::set_modification_stack);
	ClassDB::bind_method(D_METHOD("get_modification_stack"), &Skeleton2D::get_modification_stack);
	ClassDB::bind_method(D_METHOD("execute_modification"), &Skeleton2D::execute_modification);

	ADD_SIGNAL(MethodInfo("bone_setup_changed"));
}

Skeleton2D::Skeleton2D() {
	bone_setup_dirty = true;
	transform_dirty = true;

	skeleton = RS::get_singleton()->skeleton_create();
	set_notify_transform(true);
}

Skeleton2D::~Skeleton2D() {
	RS::get_singleton()->free(skeleton);
}
