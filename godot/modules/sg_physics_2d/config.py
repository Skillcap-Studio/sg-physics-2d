def can_build(env, platform):
    return True

def configure(env):
    pass

def get_doc_path():
    return "doc_classes"

def get_doc_classes():
    return [
        'SGFixed',
        'SGFixedVector2',
        'SGFixedRect2',
        'SGFixedTransform2D',
        'SGFixedNode2D',
        'SGFixedPosition2D',
        'SGCollisionObject2D',
        'SGArea2D',
        'SGStaticBody2D',
        'SGKinematicBody2D',
        'SGKinematicCollision2D',
        'SGRayCast2D',
        'SGCollisionShape2D',
        'SGCollisionPolygon2D',
        'SGShape2D',
        'SGRectangleShape2D',
        'SGCircleShape2D',
        'SGYSort',
        'SGCapsuleShape2D',
        'SGCurve2D',
        'SGPath2D',
        'SGPathFollow2D',
    ]

