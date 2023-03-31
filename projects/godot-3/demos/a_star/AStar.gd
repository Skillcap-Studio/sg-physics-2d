extends TileMap

onready var astar = SGAStar2D.new()
onready var used_cells = get_used_cells()
onready var line := $Line2D

var path

func _ready() -> void:
	_add_points()
	_connect_points()
	_get_path(Vector2(0, 0), Vector2(8, 8))
	for p in path:
		line.add_point(p.to_float() * cell_size + cell_size * 0.5)

func _add_points() -> void:
	for cell in used_cells:
		astar.add_point(id(cell),SGFixed.from_float_vector2(cell), SGFixed.ONE)

func _connect_points() -> void:
	for cell in used_cells:
		var neighbors = [Vector2(1, 0), Vector2(-1, 0), Vector2(0, 1), Vector2(0, -1)]
		for neighbor in neighbors:
			var next_cell = cell + neighbor
			if used_cells.has(next_cell):
				astar.connect_points(id(cell), id(next_cell), false)

func _get_path(start, end):
	path = astar.get_point_path(id(start), id(end))

func id(point):
	var a = point.x
	var b = point.y
	return (a + b) * (a + b + 1) / 2 + b
