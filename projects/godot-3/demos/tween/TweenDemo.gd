extends Node2D

onready var tween := $SGTween
onready var node := $SGFixedNode2D
onready var final_position_marker := $SGFixedPosition2D

func _ready() -> void:
	tween.interpolate_property(node, "fixed_position_x", null, final_position_marker.fixed_position_x, SGFixed.from_int(2), SGTween.TRANS_BOUNCE, SGTween.EASE_OUT)
	tween.interpolate_property(node, "fixed_position_y", null, final_position_marker.fixed_position_y, SGFixed.from_int(2), SGTween.TRANS_BOUNCE, SGTween.EASE_IN)
	tween.start()
