#version 330 core

#define MAX_LIGHTS 4

in vec3 vNormal;
in vec3 vWorldPos;
in vec2 vTexCoord;

uniform sampler2D uDiffuseMap;
uniform vec3 uKa;
uniform vec3 uKd;
uniform vec3 uKs;
uniform float uShininess;
uniform vec3 uViewPos;
uniform int uNumLights;
uniform vec3 uLightPos[MAX_LIGHTS];
uniform vec3 uLightColor[MAX_LIGHTS];
uniform float uLightIntensity[MAX_LIGHTS];
uniform bool uSelected;

out vec4 FragColor;

void main() {
  vec3 normal = normalize(vNormal);
  vec3 viewDir = normalize(uViewPos - vWorldPos);
  vec4 texColor = texture(uDiffuseMap, vTexCoord);

  // Billboards do Mineways (plantas, trigo, etc.) usam alpha cutout no atlas.
  if (texColor.a < 0.05) {
    discard;
  }

  vec3 ambient = uKa * (uNumLights > 0 ? uLightColor[0] : vec3(0.0));
  vec3 diffuse = vec3(0.0);
  vec3 specular = vec3(0.0);

  for (int i = 0; i < uNumLights; ++i) {
    vec3 lightDir = normalize(uLightPos[i] - vWorldPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), uShininess);

    diffuse += uKd * diff * uLightColor[i] * uLightIntensity[i];
    specular += uKs * spec * uLightColor[i] * uLightIntensity[i];
  }

  vec3 lit = (ambient + diffuse + specular) * texColor.rgb;
  if (uSelected) {
    lit = mix(lit, lit * vec3(0.85, 0.95, 1.15), 0.25);
  }
  FragColor = vec4(lit, texColor.a);
}
