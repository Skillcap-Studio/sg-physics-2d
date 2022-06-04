extends "res://addons/gut/test.gd"

func test_float_conversion():
	var a: float = 2.4
	var b: int = SGFixed.from_float(a)
	var c: float = SGFixed.to_float(b)
	
	assert_eq(b, 157286)
	assert_eq("%s" % c, "2.399994")

func test_int_conversion():
	var a: int = 2
	var b: int = SGFixed.from_int(a)
	var c: float = SGFixed.to_int(b)
	
	assert_eq(b, 65536*2)
	assert_eq(c, 2.0)

func test_multiplication():
	var a: int = SGFixed.from_float(2.5)
	var b: int = SGFixed.from_float(2.5)
	var c: int = SGFixed.mul(a, b)
	
	assert_eq(c, 409600)

func test_division():
	var a: int = SGFixed.from_int(15)
	var b: int = SGFixed.from_int(2)
	var c: int = SGFixed.div(a, b)
	
	assert_eq(c, 491520)

func test_move_toward():
	var a: int = SGFixed.from_int(1)
	var b: int = SGFixed.from_int(2)
	var c: int = SGFixed.move_toward(a, b, 32768) #0.5
	var d: int = SGFixed.move_toward(a, b, 131072) #2
	
	assert_eq(c, 98304) # 1.5
	assert_eq(d, 131072) # 2

func test_trig():
	#
	# Test some important "well known" values
	#
	
	# sin()
	var sin_0: int = SGFixed.sin(0)
	assert_eq(sin_0, 0)
	
	var sin_90deg: int = SGFixed.sin(SGFixed.PI_DIV_2)
	assert_eq(sin_90deg, 65536)
	
	var sin_180deg: int = SGFixed.sin(SGFixed.PI)
	assert_eq(sin_180deg, 0)
	
	var sin_270deg: int = SGFixed.sin(SGFixed.PI + SGFixed.PI_DIV_2)
	assert_eq(sin_270deg, -65536)
	
	var sin_360deg: int = SGFixed.sin(SGFixed.TAU)
	assert_eq(sin_360deg, 0)
	
	# cos()
	var cos_0: int = SGFixed.cos(0)
	assert_eq(cos_0, 65536)
	
	var cos_90deg: int = SGFixed.cos(SGFixed.PI_DIV_2)
	assert_eq(cos_90deg, 0)
	
	var cos_180deg: int = SGFixed.cos(SGFixed.PI)
	assert_eq(cos_180deg, -65536)
	
	var cos_270deg: int = SGFixed.cos(SGFixed.PI + SGFixed.PI_DIV_2)
	assert_eq(cos_270deg, 0)
	
	var cos_360deg: int = SGFixed.cos(SGFixed.TAU)
	assert_eq(cos_360deg, 65536)
	
	# tan()
	var tan_0: int = SGFixed.tan(0)
	assert_eq(tan_0, 0)
	
	var tan_45deg: int = SGFixed.tan(SGFixed.PI_DIV_4)
	assert_eq(tan_45deg, 65536)
	
	var tan_90deg: int = SGFixed.tan(SGFixed.PI_DIV_2)
	assert_eq(tan_90deg, 0)
	
	var tan_135deg: int = SGFixed.tan(SGFixed.PI_DIV_2 + SGFixed.PI_DIV_4)
	assert_eq(tan_135deg, -65536)
	
	var tan_180deg: int = SGFixed.tan(SGFixed.PI)
	assert_eq(tan_180deg, 0)
	
	var tan_225deg: int = SGFixed.tan(SGFixed.PI + SGFixed.PI_DIV_4)
	assert_eq(tan_225deg, 65536)
	
	var tan_270deg: int = SGFixed.tan(SGFixed.PI + SGFixed.PI_DIV_2)
	assert_eq(tan_270deg, 0)
	
	var tan_315deg: int = SGFixed.tan(SGFixed.PI + SGFixed.PI_DIV_2 + SGFixed.PI_DIV_4)
	# Overshoots it a tiny bit, due to our SGFixed.PI_DIV_4 rounding up.
	assert_eq(tan_315deg, -65538)
	
	var tan_360deg: int = SGFixed.tan(SGFixed.PI * 2)
	assert_eq(tan_360deg, 0)
	
	# asin()
	var asin_65536: int = SGFixed.asin(65536)
	assert_eq(asin_65536, SGFixed.PI_DIV_2)
	
	# acos()
	var acos_65536: int = SGFixed.acos(65536)
	assert_eq(acos_65536, 0)
	
	#
	# Test that big values loop around as expected
	#
	
	var sin_10: int = SGFixed.sin(10*65536)
	assert_eq(sin_10, -35665)
	
	var sin_50000: int = SGFixed.sin(50000*65536)
	assert_eq(sin_50000, -65313)

	var cos_10: int = SGFixed.cos(10*65536)
	assert_eq(cos_10, -55015)

#	var cos_50000: int = SGFixed.cos(50000*65536)
#	assert_eq(cos_50000, 1000)
#
#	var tan_10: int = SGFixed.tan(10*65536)
#	assert_eq(tan_10, 1000)
#
#	var tan_50000: int = SGFixed.tan(50000*65536)
#	assert_eq(tan_50000, 1000)
#
#	var asin_10: int = SGFixed.asin(10*65536)
#	assert_eq(asin_10, -35653)
#
#	var asin_50000: int = SGFixed.asin(50000*65536)
#	assert_eq(asin_50000, -65309)
#
#	var acos_10: int = SGFixed.acos(10*65536)
#	assert_eq(acos_10, 1000)
#
#	var acos_50000: int = SGFixed.acos(50000*65536)
#	assert_eq(acos_50000, 1000)
#
#	var atan_10: int = SGFixed.atan(10*65536)
#	assert_eq(atan_10, 1000)
#
#	var atan_50000: int = SGFixed.atan(50000*65536)
#	assert_eq(atan_50000, 1000)
#
#	var atan2_10: int = SGFixed.atan2(10*65536, 2*65536)
#	assert_eq(atan2_10, 1000)
#
#	var atan2_50000: int = SGFixed.atan2(50000*65536, 2*65536)
#	assert_eq(atan2_50000, 1000)

