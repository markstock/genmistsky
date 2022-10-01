/*
 * genmistsky - create a rad file for the earth and atmosphere from
 *              many shells of mist material
 *
 * Idea from Taoning Wang https://discourse.radiance-online.org/t/notes-on-misty-sky/6007/
 * Atmosphere from yagenaut@gmail.com, 2013-03-15
 * (c)2022 Mark J. Stock <markjstock@gmail.com>
 */

#include "y_atmosphere.h"

#include <cstring>
#include <iostream>
#include <vector>

//
// basic usage
//
static void usage() {
  fprintf(stderr, "Usage: genmistsky [-h] [-n=<nlayers>] [-unif] [-z=<low alt>] [+z=<high alt>] [-ms=<mie scale>]\n");
  exit(1);
}

//
// main routine - run the program
//
int main(int argc, char *argv[]) {

  // number of geometric layers
  int num_rayleigh_layers = 10;

  // set layers by density or by thickness
  bool layers_by_density = true;

  // layers start and end at these altitudes
  double bottom_alt = -300.0;	// meters (Dead Sea is -300m)
  double top_alt = 10000.0;		// meters (Mt. Everest is 8848m)

  double mie_relative_density = 0.1;	// will scale by air density

  // toggle the layers
  const bool do_rayleigh = true;
  const bool do_mie = true;
  const bool do_refract = false;

  for (int i=1; i<argc; i++) {
    if (strncmp(argv[i], "-n=", 3) == 0) {
      int num = atoi(argv[i]+3);
      if (num < 1) { std::cerr << "Need at least 1 layer!\n"; usage(); }
      if (num > 1000) { std::cerr << "Too many layers!\n"; usage(); }
      num_rayleigh_layers = num;
    } else if (strncmp(argv[i], "-z=", 3) == 0) {
      double alt = atof(argv[i]+3);
      bottom_alt = alt;
    } else if (strncmp(argv[i], "+z=", 3) == 0) {
      double alt = atoi(argv[i]+3);
      if (alt < bottom_alt) { std::cerr << "Top layer is below bottom layer!\n"; usage(); }
      top_alt = alt;
    } else if (strncmp(argv[i], "-unif", 5) == 0) {
      // use uniformly-thick layers (better for images from space)
      layers_by_density = false;
    } else if (strncmp(argv[i], "-ms", 3) == 0) {
      double scale = atoi(argv[i]+3);
      if (scale <= 0.0) { std::cerr << "Mie scale can't be negative!\n"; usage(); }
      mie_relative_density = scale;
    } else if (strncmp(argv[i], "-h", 2) == 0 or strncmp(argv[i], "--h", 3) == 0) {
      usage();
    }
  }


  // set the parameters

  // write sun brightness and direction
  const double sun_radiance = 10026204.;	// units?

  // write Earth
  const double earth_rad = 6378100;	// meters

  // Rayleigh scattering is from air molecules themselves
  // strongly color-dependent, no absorption, isotropic
  // volumetric scattering at STP, per meter
  const double rayleigh_volum_red = 1.067e-5;	// at 560nm
  const double rayleigh_volum_grn = 1.336e-5;	// at 530nm
  const double rayleigh_volum_blu = 3.479e-5;	// at 420nm
  const double rayleigh_albedo = 1.0;
  const double rayleigh_hgconst = 0.0;

  // Mie scattering is from clouds, aerosols, and large particles
  // fairly independent of color, strongly forward-scattered, some absorption
  const int num_mie_layers = num_rayleigh_layers - 1;
  const double mie_albedo = 0.95;			// total albedo
  const double mie_hgconst = 0.5;
  const double mie_per_layer = 1.0 - (1.0-mie_albedo) / num_mie_layers;

  // to account for the density change, the index of refraction of air-vs-vacuum
  // is spread out into multiple layers of a little each
  // this redirects the setting and rising sunlight a little
  // wrong - just write one refracting layer, above any mountains
  const double total_ior = 1.0003;
  const int num_refract_layers = num_rayleigh_layers - 1;
  const double ior_per_layer = (total_ior-1.0)/num_refract_layers;


  std::cerr << "# writing earth and mist sky layers\n";
  std::cerr << "# make sure to change the \"solar source sun\" below\n";
  std::cerr << "# and include the appropriate \"bubble\" around the view point\n\n";

  // write the first stuff
  std::cout << "void plastic ec 0 0 5 0.1 0.1 0.1 0 0\n";
  std::cout << "ec sphere earth 0 0 4 0 0 " << -earth_rad << " " << earth_rad << "\n";
  std::cout << "\n";

  std::cout << "void light solar 0 0 3 " << sun_radiance << " " << sun_radiance << " " << sun_radiance << " " << "\n";
  std::cout << "solar source sun 0 0 4 x y z 0.533\n";
  std::cout << "\n";


  // start to do the work

  // where does this atmosphere start and end?
  const double rho_bottom = yAtmosphere::Density(bottom_alt);
  const double rho_top = yAtmosphere::Density(top_alt);
  const double rho_per_layer = (rho_bottom-rho_top)/num_rayleigh_layers;
  const double m_per_layer = (top_alt-bottom_alt)/num_rayleigh_layers;

  // find altitudes for each band
  std::vector<double> layer_alts(num_rayleigh_layers+1);
  layer_alts[0] = bottom_alt;
  layer_alts[num_rayleigh_layers] = top_alt;
  for (int ilayer=1; ilayer<num_rayleigh_layers; ++ilayer) {

    if (layers_by_density) {
      const double this_dens = rho_bottom - ilayer*rho_per_layer;

      // start at last alt and march up
      double test_alt = layer_alts[ilayer-1];
      while (yAtmosphere::Density(test_alt) > this_dens) test_alt += 1.0;
      layer_alts[ilayer] = test_alt;

    } else {
      // layers are uniformly-spaced
      layer_alts[ilayer] = std::floor(bottom_alt + ilayer*m_per_layer + 0.5);
    }
  }


  // write alternating layers of: Rayleigh mist, Mie mist, refraction interface

  for (int ilayer=0; ilayer<num_rayleigh_layers; ++ilayer) {

    // elevs
    const double low = layer_alts[ilayer];
    const double hih = layer_alts[ilayer+1];
    const double thick = hih - low;
    const double lowrad = earth_rad + low;
    const double hirad = earth_rad + hih;

    std::cout << std::defaultfloat;
    std::cout << "# layer " << ilayer << " covers " << low << " to " << hih << " meters ASL\n\n";

    // mean relative density for this layer
    const double dens = 0.5 * (yAtmosphere::Density(low) + yAtmosphere::Density(hih)) / yAtmosphere::Density(0.0);

    // Rayleigh layer (min to max-4)
    if (do_rayleigh) {
      // adjust for being slightly thinner
      const double this_dens = dens * thick / (thick-4.0);

      std::cout << std::defaultfloat;
      std::cout << "void mist atmo" << ilayer << " 1 sun 0 7 " << rayleigh_volum_red*this_dens << " " << rayleigh_volum_grn*this_dens << " " << rayleigh_volum_blu*this_dens << " " << rayleigh_albedo << " " << rayleigh_albedo << " " << rayleigh_albedo << " " << rayleigh_hgconst << "\n";
      std::cout << std::fixed;
      std::cout << "atmo" << ilayer << " bubble inner" << ilayer << " 0 0 4 0 0 " << -earth_rad << " " << lowrad << "\n";
      std::cout << "atmo" << ilayer << " sphere outer" << ilayer << " 0 0 4 0 0 " << -earth_rad << " " << hirad-4.0 << "\n";
      std::cout << "\n";
    }

    // Mie layer (max-2 to max-1)
    if (do_mie and ilayer < num_rayleigh_layers-1) { // baby!
      // must scale up because thickness is only 1m!
      const double this_dens = mie_relative_density * dens * rayleigh_volum_grn * thick;

      std::cout << std::defaultfloat;
      std::cout << "void mist miemat" << ilayer << " 1 sun 0 7 " << this_dens << " " << this_dens << " " << this_dens << " " << mie_per_layer << " " << mie_per_layer << " " << mie_per_layer << " " << mie_hgconst << "\n";
      std::cout << std::fixed;
      std::cout << "miemat" << ilayer << " bubble inner" << ilayer << " 0 0 4 0 0 " << -earth_rad << " " << hirad-2.0 << "\n";
      std::cout << "miemat" << ilayer << " sphere outer" << ilayer << " 0 0 4 0 0 " << -earth_rad << " " << hirad-1.0 << "\n";
      std::cout << "\n";
    }

    // refraction layer (zero-thickness at max-3)
    if (do_refract and ilayer < num_rayleigh_layers-1) {
      const double n_low = total_ior - ilayer*ior_per_layer;
      const double n_high = total_ior - (ilayer+1)*ior_per_layer;

      std::cout << std::defaultfloat;
      std::cout << "void interface refr" << ilayer << " 0 0 8 1 1 1 " << n_low << " 1 1 1 " << n_high << "\n";
      std::cout << std::fixed;
      std::cout << "refr" << ilayer << " sphere bendy" << ilayer << " 0 0 4 0 0 " << -earth_rad << " " << hirad-3.0 << "\n";
      std::cout << "\n";
    }

    // mie layer (always 1m)
  }

  std::cerr << "# Suggest generating the octree with optional parameters:\n";
  std::cerr << "#     oconv -n 32 -r 10000000 this.rad > scene.oct\n";

  return 0;
}
