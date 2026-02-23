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

// Output fragment color
out vec4 finalColor;

void main()
{
    // Ambient light
    vec4 ambient = ambientColor * fragColor;

    // Diffuse light
    vec3 norm = normalize(fragNormal);
    vec3 lightDir = normalize(lightPos - fragPosition);
    float diff = max(dot(norm, lightDir), 0.0);
    vec4 diffuse = vec4(vec3(fragColor) * diff, fragColor.a);

    // Combine results
    finalColor = ambient + diffuse;

    // Apply texture color if available (for future use)
    // finalColor = finalColor * texture(textureSampler, fragTexCoord);
}
