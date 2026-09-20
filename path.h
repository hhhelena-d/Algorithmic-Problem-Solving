#include "maze.h"

//A single coordinate in the maze's grid 
typedef struct {
    int x, y;
} point;

// Growable list of points making up a solution path 
typedef struct {
    point *points;
    int length;
    int capacity;
} path_list;

// Pick a random point that is guaranteed to be a valid maze cell.
point random_point_in_maze(maze *m);

// Divide and conquer path finder: returns the ordered list of points
   //(start, every door crossed, end) that connects entry to exit. 
path_list find_path(maze *m, point entry, point exit);

// Frees the memory owned by a path_list 
void path_list_free(path_list *path);

// Draws the maze and the solution path to an SVG file
void maze_svg_solved(maze *m, char *filename, path_list path);


