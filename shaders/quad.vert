#version 330 core
layout(location = 0) in vec2 aPos;      // jedinični kvadrat (0..1), koristi se i kao UV

uniform mat4 uProj;                     // referentne koordinate (960x540) -> NDC
uniform vec2 uPos;                      // gornji levi ugao u referentnim koordinatama
uniform vec2 uSize;                     // veličina u referentnim koordinatama
uniform bool uFlipX;                    // ogledalo po X (tekstura)
uniform vec3 uXform;                    // (originY, scaleY, offsetY): preklapanje poklopca/vrata i izvlačenje fioke

out vec2 vUV;

void main() {
    vUV = vec2(uFlipX ? 1.0 - aPos.x : aPos.x, aPos.y);
    vec2 p = uPos + aPos * uSize;
    p.y = uXform.x + (p.y - uXform.x) * uXform.y + uXform.z;
    gl_Position = uProj * vec4(p, 0.0, 1.0);
}
