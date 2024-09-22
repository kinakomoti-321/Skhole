void tangentSpaceBasis(vec3 normal,inout vec3 t,inout vec3 b){
    if (abs(normal.y) < 0.9)
    {
        t = cross(normal, vec3(0, 1, 0));
    }
    else
    {
        t = cross(normal, vec3(0, 0, -1));
    }
    t = normalize(t);
    b = cross(t, normal);
    b = normalize(b);
}

vec3 worldtoLoacal(vec3 v,vec3 lx, vec3 ly,vec3 lz){
    return vec3(v.x * lx.x + v.y* lx.y + v.z * lx.z,
                 v.x * ly.x + v.y * ly.y + v.z * ly.z,
                 v.x * lz.x + v.y * lz.y + v.z * lz.z);
}

vec3 localToWorld(const vec3 v, const vec3 lx, const vec3 ly,
                   const vec3 lz)
{
    return vec3(v.x * lx.x + v.y * ly.x + v.z * lz.x,
                 v.x * lx.y + v.y * ly.y + v.z * lz.y,
                 v.x * lx.z + v.y * ly.z + v.z * lz.z);
}
