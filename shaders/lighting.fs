#version 330

// Input vertex attributes (from vertex shader)
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec3 fragNormal;
in vec4 fragColor;

// Input uniform values
uniform sampler2D textureSampler;
uniform vec3 viewPos;
uniform vec3 lightPos;
uniform vec4 ambientColor;

// Fog Uniforms
uniform vec4 fogColor;
uniform float fogDensity;

// Output fragment color
out vec4 finalColor;

void main()
{
    // 1. Ambient
    vec4 ambient = ambientColor * fragColor;

    // 2. Diffuse
    vec3 norm = normalize(fragNormal);
    vec3 lightDir = normalize(lightPos - fragPosition);
    float diff = max(dot(norm, lightDir), 0.0);
    vec4 diffuse = vec4(vec3(fragColor) * diff, fragColor.a);

    // 3. Specular (Blinn-Phong)
    vec3 viewDir = normalize(viewPos - fragPosition);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 32.0); 
    vec4 specular = vec4(0.5, 0.5, 0.5, 1.0) * spec; 

    // 4. Rim Lighting
    float rimFactor = 1.0 - max(dot(viewDir, norm), 0.0);
    rimFactor = pow(rimFactor, 3.0); 
    vec4 rimColor = vec4(1.0, 0.9, 0.5, 1.0) * rimFactor * 0.6;

    // Combine lighting results
    vec4 lightingResult = ambient + diffuse + specular + rimColor;
    lightingResult.a = fragColor.a;

    // 5. Distance Fog
    float dist = length(viewPos - fragPosition);
    float fogFactor = 1.0 / exp((dist * fogDensity) * (dist * fogDensity));
    fogFactor = clamp(fogFactor, 0.0, 1.0);

    // Mix lighting with fog
    finalColor = mix(fogColor, lightingResult, fogFactor);
    finalColor.a = fragColor.a; // Preserve alpha
}
