# genmistsky
Generate a multi-layered participating atmosphere for Radiance

## Build and run
With GCC installed on a Linux computer, this should build and run with:

    git clone https://github.com/markstock/genmistsky.git
    cd genmistsky
    make
    make test

The last command will generate the Earth and some `mist` volumes and render a view of the sunrise line from space in an `rvu` window. It assumes that Radiance is installed and the binaries are in your `PATH`.

![render from space station](media/img_make_test.png?raw=true "Rendering from space")

## Details
Rayleigh scattering (from gas molecules) is the primary source of the sky's color; Mie scattering (from larger particles such as dust and water droplets) serves to spread the sun's rays into a halo, and refraction (due to density changes) causes very low-angle sun rays to curve. The combination of these three effects can be simulated in Radiance with `mist` and `interface` materials applied to Earth-sized `sphere` and `bubble` geometry.

It it important to note that this code does not specify the sun's direction, nor does it provide the `bubble` that is necessary if the view point is inside of one of the mist volumes. The user is responsible for those.

## Thanks
The original idea for modeling skies in Radiance came from [Taoning Wang's post](https://discourse.radiance-online.org/t/notes-on-misty-sky/6007/) on the Radiance Discourse server. The [standard atmosphere code](https://apps.dtic.mil/sti/pdfs/ADA588839.pdf) is from R.J. Yager. [Radiance](https://floyd.lbl.gov/radiance/) is a lighting simulation package created by Greg Ward.
