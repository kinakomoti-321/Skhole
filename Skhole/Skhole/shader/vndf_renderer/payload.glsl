
struct PayLoadStruct{
    vec3 position;
    vec3 basecolor;

    float anisotropic;
    float roughness;
    float metallic;

    vec3 normal;

    bool isMiss;

    bool isLight;
    vec3 emission;

    bool isGlass;
    float ior;

    bool useFastMultiple;

    float t;

    uint instanceID;
    uint primID;
};