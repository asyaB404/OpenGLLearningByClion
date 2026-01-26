#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform int effect;  // 0=正常, 1=反色, 2=灰度, 3=核效果

void main()
{
    vec3 color = texture(screenTexture, TexCoords).rgb;
    
    if (effect == 0)  // 正常显示
    {
        FragColor = vec4(color, 1.0);
    }
    else if (effect == 1)  // 反色
    {
        FragColor = vec4(vec3(1.0 - color), 1.0);
    }
    else if (effect == 2)  // 灰度
    {
        float gray = dot(color, vec3(0.299, 0.587, 0.114));
        FragColor = vec4(vec3(gray), 1.0);
    }
    else if (effect == 3)  // 核效果（边缘检测）
    {
        vec2 tex_offset = 1.0 / textureSize(screenTexture, 0);
        vec3 result = color * 5.0;
        result -= texture(screenTexture, TexCoords + vec2(tex_offset.x, 0.0)).rgb;
        result -= texture(screenTexture, TexCoords + vec2(-tex_offset.x, 0.0)).rgb;
        result -= texture(screenTexture, TexCoords + vec2(0.0, tex_offset.y)).rgb;
        result -= texture(screenTexture, TexCoords + vec2(0.0, -tex_offset.y)).rgb;
        FragColor = vec4(result, 1.0);
    }
    else
    {
        FragColor = vec4(color, 1.0);
    }
}
