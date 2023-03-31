extends Node2D

onready var kinematic_right = $SGKinematicBody2DRight
onready var kinematic_right2 = $SGKinematicBody2DRight2
onready var kinematic_left = $SGKinematicBody2DLeft
onready var kinematic_left2 = $SGKinematicBody2DLeft2
var movements := []
var slides := []


func move_and_compare(kinematic: SGKinematicBody2D, velocity: SGFixedVector2) -> void:
	var before := kinematic.fixed_position.copy()
	kinematic.move_and_slide(velocity)
	movements.append(before.distance_to(kinematic.fixed_position))
	slides.append(kinematic.get_slide_count())


func _physics_process(_delta: float) -> void:
	movements.clear()
	move_and_compare(kinematic_right, SGFixed.vector2(SGFixed.ONE * 30, 0))
	move_and_compare(kinematic_right2, SGFixed.vector2(SGFixed.ONE * 30, 0));
	move_and_compare(kinematic_left, SGFixed.vector2(SGFixed.NEG_ONE * 30, 0))
	move_and_compare(kinematic_left2, SGFixed.vector2(SGFixed.NEG_ONE * 30, 0));
