/*************************************************************************/
/*  skeleton_modification.h                                              */
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

#ifndef SKELETONMODIFICATION3D_H
#define SKELETONMODIFICATION3D_H

#include "scene/3d/skeleton_3d.h"

///////////////////////////////////////

class Skeleton3D;

class SkeletonModification3D : public Resource {
	GDCLASS(SkeletonModification3D, Resource);
    friend class Skeleton3D;

protected:
	static void _bind_methods();

	Skeleton3D *skeleton;
    bool enabled;
    bool is_setup;

public:
    virtual void execute();
    virtual void setup_modification();

    void set_enabled(bool p_enabled);
    bool get_enabled();

	void set_skeleton(Skeleton3D *p_skeleton);
	Skeleton3D *get_skeleton();

	SkeletonModification3D();
};

///////////////////////////////////////

class SkeletonModification3D_LookAt : public SkeletonModification3D {
	GDCLASS(SkeletonModification3D_LookAt, SkeletonModification3D);

private:
    String bone_name;
    NodePath target_node;
    ObjectID target_node_cache;

    void update_cache();

protected:
	static void _bind_methods();
    void _validate_property(PropertyInfo &property) const;

public:

    virtual void execute();
    virtual void setup_modification();

    void set_bone_name(String p_name);
    String get_bone_name();

    void set_target_node(const NodePath &p_target_node);
	NodePath get_target_node() const;

	SkeletonModification3D_LookAt();
	~SkeletonModification3D_LookAt();
};

///////////////////////////////////////

#endif // SKELETONMODIFICATION3D_H
