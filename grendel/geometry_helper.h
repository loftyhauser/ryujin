#ifndef GEOMETRY_HELPER_H
#define GEOMETRY_HELPER_H

#include <deal.II/grid/grid_generator.h>
#include <deal.II/grid/grid_tools.h>

#include <deal.II/grid/manifold_lib.h>

namespace grendel
{
  /*
   * This header file contains a number of helper functions to create
   * varios different meshes.  It is used in the Discretization class.
   */


  /**
   * Create a 2D triangulation consisting of a rectangle with a prescribed
   * length and height with an insribed obstacle given by a centered,
   * equilateral triangle.
   *
   * Even though this function has a template parameter @p dim, it is only
   * implemented for dimension 2.
   */
  template <int dim>
  void create_coarse_grid_triangle(
      dealii::parallel::distributed::Triangulation<dim> &,
      const double /*length*/,
      const double /*height*/,
      const double /*object_height*/)
  {
    AssertThrow(false, dealii::ExcNotImplemented());
  }


  template <>
  void create_coarse_grid_triangle<2>(
      dealii::parallel::distributed::Triangulation<2> &triangulation,
      const double length,
      const double height,
      const double object_height)
  {
    constexpr int dim = 2;

    using namespace dealii;

    const double object_length = object_height * std::sqrt(3) / 2.;

    const std::vector<Point<dim>> vertices{
        {0., 0.},                                     // 0, bottom left
        {(length - object_length) / 2., 0.},          // 1, bottom center left
        {(length + object_length) / 2., 0.},          // 2, bottom center right
        {length, 0.},                                 // 3, bottom right
        {0., height / 2.},                            // 4, middle left
        {(length - object_length) / 2., height / 2.}, // 5, middle center left
        {(length + object_length) / 2.,
         (height - object_height) / 2.},         // 6, middle lower center right
        {length, (height - object_height) / 2.}, // 7, middle lower right
        {(length + object_length) / 2.,
         (height + object_height) / 2.},         // 8, middle upper center right
        {length, (height + object_height) / 2.}, // 9, middle upper right
        {0., height},                            // 10, top left
        {(length - object_length) / 2., height}, // 11, top center left
        {(length + object_length) / 2., height}, // 12, top center right
        {length, height}                         // 13, top right
    };

    std::vector<CellData<dim>> cells(7);
    {
      const auto assign = [](auto b, std::array<unsigned int, 4> a) {
        std::copy(a.begin(), a.end(), b);
      };
      assign(cells[0].vertices, {0, 1, 4, 5});
      assign(cells[1].vertices, {1, 2, 5, 6});
      assign(cells[2].vertices, {2, 3, 6, 7});
      assign(cells[3].vertices, {4, 5, 10, 11});
      assign(cells[4].vertices, {5, 8, 11, 12});
      assign(cells[5].vertices, {8, 9, 12, 13});
      assign(cells[6].vertices, {6, 7, 8, 9});
    }

    triangulation.create_triangulation(vertices, cells, SubCellData());

    /*
     * Set boundary ids:
     */

    for (auto cell : triangulation.active_cell_iterators()) {
      for (unsigned int f = 0; f < GeometryInfo<dim>::faces_per_cell; ++f) {
        const auto face = cell->face(f);

        if (!face->at_boundary())
          continue;

        /*
         * We want reflective boundary conditions (i.e. indicator 1) at top
         * bottom and on the triangle. On the left and right side we leave
         * the boundary indicator at 0, i.e. do nothing.
         */
        const auto center = face->center();
        if (center[0] > 0. && center[0] < length)
          face->set_boundary_id(1);
      }
    }
  }


  /**
   * Create an nD tube with a given length and diameter. More precisely,
   * this is a line in 1D, a rectangle in 2D, and a cylinder in 3D.
   *
   * We set boundary indicator 0 on the left and right side to indicate "do
   * nothing" boundary conditions, and boundary indicator 1 at the top and
   * bottom side in 2D, as well as the curved portion of the boundary in 3D
   * to indicate "reflective boundary conditions".
   */

  template <int dim>
  void
  create_coarse_grid_tube(dealii::parallel::distributed::Triangulation<dim> &,
                          const double /*length*/,
                          const double /*diameter*/) = delete;


  template <>
  void create_coarse_grid_tube<1>(
      dealii::parallel::distributed::Triangulation<1> &triangulation,
      const double length,
      const double /*diameter*/)
  {
    dealii::GridGenerator::hyper_cube(triangulation, 0., length);
    triangulation.begin_active()->face(0)->set_boundary_id(0);
    triangulation.begin_active()->face(1)->set_boundary_id(0);
  }


