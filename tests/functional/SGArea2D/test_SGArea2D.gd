extends "res://addons/gut/test.gd"

func test_get_overlapping_bodies() -> void:
	var GetOverlappingBodies = load("res://tests/functional/SGArea2D/GetOverlappingBodies.tscn")
	
	var result := []
	
	# Run 5 times to attempt to check if this is deterministic.
	for i in range(5):
		var scene = GetOverlappingBodies.instance()
		add_child(scene)
		
		result = scene.do_get_overlapping_bodies()
		assert_eq(result.size(), 2)
		assert_eq(result[0], scene.static_body1)
		assert_eq(result[1], scene.static_body2)
		
		# Add/remove one of the bodies to change the order in the scene tree.
		scene.remove_child(scene.static_body1)
		scene.add_child(scene.static_body1)
		scene.static_body1.sync_to_physics_engine()
		
		result = scene.do_get_overlapping_bodies()
		assert_eq(result.size(), 2)
		assert_eq(result[0], scene.static_body2)
		assert_eq(result[1], scene.static_body1)
		
		remove_child(scene)
		scene.queue_free()

func test_get_overlapping_areas() -> void:
	var GetOverlappingAreas = load("res://tests/functional/SGArea2D/GetOverlappingAreas.tscn")
	
	var scene = GetOverlappingAreas.instance()
	add_child(scene)
	
	# Ensure only the monitorable area is returned.
	var result = scene.do_get_overlapping_areas()
	assert_eq(result.size(), 1)
	assert_eq(result[0], scene.area2)
	
	remove_child(scene)
	scene.queue_free()
