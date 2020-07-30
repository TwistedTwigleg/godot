/*************************************************************************/
/*  physics_joint_2d.h                                                   */
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

#ifndef PHYSICAL_BONE_2D_H
#define PHYSICAL_BONE_2D_H

#include "scene/2d/joints_2d.h"
#include "scene/2d/physics_body_2d.h"

#include "scene/2d/skeleton_2d.h"

class PhysicalBone2D : public RigidBody2D {
	GDCLASS(PhysicalBone2D, RigidBody2D);

protected:
	bool _set(const StringName &p_name, const Variant &p_value);
	bool _get(const StringName &p_name, Variant &r_ret) const;
	void _get_property_list(List<PropertyInfo> *p_list) const;
	void _notification(int p_what);
	void _direct_state_changed(Object *p_state);
	static void _bind_methods();

private:
	Skeleton2D *parent_skeleton;
	int bone2d_index = -1;
	NodePath bone2d_nodepath;
	ObjectID bone2d_node_cache;

	// TODO: make child joint only required if there is a PhysicalBone2D child. Root PhysicalBone2D nodes shouldn't need joints.
	Joint2D *child_joint;
	bool auto_configure_joint = true;

	bool simulate_physics = false;
	bool _internal_simulate_physics = false;

	void _find_skeleton_parent();
	void _find_joint_child();
	void _auto_configure_joint();

	void _start_physics_simulation();
	void _stop_physics_simulation();

	void _update_bone2d_cache();
	void _position_at_bone2d();

public:
	Joint2D *get_joint() const;
	bool get_auto_configure_joint() const;
	void set_auto_configure_joint(bool p_auto_configure);

	void set_simulate_physics(bool p_simulate);
	bool get_simulate_physics() const;
	bool is_simulating_physics() const;

	void set_bone2d_nodepath(const NodePath &p_nodepath);
	NodePath get_bone2d_nodepath() const;
	void set_bone2d_index(int p_bone_idx);
	int get_bone2d_index() const;

	String get_configuration_warning() const override;

	PhysicalBone2D();
	~PhysicalBone2D();
};

#endif // PHYSICAL_BONE_2D_H