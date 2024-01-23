

out vec4 v_Position;

uniform mat4 u_ProjectionMatrix = mat4(1.0);
uniform mat4 u_ViewMatrix = mat4(1.0);
uniform mat4 u_ModelMatrix = mat4(1.0);

void main()
{
    v_Position = u_ModelMatrix * a_Position;
    gl_Position = u_ProjectionMatrix * u_ViewMatrix * v_Position;
}
