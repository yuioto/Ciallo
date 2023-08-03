#version 300 es
precision mediump float;
precision mediump int;

in vec2 p;
flat in vec2 p0;
flat in float r0;
flat in float l0;
flat in vec2 p1;
flat in float r1;
flat in float l1;

// Common
uniform int type;
const int Vanilla = 0, Stamp = 1, Airbrush = 2;
uniform vec3 color;
uniform float alpha;
// Stamp
uniform mediump sampler2D footprint;
uniform float stampInterval;
uniform float noiseFactor;
uniform float rotationFactor;
uniform int stampMode;
const int EquiDistance = 0, RatioDistance = 1;
float x2n(float x);
float n2x(float n);
mat2 rotate(float angle);
// Airbrush
uniform mediump sampler2D gradient;
float sampleGraident(float distance){ return texture(gradient, vec2(distance, 0.0)).r; }

// Output
out vec4 outColor;

// Noise helper functions from _The Book of Shader_.
float random (in vec2 st);
float noise (in vec2 st);
float fbm (in vec2 st);

void main()	{
    vec2 tangent = normalize(p1 - p0);
    vec2 normal = vec2(-tangent.y, tangent.x);

    float len = distance(p1, p0);
    vec2 pLocal = vec2(dot(p-p0, tangent), dot(p-p0, normal));
    vec2 p0Local = vec2(0, 0);
    vec2 p1Local = vec2(len, 0);

    float cosTheta = (r0 - r1)/len;
    float d0 = distance(p, p0);
    float d0cos = pLocal.x / d0;
    float d1 = distance(p, p1);
    float d1cos = (pLocal.x - len) / d1;

    // remove corners
    if(d0cos < cosTheta && d0 > r0) discard;
    if(d1cos > cosTheta && d1 > r1) discard;
    
    // Type specific parts
    if(type == Vanilla){
        if(d0 < r0 && d1 < r1) discard;
        float A = (d0 < r0 || d1 < r1) ? 1.0 - sqrt(1.0 - alpha) : alpha;
        outColor = vec4(color, A);
        return;
    }
    
    if(type == Stamp){
        // Two roots of the quadratic polynomial are frontedge and backedge
        // formulas from SIGGRAPH 2022 Talk - A Fast & Robust Solution for Cubic & Higher-Order Polynomials
        float a, b, c, delta;
        a = 1.0 - pow(cosTheta, 2.0);
        b = 2.0 * (r0 * cosTheta - pLocal.x);
        c = pow(pLocal.x, 2.0) + pow(pLocal.y, 2.0) - pow(r0, 2.0);
        delta = pow(b, 2.0) - 4.0*a*c;
        if(delta <= 0.0) discard; // This should never happen.
        
        float tempMathBlock = b + sign(b) * sqrt(delta);
        float x1 = -2.0 * c / tempMathBlock;
        float x2 = -tempMathBlock / (2.0*a);
        float frontEdge = x1 <= x2 ? x1 : x2;
        float backEdge = x1 > x2 ? x1 : x2;

        float summedIndex = l0/stampInterval;
        float startIndex, endIndex;
        if (frontEdge <= 0.0){
            startIndex = ceil(summedIndex);
        }
        else{
            startIndex = ceil(summedIndex + x2n(frontEdge));
        }
        endIndex = l1/stampInterval;
        float backIndex = x2n(backEdge) + summedIndex;
        endIndex = endIndex < backIndex ? endIndex : backIndex;
        if(startIndex > endIndex) discard;

        int MAX_i = 128; float currIndex = startIndex;
        float A = 0.0;
        for(int i = 0; i < MAX_i; i++){
            float currStampLocalX = n2x(currIndex - summedIndex);
            
            vec2 pToStamp = pLocal - vec2(currStampLocalX, 0.0);
            float angle = rotationFactor*radians(360.0*fract(sin(currIndex)*1.0));
            pToStamp *= rotate(angle);

            float r = r0 - cosTheta * currStampLocalX;
            vec2 textureCoordinate = (pToStamp/r + 1.0)/2.0;
            float opacity = texture(footprint, textureCoordinate).a;
            opacity = clamp(opacity - noiseFactor*fbm(textureCoordinate*50.0), 0.0, 1.0) * alpha;
            A = A * (1.0-opacity) + opacity;

            currIndex += 1.0;
            if(currIndex > endIndex) break;
        }
        if(A < 1e-4) discard;
        outColor = vec4(color, A);
        return;
    }

    if(type == Airbrush){
        float tanTheta = sqrt(1.0 - cosTheta*cosTheta)/cosTheta;
        float mid = pLocal.x - abs(pLocal.y)/tanTheta;
        float A = alpha;
        float transparency0 = d0 > r0 ? 1.0:sqrt(1.0 - A*sampleGraident(d0/r0));
        float transparency1 = d1 > r1 ? 1.0:sqrt(1.0 - A*sampleGraident(d1/r1));
        float transparency;

        if(mid <= 0.0){
            transparency = transparency0/transparency1;
        }
        if(mid > 0.0 && mid < len){
            float r = (mid * r1 + (len - mid) * r0)/len;
            float dr = distance(pLocal, vec2(mid, 0))/r;
            transparency = (1.0 - A*sampleGraident(dr))/transparency0/transparency1;
        }
        if(mid >= len){
            transparency = transparency1/transparency0;
        }

        outColor = vec4(color, 1.0 - transparency);
    }
    return;
}

float x2n(float x){
    if(stampMode == EquiDistance) return x / stampInterval;
    if(stampMode == RatioDistance){
        float L = distance(p0, p1);
        if(r0 == r1) return x/(stampInterval*r0);
        else return -L / stampInterval / (r0 - r1) * log(1.0 - (1.0 - r1/r0)/L * x);
    }
}

float n2x(float n){
    if(stampMode == EquiDistance) return n * stampInterval;
    if(stampMode == RatioDistance){
        float L = distance(p0, p1);
        if(r0 == r1) return n * stampInterval * r0;
        else return L * (1.0-exp(-(r0-r1)*n*stampInterval/L)) / (1.0-r1/r0);
    }
}

mat2 rotate(float angle){
    return mat2(cos(angle), -sin(angle), sin(angle), cos(angle));
}

// ----------------------------------------------------------------------------------
float random (in vec2 st) {
    return fract(sin(dot(st.xy,
                        vec2(12.9898,78.233)))*
        43758.5453123);
}

float noise (in vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);

    // Four corners in 2D of a tile
    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));

    vec2 u = f * f * (3.0 - 2.0 * f);

    return mix(a, b, u.x) +
            (c - a)* u.y * (1.0 - u.x) +
            (d - b) * u.x * u.y;
}

#define OCTAVES 6
float fbm (in vec2 st) {
    // Initial values
    float value = 0.0;
    float amplitude = .5;
    float frequency = 0.;
    //
    // Loop of octaves
    for (int i = 0; i < OCTAVES; i++) {
        value += amplitude * noise(st);
        st *= 2.;
        amplitude *= .5;
    }
    return value;
}