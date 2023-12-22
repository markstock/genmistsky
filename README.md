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

This program assumes that the scene is centered around (0,0,0) with East along positive x, North toward positive y, up along positive z, and all spatial units in meters. The center of the Earth (and of all atmospheric shells) is (0,0,-6378100).

It it important to note that this code does not specify the sun's direction, nor does it provide the `bubble` that is necessary if the view point is inside of one of the mist volumes. The user is responsible for those.

![rendering of dawn](media/img43_cropsm.png?raw=true "Rendering of South-Central Colorado at dawn")

## To Do
* Consider adjusting the top-of-atmosphere color such that a grey object at the Earth's surface is really achromatic.
* Generate rendering samples: of a standard scene at Earth's surface but with varying Mie coefficients, the same but with different numbers of layers, the same but at different altitudes.
* Add option to load image of Earth lights at night, correlate them to viewer's location, and apply them as area light sources on the ground in order to allow distant cities to add horizon-glow realistically.

## Thanks
The original idea for modeling skies in Radiance came from [Taoning Wang's post](https://discourse.radiance-online.org/t/notes-on-misty-sky/6007/) on the Radiance Discourse server. The [standard atmosphere code](https://apps.dtic.mil/sti/pdfs/ADA588839.pdf) is from R.J. Yager. [Radiance](https://floyd.lbl.gov/radiance/) is a lighting simulation package created by Greg Ward.
