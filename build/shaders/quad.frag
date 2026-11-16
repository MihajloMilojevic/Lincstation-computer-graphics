#version 330 core
in vec2 vUV;
out vec4 FragColor;

uniform sampler2D uTexture;
uniform int   uMode;                    // 0 = ravna boja/oblik, 1 = tekstura * uColor, 2 = glif (R8 maska)
uniform int   uShape;                   // za mode 0: 0 = pravougaonik, 1 = zaobljen, 2 = elipsa
uniform float uRadius;                  // radijus zaobljenja (referentne jedinice)
uniform vec2  uSize;                    // veličina kvadrata (isti uniform kao u vertex šejderu)
uniform vec4  uColor;                   // boja / tint (sa sopstvenom alfom)
uniform float uAlpha;                   // dodatna providnost (pulsiranje svetala, overlay...)

void main() {
    vec4 c;
    if (uMode == 1) {
        c = texture(uTexture, vUV) * uColor;
    } else if (uMode == 2) {
        c = vec4(uColor.rgb, uColor.a * texture(uTexture, vUV).r);
    } else {
        c = uColor;
        vec2 h = uSize * 0.5;
        vec2 p = (vUV - 0.5) * uSize;
        if (uShape == 1) {
            float r = min(uRadius, min(h.x, h.y));
            vec2 q = abs(p) - h + r;
            float d = length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - r;
            c.a *= clamp(0.5 - d, 0.0, 1.0);
        } else if (uShape == 2) {
            float d = (length(p / h) - 1.0) * min(h.x, h.y);
            c.a *= clamp(0.5 - d, 0.0, 1.0);
        }
    }
    FragColor = vec4(c.rgb, c.a * uAlpha);
}
