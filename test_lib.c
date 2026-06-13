#include <stdio.h>

typedef struct {
    int x;
    int y;
} Point;

typedef union {
    int i;
    double d;
} IntOrDouble;

Point add_points(Point p1, Point p2) {
    Point r;
    r.x = p1.x + p2.x;
    r.y = p1.y + p2.y;
    return r;
}

double print_union(IntOrDouble u, int is_double) {
    if (is_double) {
        printf("Union double: %f\n", u.d);
        return u.d;
    } else {
        printf("Union int: %d\n", u.i);
        return (double)u.i;
    }
}

double run_callback(double (*cb)(int), int val) {
    printf("C calling callback with %d...\n", val);
    double res = cb(val);
    printf("C callback returned %f\n", res);
    return res;
}
