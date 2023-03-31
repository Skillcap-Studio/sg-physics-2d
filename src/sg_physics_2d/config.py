#######
# NOTE: This is used for building the Godot 3 module!
#       It has to be at the top-level for technical reasons.
#######

def can_build(env, platform):
    return True

def configure(env):
    pass

def get_doc_path():
    return "godot-3/doc_classes"

def get_doc_classes():
    return [
        'SGArea2D',
        'SGAreaCollision2D',
        'SGAStar2D',
        'SGCapsuleShape2D',
        'SGCircleShape2D',
        'SGCollisionObject2D',
        'SGCollisionPolygon2D',
        'SGCollisionShape2D',
        'SGCurve2D',
        'SGFixed',
        'SGFixedNode2D',
        'SGFixedPosition2D',
        'SGFixedRect2',
        'SGFixedTransform2D',
        'SGFixedVector2',
        'SGKinematicBody2D',
        'SGKinematicCollision2D',
        'SGPath2D',
        'SGPathFollow2D',
        'SGPhysics2DServer',
        'SGRayCast2D',
        'SGRectangleShape2D',
        'SGShape2D',
        'SGStaticBody2D',
        'SGTween',
        'SGYSort',
    ]
