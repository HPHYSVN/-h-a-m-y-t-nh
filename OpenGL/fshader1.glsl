#version 400

in vec4 color;
in vec2 texCoord;

uniform sampler2D texture1;
uniform int useTexture;

out vec4 fColor;

void main()
{
    if (useTexture == 1)
        fColor = texture(texture1, texCoord);
    else
        fColor = color;
}
