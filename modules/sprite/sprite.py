from omega import *
from euclid import *
from cyclops import * 

# Constants
SPRITE_MODEL = 'sprite'
SPRITE_PROGRAM = 'sprite'
SPRITE_SIZE_UNIFORM = 'size'

# Create a model with a single vertex (our sprite template)
spriteModel = ModelGeometry.create(SPRITE_MODEL)
spriteModel.addVertex(Vector3(0, 0, 0))
spriteModel.addPrimitive(PrimitiveType.Points, 0, 1)
getSceneManager().addModel(spriteModel)

# create the sprite shader
spriteProgram = ProgramAsset()
spriteProgram.name = SPRITE_PROGRAM
# Shader code is included directly in this file
spriteProgram.embedded = True

# Vertex shader: just transform the vertx from world to eye space
spriteProgram.vertexShaderName = 'sprite-vs'
spriteProgram.vertexShaderSource = '''
void main(void)
{
    gl_Position = gl_ModelViewMatrix * gl_Vertex;
	gl_FrontColor = gl_Color;
}
'''

# Geometry shader: convert point to screen space, then output a quad
# using the size uniform to determine its size (in screen coordinates)
spriteProgram.geometryShaderName = 'sprite-gs'
spriteProgram.geometryShaderSource = '''
uniform float size;
void main(void)
{
	float halfsize = size / 2;
	
	gl_FrontColor = gl_FrontColorIn[0];
	
	vec4 pos = gl_ProjectionMatrix * gl_PositionIn[0];
	pos.xyz /= pos.w;
	gl_Position.z = pos.z;

	gl_TexCoord[0].st = vec2(1.0,0.0);
	gl_Position.xy = pos.xy + vec2(halfsize, -halfsize);
	EmitVertex();

	gl_TexCoord[0].st = vec2(1.0,1.0);
	gl_Position.xy = pos.xy + vec2(halfsize, halfsize);
	EmitVertex();

	gl_TexCoord[0].st = vec2(0.0,0.0);
	gl_Position.xy = pos.xy + vec2(-halfsize, -halfsize);
	EmitVertex();

	gl_TexCoord[0].st = vec2(0.0,1.0);
	gl_Position.xy = pos.xy + vec2(-halfsize, halfsize);
	EmitVertex();

	EndPrimitive();
}
'''

# Fragment shader: draw the quad using the diffuse texture.
spriteProgram.fragmentShaderName = 'sprite-fs'
spriteProgram.fragmentShaderSource = '''
uniform sampler2D unif_DiffuseMap;
void main (void)
{
    gl_FragColor = texture2D(unif_DiffuseMap, gl_TexCoord[0]) * gl_Color;
}
'''

# set up the geometry shader inputs and outputs, and add the program to the
# scene manager
spriteProgram.geometryOutVertices = 4
spriteProgram.geometryInput = PrimitiveType.Points
spriteProgram.geometryOutput = PrimitiveType.TriangleStrip
getSceneManager().addProgram(spriteProgram)

# API ------------------------------------------------------------------------
# convenience function for creating the sprite size uniform
def createSizeUniform():
	size = Uniform.create(SPRITE_SIZE_UNIFORM, UniformType.Float, 1)
	# set some decent default value.
	size.setFloat(0.05)
	return size
	
# convenience function for creating sprites
def createSprite(textureName, sizeUniform):
	sprite = StaticObject.create(SPRITE_MODEL)
	sprite.getMaterial().setProgram(SPRITE_PROGRAM)
	sprite.getMaterial().setTransparent(True)
	sprite.getMaterial().setDiffuseTexture(textureName)
	sprite.getMaterial().attachUniform(sizeUniform)
	return sprite


