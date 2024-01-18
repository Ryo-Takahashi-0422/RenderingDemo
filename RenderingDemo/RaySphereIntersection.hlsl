#define PI 3.14159265

// https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-sphere-intersection.html
// Œğ·‚Ì”»•Ê® 2Ÿ•û’ö®‚Ì‰ğ‚ğ—˜—p

bool hasIntersectionWithCircle(float2 o, float2 d, float r)
{
    float a = dot(d, d);
    float2 b = 2 * dot(o, d);
    float c = dot(o, o) - r * r;
    
    float discriminant = b * b - 4 * a * c;
    if (discriminant < 0)
        return false;
    
    float t1 = -b + sign(b) * sqrt(discriminant) / (2 * a);
    float t2 = c / t1;
    
    if (t1 > t2)
    {
        float tmp = t1;
        t1 = t2;
        t2 = tmp;
    }
    
    if (t1 < 0)
    {
        t1 = t2; // if t0 is negative, let's use t1 instead
        if (t1 < 0)
            return false; // both t0 and t1 are negative
    }
    
    return true;
}

bool DiscriminateIntersectionWithCircle(float2 o, float2 d, float r, out float t)
{
    float a = dot(d, d);
    float2 b = 2 * dot(o, d);
    float c = dot(o, o) - r * r;
    
    float discriminant = b * b - 4 * a * c;
    if (discriminant < 0) 
        return false;
    
    float t1 = -b + sign(b) * sqrt(discriminant) / (2 * a);
    float t2 = c / t1;
    
    if (t1 > t2)
    {
        float tmp = t1;
        t1 = t2;
        t2 = tmp;
    }
    
    if (t1 < 0)
    {
        t1 = t2; // if t0 is negative, let's use t1 instead
        if (t1 < 0)
            return false; // both t0 and t1 are negative
    }

    t = t1;
    return true;
}