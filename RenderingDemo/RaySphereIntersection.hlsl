// https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-sphere-intersection.html
// Œğ·‚Ì”»•Ê® 2Ÿ•û’ö®‚Ì‰ğ‚ğ—˜—p

bool hasIntersectionWithCircle(float2 o, float2 d, float r)
{
    float a = dot(d, d);
    float2 b = 2 * dot(o, d);
    float c = dot(o, o) - r * r;
    float t1, t2;
    
    float discriminant = b * b - 4 * a * c;
    
    if (discriminant < 0) 
        return false;
    else if (discriminant == 0)
        t1 = t2 = -0.5 * b / a;
    else
    {
        float q = (b > 0) ?
            -0.5 * (b + sqrt(discriminant)) :
            -0.5 * (b - sqrt(discriminant));
        t1 = q / a;
        t2 = c / q;
    }
    
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
    float t1, t2;
    
    float discriminant = b * b - 4 * a * c;
    
    if (discriminant < 0) 
        return false;
    else if (discriminant == 0)
        t1 = t2 = -0.5 * b / a;
    else
    {
        float q = (b > 0) ?
            -0.5 * (b + sqrt(discriminant)) :
            -0.5 * (b - sqrt(discriminant));
        t1 = q / a;
        t2 = c / q;
    }
    
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

bool hasIntersectionWithSphere(float3 o, float3 d, float r)
{
    float a = dot(d, d);
    float2 b = 2 * dot(o, d);
    float c = dot(o, o) - r * r;
    float t1, t2;
    
    float discriminant = b * b - 4 * a * c;
    
    if (discriminant < 0) 
        return false;
    else if (discriminant == 0)
        t1 = t2 = -0.5 * b / a;
    else
    {
        float q = (b > 0) ?
            -0.5 * (b + sqrt(discriminant)) :
            -0.5 * (b - sqrt(discriminant));
        t1 = q / a;
        t2 = c / q;
    }
    
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

bool DiscriminateIntersectionWithSphere(float3 o, float3 d, float r, out float t)
{
    float a = dot(d, d);
    float2 b = 2 * dot(o, d);
    float c = dot(o, o) - r * r;
    float t1, t2;
    
    float discriminant = b * b - 4 * a * c;
    
    if (discriminant < 0) 
        return false;
    else if (discriminant == 0)
        t1 = t2 = -0.5 * b / a;
    else
    {
        float q = (b > 0) ?
            -0.5 * (b + sqrt(discriminant)) :
            -0.5 * (b - sqrt(discriminant));
        t1 = q / a;
        t2 = c / q;
    }
    
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