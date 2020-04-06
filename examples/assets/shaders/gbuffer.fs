#version 330
layout (location = 0) out vec3 gposition;
layout (location = 1) out vec3 gnormal;
layout (location = 2) out vec4 galbedospec;
layout (location = 3) out vec4 gemission;

in vec2 fragTexCoord;
in vec3 fragPos;
in vec3 fragNormal;

uniform sampler2D texture0; // diffuse
uniform sampler2D texture1; // specular
uniform sampler2D texture2; // normals

out vec4 finalColor;

void main()
{
    gnormal = texture(texture2, fragTexCoord).rgb;
    if (gnormal.r == 1 && gnormal.g == 1 && gnormal.b == 1)
        gnormal = fragNormal;
    
    gposition = fragPos;
    galbedospec.rgb = texture(texture0, fragTexCoord).rgb;
    galbedospec.a = texture(texture1, fragTexCoord).r;
}
