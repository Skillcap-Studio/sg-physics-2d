extends Node2D


func _ready() -> void:
	$SGStaticBody2D/SGCollisionPolygon2D.fixed_polygon = [
		SGFixed.vector2(0, 6291456),
		SGFixed.vector2(12582912, 0),
		SGFixed.vector2(0, -6291456),
		SGFixed.vector2(-12582912, 0),
	]
	$SGStaticBody2D.sync_to_physics_engine()


func _physics_process(_delta: float) -> void:
	var input = Input.get_vector("ui_left", "ui_right", "ui_up", "ui_down")
	$SGKinematicBody2D.move_and_slide(SGFixed.from_float_vector2(input))


func slide_left() -> void:
	$SGKinematicBody2D.move_and_slide(SGFixed.from_float_vector2(Vector2.LEFT))


func move(input: Vector2) -> SGCollisionObject2D:
	return $SGKinematicBody2D.move_and_collide(SGFixed.from_float_vector2(input))
