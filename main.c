#include <stdio.h>
#include <stdlib.h>
#include "maze.h"
#include "path.h"


int main(int argc, char **argv) {
  //unsigned int len;
  if (argc < 3) {
    printf("Usage: %s <width> <height>\n", *argv);
    return 1;
  }

  /* You need to define this function in "maze.c" */
  maze *m = maze_random(atoi(argv[1]), atoi(argv[2]));

  /* And also this one, using the SVG drawing function provided in "svg.h" */
  maze_svg(m, "maze.svg");

point start = random_point_in_maze(m);
point end   = random_point_in_maze(m);
path_list solution = find_path(m, start, end);

maze_svg_solved(m, "maze.svg", solution);
for (int i = 0; i < solution.length; i++)
    printf("(%d, %d)\n", solution.points[i].x, solution.points[i].y);

path_list_free(&solution);


  return 0;
}
