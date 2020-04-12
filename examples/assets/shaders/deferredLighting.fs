#version 330

in vec2 fragTexCoord;
in vec3 fragColor;

out vec4 finalColor;

uniform sampler2D texture0;
uniform sampler2D colorbuffer;
uniform sampler2D normalbuffer;
uniform sampler2D positionbuffer;
uniform sampler2D ssaobuffer;

struct light {
    vec3 position;
    vec3 color;
    
    float linear;
    float quadratic;
};
const int num_lights = 32;
uniform light lights[num_lights];
uniform vec3 viewpos;

vec3 calc_lighting()
{
    vec3 Normal = texture(normalbuffer, fragTexCoord).rgb;
    vec3 FragPos = texture(positionbuffer, fragTexCoord).rgb;
    vec3 Diffuse = texture(colorbuffer, fragTexCoord).rgb;
    float Specular = texture(colorbuffer, fragTexCoord).a;
    
    vec3 lighting = Diffuse * 0.1;
    vec3 viewdir = normalize(viewpos - FragPos);
    for (int i = 0; i < num_lights; i++) {
        vec3 lightdir = normalize(lights[i].position - FragPos);
        vec3 diffuse = max(dot(Normal, lightdir), 0.0) * Diffuse * lights[i].color;
        
        vec3 halfwaydir = normalize(lightdir + viewdir);
        float spec = pow(max(dot(Normal, halfwaydir), 0.0), 16.0);
        vec3 specular = lights[i].color * spec * Specular;
        
        float distance = length(lights[i].position - FragPos);
        float attenuation = 1.0 / (1.0 + lights[i].linear * distance + lights[i].quadratic * distance * distance);
        diffuse *= attenuation;
        specular *= attenuation;
        lighting += diffuse + specular;
    }
    lighting *= texture(ssaobuffer, fragTexCoord).rgb;
    
    return lighting;
    
}

void main()
{
    finalColor = vec4(calc_lighting(), 1);
}
