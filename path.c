#include <stdlib.h>
#include <stdio.h>
#include "path.h"
#include "svg.h"

//Returns a random integer in the range [a,b] inclusive

static int random_int(int a, int b) {
    return a + rand() % (b - a + 1);
}

//path_list is a growable array of points, used to represent a solution path.
// The following functions are used to create, add to, and free a path_list.

static path_list path_list_create(void) {
    path_list path;
    path.capacity = 4;
    path.length = 0;
    path.points = malloc(sizeof(point) * path.capacity);
    return path;
}

static void path_list_add(path_list *path, point p) {
    if (path->length == path->capacity) {
        path->capacity *= 2;
        path->points = realloc(path->points, sizeof(point) * path->capacity);
    }
    path->points[path->length++] = p;
}

void path_list_free(path_list *path) {
    free(path->points);
    path->points = NULL;
    path->length = 0;
    path->capacity = 0;
}

//tree helper functions for path finding

static int is_leaf(tree *node) {
    return node->left == NULL && node->right == NULL;
}

/* Half-open on the far edge: every (x,y) belongs to EXACTLY one region.
   This is the fix for the "point on a wall" ambiguity: a point can never
   simultaneously satisfy two sibling regions. */
static int belongs_to_region(rectangle area, int x, int y) {
    if (x < area.x) return 0;
    if (x >= area.x + area.width) return 0;
    if (y < area.y) return 0;
    if (y >= area.y + area.height) return 0;
    return 1;
}

static tree *child_containing_point(tree *node, int x, int y) {
    if (belongs_to_region(node->left->rectangle, x, y))
        return node->left;
    if (belongs_to_region(node->right->rectangle, x, y))
        return node->right;
    return NULL; /* should not happen for a point that is valid in node's own rectangle */
}

// Given a node and its wall, return the coordinates of the door in that wall
static void get_doors(tree *node, point *door_left, point *door_right) {
    if (node->wall_direction == VERTICAL) {
        door_left->x  = node->wall_pos - 1;
        door_left->y  = node->door_pos;
        door_right->x = node->wall_pos;
        door_right->y = node->door_pos;
    } else {
        door_left->x  = node->door_pos;
        door_left->y  = node->wall_pos - 1;
        door_right->x = node->door_pos;
        door_right->y = node->wall_pos;
    }
}

// Recursive helper function for find_path: returns the ordered list of points
// (start, every door crossed, end) that connects entry to exit.

static path_list find_path_node(tree *node, point entry, point exit) {
    path_list path;

    if (is_leaf(node)) {
        path = path_list_create();
        path_list_add(&path, entry);
        path_list_add(&path, exit);
        return path;
    }

    tree *entry_child = child_containing_point(node, entry.x, entry.y);
    tree *exit_child  = child_containing_point(node, exit.x, exit.y);

    if (entry_child == exit_child) {
        // both points are in the same sub-region: recurse, no wall crossed here 
        return find_path_node(entry_child, entry, exit);
    }

    /* entry and exit are on opposite sides of this node's wall:
       route through the door, using the coordinate valid for each side */
    point door_left, door_right;
    get_doors(node, &door_left, &door_right);

    point entry_door = (entry_child == node->left) ? door_left : door_right;
    point exit_door  = (exit_child  == node->left) ? door_left : door_right;

    path_list path1 = find_path_node(entry_child, entry, entry_door);
    path_list path2 = find_path_node(exit_child, exit_door, exit);

    path = path_list_create();

    for (int i = 0; i < path1.length; i++)
        path_list_add(&path, path1.points[i]);

    for (int i = 0; i < path2.length; i++)
        path_list_add(&path, path2.points[i]);

    path_list_free(&path1);
    path_list_free(&path2);

    return path;
}

// Public API functions

point random_point_in_maze(maze *m) {
    point p;
    p.x = random_int(0, m->rectangle.width - 1);
    p.y = random_int(0, m->rectangle.height - 1);
    return p;
}

path_list find_path(maze *m, point entry, point exit) {
    return find_path_node(m, entry, exit);
}

void maze_svg_solved(maze *m, char *filename, path_list path) {
    FILE *f = fopen(filename, "w");
    if (f == NULL) return;

    unsigned int width = m->rectangle.width;
    unsigned int height = m->rectangle.height;

    svg_header(f, width, height);
    svg_rect(f, 0, 0, width, height);
    draw_tree(f, m);

    // Draw the solution path as a series of connected lines, with a blue circle at the start and an orange circle at the end.
    for (int i = 0; i < path.length - 1; i++) {
        point a = path.points[i];
        point b = path.points[i + 1];
        fprintf(f,
    "<line x1=\"%.1f\" y1=\"%.1f\" x2=\"%.1f\" y2=\"%.1f\" "
    "style=\"stroke:rgb(0,150,0);stroke-width:0.4;stroke-opacity:0.5\"/>\n",
    a.x + 0.5, a.y + 0.5, b.x + 0.5, b.y + 0.5);
        }

    // Draw circles at the start and end points of the path
    if (path.length > 0) {
        point start = path.points[0];
        point end   = path.points[path.length - 1];
        fprintf(f, "<circle cx=\"%.1f\" cy=\"%.1f\" r=\"0.6\" fill=\"blue\"/>\n", start.x + 0.5, start.y + 0.5);
        fprintf(f, "<circle cx=\"%.1f\" cy=\"%.1f\" r=\"0.6\" fill=\"orange\"/>\n", end.x + 0.5, end.y + 0.5);
    }

    svg_footer(f);
    fclose(f);
}