  template <>
  void create_coarse_grid_tube<2>(
      dealii::parallel::distributed::Triangulation<2> &triangulation,
      const double length,
      const double diameter)
  {
    using namespace dealii;

    GridGenerator::hyper_rectangle(triangulation,
                                   Point<2>(-length / 2., 0.),
                                   Point<2>(length / 2., diameter));

    /*
     * Set boundary ids:
     */

    for (auto cell : triangulation.active_cell_iterators()) {
      for (unsigned int f = 0; f < GeometryInfo<2>::faces_per_cell; ++f) {
        const auto face = cell->face(f);

        if (!face->at_boundary())
          continue;

        /*
         * We want reflective boundary conditions (i.e. indicator 1) at top
         * and bottom of the rectangle. On the left and right side we leave
         * the boundary indicator at 0, i.e. do nothing.
         */

        const auto center = face->center();
        if (center[0] > -length / 2. && center[0] < length / 2.)
          face->set_boundary_id(1);
      }
    }
  }


  template <>
  void create_coarse_grid_tube<3>(
      dealii::parallel::distributed::Triangulation<3> &triangulation,
      const double length,
      const double diameter)
  {
    using namespace dealii;

    GridGenerator::cylinder(triangulation, diameter / 2., length / 2.);

    /*
     * Set boundary ids:
     */

    for (auto cell : triangulation.active_cell_iterators()) {
      for (unsigned int f = 0; f < GeometryInfo<2>::faces_per_cell; ++f) {
        const auto face = cell->face(f);

        if (!face->at_boundary())
          continue;

        /*
         * We want reflective boundary conditions (i.e. indicator 1) at top
         * and bottom of the rectangle. On the left and right side we leave
         * the boundary indicator at 0, i.e. do nothing.
         */

        const auto center = face->center();
        if (center[0] > -length / 2. && center[0] < length / 2.)
          face->set_boundary_id(1);
      }
    }
  }


  /**
   * Create the 2D mach step triangulation.
   *
   * Even though this function has a template parameter @p dim, it is only
   * implemented for dimension 2.
   */

  template <int dim>
  void
  create_coarse_grid_step(dealii::parallel::distributed::Triangulation<dim> &,
                          const double /*length*/,
                          const double /*height*/,
                          const double /*step_position*/,
                          const double /*step_height*/)
  {
    AssertThrow(false, dealii::ExcNotImplemented());
  }


  template <>
  void create_coarse_grid_step<2>(
      dealii::parallel::distributed::Triangulation<2> &triangulation,
      const double length,
      const double height,
      const double step_position,
      const double step_height)
  {
    constexpr int dim = 2;

    using namespace dealii;

    Triangulation<dim> tria1;

    GridGenerator::subdivided_hyper_rectangle(
        tria1, {15, 4}, Point<2>(0., step_height), Point<2>(length, height));

    Triangulation<dim> tria2;

    GridGenerator::subdivided_hyper_rectangle(
        tria2, {3, 1}, Point<2>(0., 0.), Point<2>(step_position, step_height));

    GridGenerator::merge_triangulations(tria1, tria2, triangulation);

    /*
     * Set boundary ids:
     */

    for (auto cell : triangulation.active_cell_iterators()) {
      for (unsigned int f = 0; f < GeometryInfo<2>::faces_per_cell; ++f) {
        const auto face = cell->face(f);

        if (!face->at_boundary())
          continue;

        /*
         * We want reflective boundary conditions (i.e. indicator 1) at top
         * and bottom of the rectangle. On the left and right side we leave
         * the boundary indicator at 0, i.e. do nothing.
         */

        const auto center = face->center();
        if (center[0] > 0. + length / 15. && center[0] < length - length / 15.)
          face->set_boundary_id(1);
      }
    }

    /*
     * Refine four times and round off corner with radius 0.0125:
     */

    triangulation.refine_global(4);

    Point<dim> point(step_position + 0.0125, step_height - 0.0125);
    triangulation.set_manifold(1, SphericalManifold<dim>(point));

    for (auto cell : triangulation.active_cell_iterators())
      for (unsigned int v = 0; v < GeometryInfo<dim>::vertices_per_cell; ++v) {
        double distance =
            (cell->vertex(v) - Point<dim>(step_position, step_height)).norm();
        if (distance < 1.e-6) {
          for (unsigned int f = 0; f < GeometryInfo<dim>::faces_per_cell; ++f) {
            const auto face = cell->face(f);
            if(face->at_boundary())
              face->set_manifold_id(1);
            cell->set_manifold_id(1); // temporarily for second loop
          }
        }
      }

    for (auto cell : triangulation.active_cell_iterators()) {
      if (cell->manifold_id() != 1)
        continue;

      cell->set_manifold_id(0); // reset manifold id again

      for (unsigned int v = 0; v < GeometryInfo<dim>::vertices_per_cell; ++v) {
        auto &vertex = cell->vertex(v);

        if (std::abs(vertex[0] - step_position) < 1.e-6 &&
            vertex[1] > step_height - 1.e-6)
          vertex[0] = step_position + 0.0125 * (1 - std::sqrt(1. / 2.));

        if (std::abs(vertex[1] - step_height) < 1.e-6 &&
            vertex[0] < step_position + 0.005)
          vertex[1] = step_height - 0.0125 * (1 - std::sqrt(1. / 2.));
      }
    }
  }

} // namespace grendel

#endif /* GEOMETRY_HELPER_H */
