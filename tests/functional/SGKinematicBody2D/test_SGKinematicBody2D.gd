extends "res://addons/gut/test.gd"

func test_move_and_collide_deepest_penetration() -> void:
	var MoveAndCollide1 = load("res://tests/functional/SGKinematicBody2D/MoveAndCollide1.tscn")
	
	# Run 5 times to attempt to check if this is deterministic.
	for i in range(5):
		var scene = MoveAndCollide1.instance()
		add_child(scene)
		
		var collision: SGKinematicCollision2D 
		
		collision = scene.do_move_and_collide()
		assert_not_null(collision)
		assert_eq(collision.collider, scene.static_body2)
		assert_eq(collision.normal.x, 0)
		assert_eq(collision.normal.y, 65536)
		assert_eq(scene.kinematic_body.fixed_transform.origin.x, 1310720)
		assert_eq(scene.kinematic_body.fixed_transform.origin.y, 1966592)
		assert_eq(collision.remainder.x, 0)
		assert_eq(collision.remainder.y, -66048)
		
		# Add/remove one of the bodies to change the order in the scene tree.
		# It shouldn't have any effect because one shape has deeper penetration.
		scene.remove_child(scene.static_body1)
		scene.add_child(scene.static_body1)
		scene.static_body1.sync_to_physics_engine()
		
		scene.reset_kinematic_body()
		collision = scene.do_move_and_collide()
		assert_not_null(collision)
		assert_eq(collision.collider, scene.static_body2)
		assert_eq(collision.normal.x, 0)
		assert_eq(collision.normal.y, 65536)
		assert_eq(scene.kinematic_body.fixed_transform.origin.x, 1310720)
		assert_eq(scene.kinematic_body.fixed_transform.origin.y, 1966592)
		assert_eq(collision.remainder.x, 0)
		assert_eq(collision.remainder.y, -66048)
		
		remove_child(scene)
		scene.queue_free()

func test_move_and_collide_lowest_scene_tree() -> void:
	var MoveAndCollide2 = load("res://tests/functional/SGKinematicBody2D/MoveAndCollide2.tscn")
	
	# Run 5 times to attempt to check if this is deterministic.
	for i in range(5):
		var scene = MoveAndCollide2.instance()
		add_child(scene)

		var collision: SGKinematicCollision2D 

		collision = scene.do_move_and_collide()
		assert_not_null(collision)
		assert_eq(collision.collider, scene.static_body1)
		assert_eq(collision.normal.x, 0)
		assert_eq(collision.normal.y, 65536)
		assert_eq(scene.kinematic_body.fixed_transform.origin.x, 1310720)
		assert_eq(scene.kinematic_body.fixed_transform.origin.y, 1966592)
		assert_eq(collision.remainder.x, 0)
		assert_eq(collision.remainder.y, -66048)

		# Add/remove one of the bodies to change the order in the scene tree
		scene.remove_child(scene.static_body1)
		scene.add_child(scene.static_body1)
		scene.static_body1.sync_to_physics_engine()

		scene.reset_kinematic_body()
		collision = scene.do_move_and_collide()
		assert_not_null(collision)
		assert_eq(collision.collider, scene.static_body2)
		assert_eq(collision.normal.x, 0)
		assert_eq(collision.normal.y, 65536)
		assert_eq(scene.kinematic_body.fixed_transform.origin.x, 1310720)
		assert_eq(scene.kinematic_body.fixed_transform.origin.y, 1966592)
		assert_eq(collision.remainder.x, 0)
		assert_eq(collision.remainder.y, -66048)

		remove_child(scene)
		scene.queue_free()

func test_move_and_collide_slide_on_big_polygon() -> void:
	var MoveAndCollide3 = load("res://tests/functional/SGKinematicBody2D/MoveAndCollide3.tscn")
	
	var scene = MoveAndCollide3.instance()
	add_child(scene)

	for _i in range(200):
		scene._physics_process(1/60)
		assert_gt(scene.movements[0], 1000000)
		assert_gt(scene.movements[1], 1000000)
		assert_gt(scene.movements[2], 1000000)
		assert_gt(scene.movements[3], 1000000)
		assert_lt(scene.slides[0], 2)
		assert_lt(scene.slides[1], 2)
		assert_lt(scene.slides[2], 2)
		assert_lt(scene.slides[3], 2)

	remove_child(scene)
	scene.queue_free()
