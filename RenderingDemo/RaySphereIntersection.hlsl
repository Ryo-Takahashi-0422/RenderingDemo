#define PI 3.14159265

// https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-sphere-intersection.html
// �����̔��ʎ� 2���������̉��𗘗p

bool hasIntersectionWithCircle(float2 o, float2 d, float r)
{
    float a = dot(d, d);
    float2 b = 2 * dot(o, d);
    float c = dot(o, o) - r * r;
    
    float discriminant = b * b - 4 * a * c;
    return discriminant >= 0 && c < 0; // ���͈�ȏ㑶�݂��A����dir�̐�Ō�������ꍇ
}

bool DiscriminateIntersectionWithCircle(float2 o, float2 d, float r, out float t)
{
    float a = dot(d, d);
    float2 b = 2 * dot(o, d);
    float c = dot(o, o) - r * r;
    
    float discriminant = b * b - 4 * a * c;
    if (discriminant < 0) 
        return false;
    
    t = -b + sqrt(discriminant) / (2 * a);
    
    return c < 0; // ���͈�ȏ㑶�݂��A����dir�̐�Ō�������ꍇ
}