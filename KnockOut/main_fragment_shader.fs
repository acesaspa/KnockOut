#version 330 core
out vec4 FragColor;

struct Material {
   sampler2D diffuse;
   sampler2D specular;
   float shininess;
};

struct Light {
   vec3 direction;
   vec3 ambient;
   vec3 diffuse;
   vec3 specular;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec4 FragPosLightSpace;

uniform vec3 viewPos;
uniform vec3 lightPos;
uniform sampler2D shadowMap;

uniform Material material;
uniform Light light;

uniform vec3 textColor;
uniform sampler2D text;


float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // calculate bias (based on depth map resolution and slope)
    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    // check whether current frag pos is in shadow
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    
    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z > 1.0)
        shadow = 0.0;
        
    return shadow;
}

void main()
{
   vec3 color = texture(material.diffuse, TexCoords).rgb;
   vec3 ambient = light.ambient * color;

   vec3 norm = normalize(Normal);
   vec3 lightDir = normalize(-light.direction);
   float diff = max(dot(norm, lightDir), 0.0);
   vec3 diffuse = light.diffuse * diff * color;

   vec3 viewDir = normalize(viewPos - FragPos);
   vec3 reflectDir = reflect(-lightDir, norm);
   float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
   vec3 specular = light.specular * spec * texture(material.specular, TexCoords).rgb;

   float shadow = ShadowCalculation(FragPosLightSpace);   
   vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;  
   
   FragColor = vec4(lighting, 1.0);
}