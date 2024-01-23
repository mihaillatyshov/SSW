#extension GL_ARB_bindless_texture : enable

layout(location = 0) out vec4 o_Color;

uniform vec4 u_Color = vec4(1.0);

in vec4 v_Position;

void main() { o_Color = u_Color; }
