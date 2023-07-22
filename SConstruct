#!/usr/bin/env python
import os

opts = Variables([], ARGUMENTS)
env = SConscript("godot-cpp/SConstruct")

# Define our options
opts.Add(PathVariable('target_path', 'The path where the lib is installed.', 'bin/'))
opts.Add(PathVariable('target_name', 'The library name.', 'libsgphysics2d', PathVariable.PathAccept))

# For the reference:
# - CCFLAGS are compilation flags shared between C and C++
# - CFLAGS are for C-specific compilation flags
# - CXXFLAGS are for C++-specific compilation flags
# - CPPFLAGS are for pre-processor flags
# - CPPDEFINES are for pre-processor defines
# - LINKFLAGS are for linking flags

env.Append(CPPDEFINES={'GODOT_MAJOR_VERSION': "4"})
env.Append(CPPDEFINES=['USE_GDEXTENSION'])

# Updates the environment with the option variables.
opts.Update(env)

sg_physics_2d_subdirs = [
	"internal/",
    "godot-4/math/",
    "godot-4/servers/",
    "godot-4/scene/2d/",
    "godot-4/scene/resources/",
    "godot-4/scene/animation/",
    "godot-4/",
]
sources = []

for d in sg_physics_2d_subdirs:
    sources += Glob('src/sg_physics_2d/' + d + "*.cpp")

if env.editor_build or env.debug_features:
    sources += Glob('src/sg_physics_2d/godot-4/editor/*.cpp')

env.Append(CPPPATH=['src/'])

# Platform specifics.
if env["platform"] == "macos":
    library = env.SharedLibrary(
        (env['target_path'] + 'sg-physics-2d/lib/' + env['target_name'] + ".{}.{}.framework/" + env['target_name'] + ".{}.{}").format(
            env["platform"], env["target"], env["platform"], env["target"]
        ),
        source=sources,
    )
else:
    library = env.SharedLibrary(
        env['target_path'] + 'sg-physics-2d/lib/' + env['target_name'] + env["suffix"] + env["SHLIBSUFFIX"],
        source=sources,
    )


build_icons = env.Install(target=env['target_path'] + 'sg-physics-2d/icons/', source=Glob('src/sg_physics_2d/godot-4/icons/*.svg'))
project_icons = env.Install(target='projects/godot-4/sg-physics-2d/icons/', source=Glob('src/sg_physics_2d/godot-4/icons/*.svg'))

library_copy = env.Install(target='projects/godot-4/sg-physics-2d/lib/', source=library)
gdextension_copy = env.Install(target='projects/godot-4/sg-physics-2d/', source=env['target_path'] + 'sg-physics-2d/sg-physics-2d.gdextension')

Default(library, build_icons, project_icons, library_copy, gdextension_copy)


# Generates help for the -h scons option.
Help(opts.GenerateHelpText(env))
