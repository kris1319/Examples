#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#define PI      3.14159265358979323846
#define slots   30
#define newtime   100
#define gleaftime 80
#define gbranchtime 30
#define gheigh  0.05//0.25
#define gwidth  0.0015//0.0075
#define dheigh  0.03 * 0.05
#define dwidth  0.03 * 0.0015
#define pheigh  0.05 * 0.05
#define pwidth  0.05 * 0.0015

enum obj {
    LEAF, BRANCH
};

enum Seasons {
    WINTER, SPRING, SUMMER, AUTUMN_1, AUTUMN_2, AUTUMN_3
};

class tree {
    float width, heigh;
    float w0, h0;
    float place;
    float xz_angle, xy_angle;
    unsigned born_time;

    int free_slot;

public:
    Seasons season;
    tree *prev;
    tree *child[slots];
    static unsigned time;
    static float dw, dh;
    static unsigned nbranch, nleaf;
    glm::mat4x4 mod;
    obj type;

public:
    tree(obj t, float w, float h, tree *par);

    void grow(bool f);
    void disp();
    void roll(float angle);

    float get_heigh() {
        return heigh;
    }
};
